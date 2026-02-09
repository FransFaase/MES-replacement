#!/bin/sh

set -x

# Delete existing rootfs
rm -rf rootfs

# Create minimal directories
mkdir -p rootfs
mkdir -p rootfs/usr
mkdir -p rootfs/usr/bin
mkdir -p rootfs/tmp

# Create and fill bootstrap-seeds directory
mkdir -p rootfs/bootstrap-seeds
mkdir -p rootfs/bootstrap-seeds/POSIX
mkdir -p rootfs/bootstrap-seeds/POSIX/x86
cp -f src/kaem-minimal_s rootfs/bootstrap-seeds/POSIX/x86/kaem-optional-seed
cp -f src/hex0_s rootfs/bootstrap-seeds/POSIX/x86/hex0-seed
cp -f task3/kaem.x86 rootfs/kaem.x86

# Create and fill x86 specific directory for source files
mkdir -p rootfs/x86
mkdir -p rootfs/x86/artifact
cp -f -t rootfs/x86 \
    task3/tools-seed-kaem.kaem \
    task3/tools-mini-kaem.kaem \
    task3/check-tools.kaem \
    task3/tools-kaem.kaem \
    task3/after.kaem \
    src/hex0_s.hex0 \
    src/kaem-minimal_s.hex0 \
    src/hex2_s.hex0 \
    src/blood-elf_s.macro \
    src/blood-elf_s.blood_elf \
    src/M1_s.macro \
    src/M1_s.blood_elf \
    src/stack_c_s.M1 \
    src/stack_c_intro.M1 \
    M2libc/x86/ELF-x86-debug.hex2

# Create and fill directory for generic source files
mkdir -p rootfs/src
cp -f -t rootfs/src \
    src/stdlib.c \
    src/tcc_cc.sl \
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
    src/stack_c_interpreter.c

# Also add source files for checking
cp -f -t rootfs/src \
    src/equal.c \
    src/hex0.c \
    src/hex2.c \
    src/blood-elf.c \
    src/M1.c \
    src/stack_c.c \
    src/tcc_cc.c \
    src/kaem-minimal.c

cp -f task3/seed.kaem rootfs
cp -f task3/configurator.x86.checksums rootfs
cp -f task3/script-generator.x86.checksums rootfs

cp -r task3/steps rootfs
mkdir rootfs/external
cp -r task3/distfiles rootfs/external

