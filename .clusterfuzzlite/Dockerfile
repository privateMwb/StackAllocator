# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:5add54bcdde77ae3427025db959a44f379ca64b6332bcddcc2766924582e47d9

COPY . $SRC/StackAllocator
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/StackAllocator
