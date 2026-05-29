#!/bin/sh
# Fully-aarch64 chroot bootstrap of tcc 0.9.26. Analogous to task1.sh (x86) but
# every binary that runs inside the chroot is an aarch64 static ELF, executed
# natively through the host's qemu-aarch64 binfmt handler. The handler is
# registered with the F (fix-binary) flag, so the qemu interpreter is preloaded
# at registration time and NO qemu binary needs to live inside the rootfs.
#
# The seed/cross tools (tcc_cc, stack_c_arm64, blood-elf, M1, hex2) are built by
# `make arm64`; on an x86_64 host they are x86_64 ELFs that only ever EMIT
# aarch64 artifacts (never run in the chroot). On a native aarch64 host the same
# `make arm64` uses the native gcc, so even the generators are aarch64 — nothing
# amd64 anywhere. See report-aarch64.md / the build_arm64.sh harness.

set -ex

ROOT=$(cd "$(dirname "$0")" && pwd)
MES_ARCH=aarch64
TCC_PKG=tcc-0.9.26-1147-gee75a10c

#############################################################################
# Phase A: build the seed/cross tools and the aarch64 seed artifacts
#############################################################################
make -C src tcc_cc stack_c_arm64 blood-elf M1 hex2
make -C src arm64
gcc -include fcntl.h -include unistd.h -o src/simple-patch \
    steps/simple-patch-1.0/src/simple-patch.c

#############################################################################
# Phase B: assemble the rootfs
#############################################################################
rm -rf rootfs
mkdir -p rootfs/usr/bin
mkdir -p rootfs/usr/lib/mes/tcc
mkdir -p rootfs/usr/include/mes
mkdir -p rootfs/tmp
mkdir -p rootfs/src
mkdir -p rootfs/arm64/artifact

# Bootstrap seeds (aarch64 static ELFs: the irreducible starting binaries).
mkdir -p rootfs/bootstrap-seeds/POSIX/AArch64
cp -f src/kaem-minimal.arm64 rootfs/bootstrap-seeds/POSIX/AArch64/kaem-optional-seed
cp -f src/hex0.arm64         rootfs/bootstrap-seeds/POSIX/AArch64/hex0-seed

# kaem scripts.
cp -f target_arm64/kaem.arm64           rootfs/kaem.arm64
cp -f target_arm64/tools-seed-kaem.kaem rootfs/arm64/
cp -f target_arm64/tools-mini-kaem.kaem rootfs/arm64/
cp -f target_arm64/kaem.run             rootfs/arm64/

# aarch64 seed assemblies (hex0 listings, macro+blood_elf seeds, prebuilt M1/sl).
cp -f src/hex0.arm64_hex0          rootfs/arm64/hex0.arm64_hex0
cp -f src/kaem-minimal.arm64_hex0  rootfs/arm64/kaem-minimal.arm64_hex0
cp -f src/hex2.arm64_hex0          rootfs/arm64/hex2.arm64_hex0
cp -f src/blood-elf.macro_arm64    rootfs/arm64/blood-elf.macro_arm64
cp -f src/blood-elf.blood_elf_arm64 rootfs/arm64/blood-elf.blood_elf_arm64
cp -f src/M1.macro_arm64           rootfs/arm64/M1.macro_arm64
cp -f src/M1.blood_elf_arm64       rootfs/arm64/M1.blood_elf_arm64
cp -f src/stack_c_arm64.M1_arm64   rootfs/arm64/stack_c.M1
cp -f src/stack_c_intro_arm64.M1   rootfs/arm64/stack_c_intro_arm64.M1
cp -f src/tcc_cc.sl64a             rootfs/arm64/tcc_cc.sl
cp -f M2libc/aarch64/ELF-aarch64-debug.hex2 rootfs/arm64/ELF-aarch64-debug.hex2

# Generic C sources compiled inside the chroot.
cp -f -t rootfs/src \
    src/stdlib.c \
    src/sys_syscall.h \
    src/kaem.c \
    src/catm.c \
    src/bootstrappable.c \
    src/equal.c

# GNU mes sources (the submodule already carries the aarch64 libc + headers, so
# no overlay is needed). Mirrors task1.sh.
mkdir -p rootfs/mes
cp -rf -t rootfs/mes mes/*
cp -rf -t rootfs/usr/include/mes mes/include/*

# Architecture-specific includes for the libc build (arch/{kernel-stat,signal,syscall}.h).
MES_PKG=rootfs/mes
mkdir -p ${MES_PKG}/include/arch
cp -f ${MES_PKG}/include/linux/${MES_ARCH}/kernel-stat.h ${MES_PKG}/include/arch/kernel-stat.h
cp -f ${MES_PKG}/include/linux/${MES_ARCH}/signal.h      ${MES_PKG}/include/arch/signal.h
cp -f ${MES_PKG}/include/linux/${MES_ARCH}/syscall.h     ${MES_PKG}/include/arch/syscall.h

# Tiny C Compiler sources, patched host-side (same simple-patches as build_arm64.sh).
mkdir -p rootfs/steps/tcc-0.9.26/build
tar -xzf distfiles/tcc-0.9.26.tar.gz -C rootfs/steps/tcc-0.9.26/build
SRCDIR=rootfs/steps/tcc-0.9.26/build/${TCC_PKG}

src/simple-patch ${SRCDIR}/tcctools.c \
    steps/tcc-0.9.26/simple-patches/remove-fileopen.before \
    steps/tcc-0.9.26/simple-patches/remove-fileopen.after
src/simple-patch ${SRCDIR}/tcctools.c \
    steps/tcc-0.9.26/simple-patches/addback-fileopen.before \
    steps/tcc-0.9.26/simple-patches/addback-fileopen.after

# arm64 inline-asm: drop in the minimal data-directive assembler and include it.
cp src/arm64-asm.c ${SRCDIR}/arm64-asm.c
src/simple-patch ${SRCDIR}/tcc.h \
    steps/tcc-0.9.26/simple-patches/arm64-asm-defs.before \
    steps/tcc-0.9.26/simple-patches/arm64-asm-defs.after
src/simple-patch ${SRCDIR}/libtcc.c \
    steps/tcc-0.9.26/simple-patches/arm64-asm-include.before \
    steps/tcc-0.9.26/simple-patches/arm64-asm-include.after

# parse_builtin_params: rewrite the loop the seed tcc_cc miscompiles (needed for
# the arm64 __va_start/__va_arg builtins).
src/simple-patch ${SRCDIR}/tccgen.c \
    steps/tcc-0.9.26/simple-patches/arm64-va-builtin-loop.before \
    steps/tcc-0.9.26/simple-patches/arm64-va-builtin-loop.after

#############################################################################
# Phase C: run the bootstrap inside the chroot
#############################################################################
# Numeric uid:gid (the rootfs has no /etc/passwd, so a username can't be resolved).
sudo chroot --userspec=$(id -u):$(id -g) rootfs \
    /bootstrap-seeds/POSIX/AArch64/kaem-optional-seed kaem.arm64
