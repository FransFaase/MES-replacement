#!/bin/sh

set -x

# Build sources
make -C src

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
mkdir -p rootfs/bootstrap-seeds/POSIX/AMD64
cp -f src/kaem-minimal.amd64 rootfs/bootstrap-seeds/POSIX/AMD64/kaem-optional-seed
cp -f src/hex0.amd64 rootfs/bootstrap-seeds/POSIX/AMD64/hex0-seed

# Copy root kaem script
cp -f target_amd64/kaem.amd64 rootfs/kaem.amd64

# Create and fill amd64 specific directory with scripts and source files
mkdir -p rootfs/amd64
mkdir -p rootfs/amd64/artifact
cp -f -t rootfs/amd64 \
    target_amd64/tools-seed-kaem.kaem \
    target_amd64/tools-mini-kaem.kaem \
    target_amd64/check-tools.kaem \
    target_amd64/tools-kaem.kaem \
    target_amd64/after.kaem \
    M2libc/amd64/ELF-amd64-debug.hex2
cp -f src/hex0.amd64_hex0 rootfs/amd64/hex0.hex0
cp -f src/kaem-minimal.amd64_hex0 rootfs/amd64/kaem-minimal.hex0
cp -f src/hex2.amd64_hex0 rootfs/amd64/hex2.hex0
cp -f src/blood-elf.macro_amd64 rootfs/amd64/blood-elf.macro
cp -f src/blood-elf.blood_elf_amd64 rootfs/amd64/blood-elf.blood_elf
cp -f src/M1.macro_amd64 rootfs/amd64/M1.macro
cp -f src/M1.blood_elf_amd64 rootfs/amd64/M1.blood_elf
cp -f src/stack_c_amd64.M1_amd64 rootfs/amd64/stack_c.M1    
cp -f src/stack_c_intro_amd64.M1 rootfs/amd64/stack_c_intro.M1

# Create and fill directory for generic source files
mkdir -p rootfs/src
cp -f -t rootfs/src \
    src/stdlib.c \
    src/sys_syscall.h \
    src/tcc_cc.sl64 \
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
    src/stack_c_amd64.c \
    src/tcc_cc.c \
    src/kaem-minimal.c

# Copy scripts for processing steps
cp -f target_amd64/seed.kaem rootfs
cp -f target_amd64/configurator.amd64.checksums rootfs
cp -f target_amd64/script-generator.amd64.checksums rootfs

# Copy steps
cp -r steps rootfs
cp -f target_amd64/bootstrap.cfg rootfs/steps

# Copy the necessary distribution files
mkdir rootfs/external
cp -r distfiles rootfs/external

# Execute kaem scripts in change root environment
#sudo chroot --userspec=${USER}:${USER} rootfs /bootstrap-seeds/POSIX/AMD64/kaem-optional-seed kaem.amd64

# Alternative with strace to generate trace.txt file that can be used
# to generate documentation with the help op scan_trace.cpp
sudo strace -f -o trace.txt -e trace=open,openat,close,chmod,chdir,dup,fcntl,link,linkat,unlink,fork,execve chroot --userspec=${USER}:${USER} rootfs /bootstrap-seeds/POSIX/AMD64/kaem-optional-seed kaem.amd64

