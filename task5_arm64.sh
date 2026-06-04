#!/bin/sh
# aarch64 analogue of task5_amd64.sh. On an x86_64 host, first register the
# qemu-aarch64 binfmt handler with the F (fix-binary) flag (qemu-user-static).

set -x

# Build sources
make -C src arm64_tests

# Delete existing rootfs
rm -rf rootfs

# Create minimal directories
mkdir -p rootfs
mkdir -p rootfs/usr
mkdir -p rootfs/usr/bin
mkdir -p rootfs/tmp

# Create and fill bootstrap-seeds directory
mkdir -p rootfs/bootstrap-seeds/POSIX/AArch64
cp -f src/kaem-minimal.arm64 rootfs/bootstrap-seeds/POSIX/AArch64/kaem-optional-seed
cp -f src/hex0.arm64         rootfs/bootstrap-seeds/POSIX/AArch64/hex0-seed

# Copy root kaem script
cp -f target_arm64/kaem.arm64 rootfs/kaem.arm64

# Create and fill arm64 specific directory with scripts and source files
mkdir -p rootfs/arm64
mkdir -p rootfs/arm64/artifact
cp -f -t rootfs/arm64 \
    target_arm64/tools-seed-kaem.kaem \
    target_arm64/tools-mini-kaem.kaem \
    target_arm64/check-tools.kaem \
    target_arm64/tools-kaem.kaem \
    target_arm64/after.kaem
# M2libc names this aarch64; rename to the ${ARCH}=arm64 convention
cp -f M2libc/aarch64/ELF-aarch64-debug.hex2 rootfs/arm64/ELF-arm64-debug.hex2
cp -f src/hex0.arm64_hex0           rootfs/arm64/hex0.hex0
cp -f src/kaem-minimal.arm64_hex0   rootfs/arm64/kaem-minimal.hex0
cp -f src/hex2.arm64_hex0           rootfs/arm64/hex2.hex0
cp -f src/blood-elf.macro_arm64     rootfs/arm64/blood-elf.macro
cp -f src/blood-elf.blood_elf_arm64 rootfs/arm64/blood-elf.blood_elf
cp -f src/M1.macro_arm64            rootfs/arm64/M1.macro
cp -f src/M1.blood_elf_arm64        rootfs/arm64/M1.blood_elf
cp -f src/stack_c_arm64.M1_arm64    rootfs/arm64/stack_c.M1
cp -f src/stack_c_intro_arm64.M1    rootfs/arm64/stack_c_intro.M1

# Create and fill directory for generic source files
mkdir -p rootfs/src
cp -f -t rootfs/src \
    src/stdlib.c \
    src/sys_syscall.h \
    src/tcc_cc.arm64.sl64 \
    src/kaem.c \
    src/catm.c \
    src/bootstrappable.c \
    src/match.c \
    src/mkdir.c \
    src/cp.c \
    src/chmod.c \
    src/rm.c \
    src/untar.c \
    src/ungz.c \
    src/unxz.c \
    src/unbz2.c \
    src/sha256sum.c \
    src/configurator.c \
    src/script-generator.c \
    src/stack_c_interpreter.c \
    src/arm64-asm.c

# Also add source files for checking
cp -f -t rootfs/src \
    src/equal.c \
    src/hex0.c \
    src/hex2.c \
    src/blood-elf.c \
    src/M1.c \
    src/stack_c_arm64.c \
    src/tcc_cc.c \
    src/kaem-minimal.c

# Copy scripts for processing steps
cp -f target_arm64/seed.kaem rootfs
cp -f target_arm64/configurator.arm64.checksums rootfs
cp -f target_arm64/script-generator.arm64.checksums rootfs

# Copy steps
cp -r steps rootfs
cp -f target_arm64/bootstrap.cfg rootfs/steps

# Copy the necessary distribution files
mkdir rootfs/external
cp -r distfiles rootfs/external

# Execute kaem scripts in change root environment
sudo chroot --userspec=$(id -u):$(id -g) rootfs \
    /bootstrap-seeds/POSIX/AArch64/kaem-optional-seed kaem.arm64
