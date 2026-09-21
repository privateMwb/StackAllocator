# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:47bcf5c42c4863af02843d16315014011571e31ece4ac9110a77a2e43949c5d2

COPY . $SRC/StackAllocator
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/StackAllocator
