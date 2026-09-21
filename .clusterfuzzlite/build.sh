#!/bin/bash -eu
# ============================================================
# .clusterfuzzlite/build.sh
#
# StackPro is header-only, so unlike a harness that needs to compile
# separate .cpp translation units first, this just compiles the fuzz
# target directly against the headers under include/.
#
# -UNDEBUG keeps the AP_PRE/AP_POST/AP_INVARIANT/AP_ASSERT contract
# macros (which map to assert()) live even if the build environment's
# default flags ever define NDEBUG, so a violated contract aborts the
# fuzzer instead of silently becoming undefined behavior.
#
# Add more `${SRC}/StackAllocator/fuzz/fuzz_*.cpp` harnesses here as
# they're added; each becomes its own $OUT binary.
# ============================================================

cd "${SRC}/StackAllocator"

$CXX $CXXFLAGS -std=c++20 -UNDEBUG \
  -I"${SRC}/StackAllocator/include" \
  fuzz/fuzz_stack.cpp \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_stack"
