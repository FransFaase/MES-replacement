#!/bin/sh

set -x

rm -rf rootfs

mkdir -p rootfs
mkdir -p rootfs/usr
mkdir -p rootfs/usr/bin
mkdir -p rootfs/usr/lib
mkdir -p rootfs/tmp

mkdir -p rootfs/bootstrap-seeds
mkdir -p rootfs/bootstrap-seeds/POSIX
mkdir -p rootfs/bootstrap-seeds/POSIX/x86
cp -f src/kaem-minimal_s rootfs/bootstrap-seeds/POSIX/x86/kaem-optional-seed
cp -f src/hex0_s rootfs/bootstrap-seeds/POSIX/x86/hex0-seed
cp -f task3/kaem.x86 rootfs/kaem.x86

mkdir -p rootfs/x86
mkdir -p rootfs/x86/artifact
cp -f task3/tools-seed-kaem.kaem rootfs/x86
cp -f task3/tools-mini-kaem.kaem rootfs/x86
cp -f task3/kaem.run rootfs/x86
cp -f -t rootfs/x86 \
    src/hex0_s.hex0 \
    src/kaem-minimal_s.hex0 \
    src/hex2_s.hex0 \
    src/blood-elf_s.macro \
    src/blood-elf_s.blood_elf \
    src/M1_s.macro \
    src/M1_s.blood_elf \
    src/stack_c_s.M1 \
    src/stack_c_intro.M1 \
    src/tcc_cc.sl

cp M2libc/x86/ELF-x86-debug.hex2 rootfs/x86

mkdir -p rootfs/src
cp -f -t rootfs/src \
    src/stdlib.c \
    src/tcc_cc.sl \
    src/kaem.c \
    src/catm.c \
    src/match.c \
    src/stack_c_interpreter.c


mkdir -p rootfs/x86/artifact

mkdir -p rootfs/mes
cp -rf -t rootfs/mes mes/*
mkdir -p rootfs/usr/include/mes
cp -rf -t rootfs/usr/include/mes mes/include/*

mkdir -p rootfs/steps
mkdir -p rootfs/steps/tcc-0.9.26/
mkdir -p rootfs/steps/tcc-0.9.26/build
cp -rf -t rootfs/steps/tcc-0.9.26/build tcc_sources/tcc-0.9.26-1147-gee75a10c

MES_PKG=rootfs/mes
TCC_PKG=rootfs/tcc
MES_ARCH=x86

# To replace a mkdir statement in kaem.run
mkdir -p rootfs/usr/lib/tcc
mkdir -p rootfs/usr/lib/mes/tcc

# Fill arch directory with architecture specific includes

mkdir -p ${MES_PKG}/include/arch
cp -f ${MES_PKG}/include/linux/${MES_ARCH}/kernel-stat.h ${MES_PKG}/include/arch/kernel-stat.h
cp -f ${MES_PKG}/include/linux/${MES_ARCH}/signal.h ${MES_PKG}/include/arch/signal.h
cp -f ${MES_PKG}/include/linux/${MES_ARCH}/syscall.h ${MES_PKG}/include/arch/syscall.h

