# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:5d8ad2d0945830c9cae00441202025e0609eb2d846e351a3217cb5355c1f2109

COPY . $SRC/StackAllocator
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/StackAllocator
