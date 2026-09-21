// ============================================================
// fuzz/fuzz_stack.cpp
//
// Model-based fuzzer for StackPro::Stack<EnableStats> and
// StackPro::StackScope, checked against an independent shadow model
// after every single operation (not just at the end) so a
// fuzzer-found failure localizes to the exact operation that caused
// it.
//
// The model tracks the expected bump offset, every live allocation
// (with a unique fill pattern written into it), every outstanding
// Marker, and -- when EnableStats is true -- the expected Stats. After
// each operation it verifies that:
//   - allocate()/create() succeed exactly when the model says there is
//     room, and return exactly base + alignedOffset, correctly
//     aligned and owned by the stack
//   - no live allocation's fill pattern was ever clobbered (i.e.
//     nothing overlaps, and rollback/reset never scribbles on memory
//     that is still live)
//   - used()/remaining()/capacity()/getMarker()/getStats() all agree
//     with the model
//
// Specifically targets:
//   - capacity exhaustion boundaries: exact-fit allocations, one byte
//     over, and near-SIZE_MAX sizes (the subtraction-based bounds
//     check in allocate() exists precisely to avoid overflow there)
//   - alignment padding: every per-call alignment from 1 up to the
//     buffer's own alignment, across buffer alignments of 1..128
//   - getMarker()/freeToMarker() rollback, including markers that a
//     later rollback or reset has made stale (the harness never passes
//     those back in -- that would violate freeToMarker()'s
//     precondition -- but it does keep using every still-valid one)
//   - StackScope, including nested scopes
//   - reset(), including that it clears statistics
//   - create<T>()/destroy<T>() across differently-aligned types, plus
//     a type with a non-trivial destructor to confirm destroy() runs
//     ~T() exactly once and create() constructs only on success
//   - move construction and move assignment (including self-move),
//     that the moved-from stack is left empty and reusable, and that
//     its buffer and statistics transfer intact
//
// Deliberately NOT covered yet: constructor failure paths
// (std::bad_alloc), a T whose constructor throws inside create(), and
// contract violations -- the harness never breaks a documented
// precondition on purpose, so AP_PRE stays a pure "the harness itself
// is correct" check. Only capacities up to ~2 KiB are used.
// ============================================================

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

using StackPro::Stack;
using StackPro::StackScope;

namespace {

// Aborts (rather than throwing/returning) on mismatch so libFuzzer
// captures a minimal, precise reproducer for exactly the operation
// that broke an invariant.
inline void check(bool ok) {
    if (!ok)
        std::abort();
}

// Hands out input bytes one at a time; yields 0 once exhausted so
// an operation that needs parameters near the end of the input still
// runs deterministically.
class Reader {
  public:
    Reader(const std::uint8_t* data, std::size_t size) noexcept : data_{data}, size_{size} {}

    [[nodiscard]] bool empty() const noexcept {
        return pos_ >= size_;
    }
    std::uint8_t next() noexcept {
        return pos_ < size_ ? data_[pos_++] : std::uint8_t{0};
    }

  private:
    const std::uint8_t* data_;
    std::size_t size_;
    std::size_t pos_ = 0;
};

// Non-trivially-destructible test type: `live` must be exactly 1
// between create() and destroy(), and back to 0 afterwards.
struct Tracked {
    static inline int live = 0;
    std::uint32_t value;
    explicit Tracked(std::uint32_t v) noexcept : value{v} {
        ++live;
    }
    Tracked(const Tracked&) = delete;
    Tracked& operator=(const Tracked&) = delete;
    ~Tracked() {
        --live;
    }
};

// Over-aligned test type.
struct alignas(64) Wide {
    std::uint64_t value;
    explicit Wide(std::uint64_t v) noexcept : value{v} {}
};

template <typename T> std::uint64_t valueOf(const T& t) {
    if constexpr (std::is_class_v<T>)
        return static_cast<std::uint64_t>(t.value);
    else
        return static_cast<std::uint64_t>(t);
}

template <bool EnableStats> class Harness {
    using StackT = Stack<EnableStats>;
    using MarkerT = typename StackT::Marker;

    struct Live {
        std::size_t start; ///< Offset of the allocation within the buffer.
        std::size_t size;
        std::uint8_t fill;
        std::byte* ptr;
    };

    struct ModelStats {
        std::size_t total = 0;
        std::size_t current = 0;
        std::size_t peak = 0;
        std::size_t count = 0;
    };

    struct Prediction {
        bool fits;
        std::size_t aligned;
    };

  public:
    Harness(std::size_t capacity, unsigned bufferShift)
        : stack_{capacity, std::size_t{1} << bufferShift}, cap_{capacity}, bufShift_{bufferShift} {
        // Learn the buffer's base address once, up front, then put the
        // stack back to a pristine state (which also clears any stats
        // that probe allocation just recorded).
        base_ = stack_.allocate(1, 1);
        check(base_ != nullptr);
        stack_.reset();
        verify();
    }

    void run(Reader& r) {
        while (!r.empty()) {
            switch (r.next() % 14) {
            case 0:
            case 1:
            case 2:
                allocateOp(r);
                break;
            case 3:
                allocateHugeOp(r);
                break;
            case 4:
                allocateExactFitOp();
                break;
            case 5:
                allocateOverFitOp(r);
                break;
            case 6:
                markerOp();
                break;
            case 7:
                rollbackOp(r);
                break;
            case 8:
                scopeOp(r);
                break;
            case 9:
                resetOp();
                break;
            case 10:
            case 11:
                createOp(r);
                break;
            case 12:
            case 13:
                moveOp(r);
                break;
            default:
                break;
            }
            verify();
        }
    }

  private:
    // ---- model helpers ----------------------------------------------

    [[nodiscard]] Prediction predict(std::size_t size, std::size_t align) const {
        // Deliberately computed differently from Stack::alignForward()
        // (divide/multiply instead of mask) so the two can disagree.
        // offset_ <= cap_ <= ~2 KiB, so this cannot overflow.
        const std::size_t aligned = ((offset_ + align - 1) / align) * align;
        const bool fits = aligned <= cap_ && size <= cap_ - aligned;
        return {fits, aligned};
    }

    // Validates a returned block against the prediction, then records it
    // in the model and stamps it with a unique fill pattern.
    void commit(std::byte* ptr, Prediction pred, std::size_t size, std::size_t align) {
        if (!pred.fits) {
            check(ptr == nullptr);
            return;
        }
        check(ptr != nullptr);
        check(ptr == base_ + pred.aligned);
        check(reinterpret_cast<std::uintptr_t>(ptr) % align == 0);
        check(stack_.owns(ptr));
        check(stack_.owns(ptr + (size - 1)));

        const std::uint8_t fill = nextFill();
        std::memset(ptr, fill, size);
        live_.push_back(Live{pred.aligned, size, fill, ptr});

        offset_ = pred.aligned + size;
        stats_.total += size;
        stats_.current = offset_;
        stats_.peak = std::max(stats_.peak, offset_);
        ++stats_.count;
    }

    // Model-only rollback: mirrors what freeToMarker() must have done.
    void modelRollback(std::size_t newOffset) {
        offset_ = newOffset;
        stats_.current = offset_;
        std::erase_if(live_, [&](const Live& l) { return l.start + l.size > newOffset; });
        while (!markers_.empty() && markers_.back().get() > newOffset)
            markers_.pop_back();
    }

    std::uint8_t nextFill() {
        if (++fill_ == 0)
            fill_ = 1;
        return fill_;
    }

    [[nodiscard]] std::size_t pickAlign(Reader& r) const {
        return std::size_t{1} << (r.next() % (bufShift_ + 1));
    }

    void allocateChecked(std::size_t size, std::size_t align) {
        const Prediction pred = predict(size, align);
        std::byte* p = stack_.allocate(size, align);
        commit(p, pred, size, align);
    }

    // ---- verification -----------------------------------------------

    static void checkEmpty(StackT& s, const std::byte* base) {
        check(s.capacity() == 0);
        check(s.used() == 0);
        check(s.remaining() == 0);
        check(s.getMarker().get() == 0);
        check(!s.owns(base));
        check(s.allocate(1, 1) == nullptr);
        if constexpr (EnableStats) {
            const auto& st = s.getStats();
            check(st.totalAllocated_ == 0 && st.currentUsed_ == 0 && st.peakUsed_ == 0 &&
                  st.allocations_ == 0);
        }
    }

    void verify() const {
        check(stack_.capacity() == cap_);
        check(stack_.used() == offset_);
        check(stack_.remaining() == cap_ - offset_);
        check(stack_.getMarker().get() == offset_);

        if constexpr (EnableStats) {
            const auto& st = stack_.getStats();
            check(st.totalAllocated_ == stats_.total);
            check(st.currentUsed_ == stats_.current);
            check(st.peakUsed_ == stats_.peak);
            check(st.allocations_ == stats_.count);
        }

        // owns(): the buffer's first byte, last byte, and both
        // neighbours. base_ - 1 is formed via uintptr_t to avoid
        // forming a pointer before the start of the array.
        check(stack_.owns(base_));
        check(stack_.owns(base_ + (cap_ - 1)));
        check(!stack_.owns(base_ + cap_));
        check(!stack_.owns(reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(base_) -
                                                         1)));
        check(!stack_.owns(nullptr));

        // Markers must all still be valid (never ahead of the offset).
        for (const MarkerT& m : markers_)
            check(m.get() <= offset_);

        // Every live allocation must still hold its unique pattern.
        for (const Live& l : live_) {
            for (std::size_t i = 0; i < l.size; ++i)
                check(l.ptr[i] == static_cast<std::byte>(l.fill));
        }
    }

    // ---- operations -------------------------------------------------

    void allocateOp(Reader& r) {
        const std::size_t size = 1 + (r.next() % 96);
        allocateChecked(size, pickAlign(r));
    }

    // Sizes at/near SIZE_MAX must fail cleanly, without overflow.
    void allocateHugeOp(Reader& r) {
        const std::size_t size = SIZE_MAX - (r.next() % 16);
        allocateChecked(size, pickAlign(r));
    }

    // Requests exactly what is left: must succeed and leave 0 remaining.
    void allocateExactFitOp() {
        const std::size_t remaining = cap_ - offset_;
        if (remaining > 0)
            allocateChecked(remaining, 1);
    }

    // Requests one or more bytes past what is left: must fail.
    void allocateOverFitOp(Reader& r) {
        const std::size_t remaining = cap_ - offset_;
        allocateChecked(remaining + 1 + (r.next() % 3), pickAlign(r));
    }

    void markerOp() {
        const MarkerT m = stack_.getMarker();
        check(m.get() == offset_);
        markers_.push_back(m);
    }

    void rollbackOp(Reader& r) {
        if (markers_.empty())
            return;
        const MarkerT m = markers_[r.next() % markers_.size()];
        stack_.freeToMarker(m);
        modelRollback(m.get());
    }

    void scopeOp(Reader& r) {
        const std::size_t before = offset_;
        {
            StackScope<EnableStats> outer{stack_};
            const std::size_t n = 1 + (r.next() % 4);
            for (std::size_t i = 0; i < n; ++i) {
                const std::size_t size = 1 + (r.next() % 64);
                allocateChecked(size, pickAlign(r));
            }

            if ((r.next() & 1u) != 0) {
                const std::size_t mid = offset_;
                {
                    StackScope<EnableStats> inner{stack_};
                    allocateChecked(1 + (r.next() % 64), pickAlign(r));
                }
                modelRollback(mid);
                check(stack_.used() == mid);
            }
        }
        modelRollback(before);
        check(stack_.used() == before);
    }

    void resetOp() {
        stack_.reset();
        offset_ = 0;
        stats_ = ModelStats{};
        live_.clear();
        markers_.clear();
    }

    template <typename T, typename V> void createTyped(V value) {
        constexpr std::size_t align = alignof(T);
        // A per-call alignment above the buffer's would violate
        // allocate()'s documented precondition; skip rather than trip it.
        if (align > (std::size_t{1} << bufShift_))
            return;

        stack_.template destroy<T>(static_cast<T*>(nullptr)); // documented no-op

        const Prediction pred = predict(sizeof(T), align);
        T* obj = stack_.template create<T>(value);

        if (pred.fits) {
            check(obj != nullptr);
            check(valueOf(*obj) == static_cast<std::uint64_t>(value));
            if constexpr (std::is_same_v<T, Tracked>)
                check(Tracked::live == 1);
        } else {
            check(obj == nullptr);
            if constexpr (std::is_same_v<T, Tracked>)
                check(Tracked::live == 0);
        }

        stack_.destroy(obj);
        if constexpr (std::is_same_v<T, Tracked>)
            check(Tracked::live == 0);

        // Storage stays reserved after destroy(); account for it (and
        // stamp it) exactly like a plain allocation.
        commit(reinterpret_cast<std::byte*>(obj), pred, sizeof(T), align);
    }

    void createOp(Reader& r) {
        const std::uint8_t type = r.next() % 6;
        const std::uint8_t lo = r.next();
        const std::uint32_t value = (std::uint32_t{r.next()} << 8) | lo;

        switch (type) {
        case 0:
            createTyped<std::uint8_t>(static_cast<std::uint8_t>(value));
            break;
        case 1:
            createTyped<std::uint16_t>(static_cast<std::uint16_t>(value));
            break;
        case 2:
            createTyped<std::uint32_t>(value);
            break;
        case 3:
            createTyped<std::uint64_t>(std::uint64_t{value});
            break;
        case 4:
            createTyped<Tracked>(value);
            break;
        default:
            createTyped<Wide>(std::uint64_t{value});
            break;
        }
    }

    void moveOp(Reader& r) {
        switch (r.next() % 3) {
        case 0: { // move-construct away, then move-assign back
            StackT tmp{std::move(stack_)};
            checkEmpty(stack_, base_);
            stack_ = std::move(tmp);
            checkEmpty(tmp, base_);
            break;
        }
        case 1: { // move-assign into a live stack (which must free its own buffer)
            StackT other{std::size_t{1} + (r.next() % 64), 16};
            other = std::move(stack_);
            checkEmpty(stack_, base_);
            stack_ = std::move(other);
            checkEmpty(other, base_);
            break;
        }
        default: { // self-move-assignment must be a no-op
            StackT& self = stack_;
            stack_ = std::move(self);
            break;
        }
        }
        // Everything (offset, contents, stats) must have survived intact;
        // verify() in the caller confirms it.
    }

    StackT stack_;
    std::size_t cap_;
    unsigned bufShift_;
    std::byte* base_ = nullptr;

    std::size_t offset_ = 0;
    ModelStats stats_{};
    std::vector<Live> live_;
    std::vector<MarkerT> markers_;
    std::uint8_t fill_ = 0;
};

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size < 4)
        return 0;

    // Byte 0: bit 0 picks EnableStats, bits 1-3 the buffer alignment
    // (1..128). Byte 1 picks the capacity's magnitude and bytes 2-3 the
    // capacity itself, so runs range from a few bytes (constantly
    // exhausted) up to ~2 KiB.
    const bool withStats = (data[0] & 1u) != 0;
    const unsigned bufShift = (data[0] >> 1) % 8;
    const std::size_t limit = std::size_t{16} << (data[1] % 8);
    const std::size_t capacity = 1 + (((std::size_t{data[3]} << 8) | data[2]) % limit);

    Reader reader{data + 4, size - 4};

    if (withStats) {
        Harness<true> h{capacity, bufShift};
        h.run(reader);
    } else {
        Harness<false> h{capacity, bufShift};
        h.run(reader);
    }
    return 0;
}
