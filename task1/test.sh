#!/bin/sh

# SPDX-FileCopyrightText: 2021-22 fosslinux <fosslinux@aussies.space>
#
# SPDX-License-Identifier: GPL-3.0-or-later

make tcc_g
make tcc_s

set -ex

ARCH=x86
MES_PKG=mes
pkg=tcc-0.9.26
BIN_DIFF=diff

# Vars

TCC_TAR=tcc_sources
TCC_PKG=tcc_sources
BINDIR=~/git/MES-replacement
LIBDIR=${BINDIR}/tcc_sources/lib
PREFIX=${BINDIR}/mes
TCC_BOOT=${BINDIR}/tcc_s

# Clean previous runs

rm -rf ${LIBDIR}/tcc

# Create config.h
${BINDIR}/catm ${MES_PKG}/include/mes/config.h
${BINDIR}/catm ${TCC_PKG}/config.h

if ${BINDIR}/match ${ARCH} x86; then
    MES_ARCH=x86
    TCC_TARGET_ARCH=I386
    MES_LIBC_SUFFIX=gcc
    HAVE_LONG_LONG=0
fi
if ${BINDIR}/match ${ARCH} amd64; then
    MES_ARCH=x86_64
    TCC_TARGET_ARCH=X86_64
    MES_LIBC_SUFFIX=gcc
    HAVE_LONG_LONG=1
fi
if ${BINDIR}/match ${ARCH} riscv64; then
    MES_ARCH=riscv64
    TCC_TARGET_ARCH=RISCV64
    MES_LIBC_SUFFIX=tcc
    HAVE_LONG_LONG=1
fi

echo MES_ARCH = ${MES_ARCH}

# Recompile the mes C library
cd ${MES_PKG}

# Create unified libc file
cd lib
# ${BINDIR}/catm ../unified-libc.c ctype/isalnum.c ctype/isalpha.c ctype/isascii.c ctype/iscntrl.c ctype/isdigit.c ctype/isgraph.c ctype/islower.c ctype/isnumber.c ctype/isprint.c ctype/ispunct.c ctype/isspace.c ctype/isupper.c ctype/isxdigit.c ctype/tolower.c ctype/toupper.c dirent/closedir.c dirent/__getdirentries.c dirent/opendir.c linux/readdir.c linux/access.c linux/brk.c linux/chdir.c linux/chmod.c linux/clock_gettime.c linux/close.c linux/dup2.c linux/dup.c linux/execve.c linux/fcntl.c linux/fork.c linux/fsync.c linux/fstat.c linux/_getcwd.c linux/getdents.c linux/getegid.c linux/geteuid.c linux/getgid.c linux/getpid.c linux/getppid.c linux/getrusage.c linux/gettimeofday.c linux/getuid.c linux/ioctl.c linux/ioctl3.c linux/kill.c linux/link.c linux/lseek.c linux/lstat.c linux/malloc.c linux/mkdir.c linux/mknod.c linux/nanosleep.c linux/_open3.c linux/pipe.c linux/_read.c linux/readlink.c linux/rename.c linux/rmdir.c linux/setgid.c linux/settimer.c linux/setuid.c linux/signal.c linux/sigprogmask.c linux/symlink.c linux/stat.c linux/time.c linux/unlink.c linux/waitpid.c linux/wait4.c linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/_exit.c linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/syscall.c linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/_write.c math/ceil.c math/fabs.c math/floor.c mes/abtod.c mes/abtol.c mes/__assert_fail.c mes/assert_msg.c mes/__buffered_read.c mes/__init_io.c mes/cast.c mes/dtoab.c mes/eputc.c mes/eputs.c mes/fdgetc.c mes/fdgets.c mes/fdputc.c mes/fdputs.c mes/fdungetc.c mes/globals.c mes/itoa.c mes/ltoab.c mes/ltoa.c mes/__mes_debug.c mes/mes_open.c mes/ntoab.c mes/oputc.c mes/oputs.c mes/search-path.c mes/ultoa.c mes/utoa.c posix/alarm.c posix/buffered-read.c posix/execl.c posix/execlp.c posix/execv.c posix/execvp.c posix/getcwd.c posix/getenv.c posix/isatty.c posix/mktemp.c posix/open.c posix/raise.c posix/sbrk.c posix/setenv.c posix/sleep.c posix/unsetenv.c posix/wait.c posix/write.c stdio/clearerr.c stdio/fclose.c stdio/fdopen.c stdio/feof.c stdio/ferror.c stdio/fflush.c stdio/fgetc.c stdio/fgets.c stdio/fileno.c stdio/fopen.c stdio/fprintf.c stdio/fputc.c stdio/fputs.c stdio/fread.c stdio/freopen.c stdio/fscanf.c stdio/fseek.c stdio/ftell.c stdio/fwrite.c stdio/getc.c stdio/getchar.c stdio/perror.c stdio/printf.c stdio/putc.c stdio/putchar.c stdio/remove.c stdio/snprintf.c stdio/sprintf.c stdio/sscanf.c stdio/ungetc.c stdio/vfprintf.c stdio/vfscanf.c stdio/vprintf.c stdio/vsnprintf.c stdio/vsprintf.c stdio/vsscanf.c stdlib/abort.c stdlib/abs.c stdlib/alloca.c stdlib/atexit.c stdlib/atof.c stdlib/atoi.c stdlib/atol.c stdlib/calloc.c stdlib/__exit.c stdlib/exit.c stdlib/free.c stdlib/mbstowcs.c stdlib/puts.c stdlib/qsort.c stdlib/realloc.c stdlib/strtod.c stdlib/strtof.c stdlib/strtol.c stdlib/strtold.c stdlib/strtoll.c stdlib/strtoul.c stdlib/strtoull.c string/bcmp.c string/bcopy.c string/bzero.c string/index.c string/memchr.c string/memcmp.c string/memcpy.c string/memmem.c string/memmove.c string/memset.c string/rindex.c string/strcat.c string/strchr.c string/strcmp.c string/strcpy.c string/strcspn.c string/strdup.c string/strerror.c string/strlen.c string/strlwr.c string/strncat.c string/strncmp.c string/strncpy.c string/strpbrk.c string/strrchr.c string/strspn.c string/strstr.c string/strupr.c stub/atan2.c stub/bsearch.c stub/chown.c stub/__cleanup.c stub/cos.c stub/ctime.c stub/exp.c stub/fpurge.c stub/freadahead.c stub/frexp.c stub/getgrgid.c stub/getgrnam.c stub/getlogin.c stub/getpgid.c stub/getpgrp.c stub/getpwnam.c stub/getpwuid.c stub/gmtime.c stub/ldexp.c stub/localtime.c stub/log.c stub/mktime.c stub/modf.c stub/mprotect.c stub/pclose.c stub/popen.c stub/pow.c stub/rand.c stub/rewind.c stub/setbuf.c stub/setgrent.c stub/setlocale.c stub/setvbuf.c stub/sigaction.c stub/sigaddset.c stub/sigblock.c stub/sigdelset.c stub/sigemptyset.c stub/sigsetmask.c stub/sin.c stub/sys_siglist.c stub/system.c stub/sqrt.c stub/strftime.c stub/times.c stub/ttyname.c stub/umask.c stub/utime.c ${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/setjmp.c
cd ..
pwd

${BINDIR}/tcc_g -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crt1_g.o lib/linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/crt1.c
${TCC_BOOT} -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crt1.o lib/linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/crt1.c
${BINDIR}/stack_c_interpreter ${BINDIR}/tcc.sl -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crt1_s_i.o lib/linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/crt1.c

${BIN_DIFF} ${LIBDIR}/crt1.o ${LIBDIR}/crt1_g.o
${BIN_DIFF} ${LIBDIR}/crt1.o ${LIBDIR}/crt1_s_i.o

${BINDIR}/catm ${LIBDIR}/crtn.o
${BINDIR}/catm ${LIBDIR}/crti.o
if ${BINDIR}/match ${ARCH} x86; then
    # crtn.o
    ${BINDIR}/tcc_g -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crtn_g.o lib/linux/${MES_ARCH}-mes-gcc/crtn.c
    ${TCC_BOOT} -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crtn.o lib/linux/${MES_ARCH}-mes-gcc/crtn.c
    ${BIN_DIFF} ${LIBDIR}/crtn.o ${LIBDIR}/crtn_g.o

    # crti.o
    ${BINDIR}/tcc_g -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crti_g.o lib/linux/${MES_ARCH}-mes-gcc/crti.c
    ${TCC_BOOT} -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crti.o lib/linux/${MES_ARCH}-mes-gcc/crti.c
    ${BIN_DIFF} ${LIBDIR}/crti.o ${LIBDIR}/crti_g.o
fi

# libc+gcc.a
${BINDIR}/tcc_g -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o unified-libc_g.o unified-libc.c
${TCC_BOOT} -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o unified-libc.o unified-libc.c
${BIN_DIFF} unified-libc.o unified-libc_g.o
${BINDIR}/tcc_g -ar cr ${LIBDIR}/libc_g.a unified-libc.o
${TCC_BOOT} -ar cr ${LIBDIR}/libc.a unified-libc.o
${BIN_DIFF} ${LIBDIR}/libc.a ${LIBDIR}/libc_g.a

# libtcc1.a
mkdir ${LIBDIR}/tcc
${BINDIR}/tcc_g -c -D HAVE_CONFIG_H=1 -D HAVE_LONG_LONG=${HAVE_LONG_LONG} -D HAVE_FLOAT=0 -I include -I include/linux/${MES_ARCH} -o libtcc1_g.o lib/libtcc1.c >~/git/MES-replacement/out_g.txt
${TCC_BOOT} -c -D HAVE_CONFIG_H=1 -D HAVE_LONG_LONG=${HAVE_LONG_LONG} -D HAVE_FLOAT=0 -I include -I include/linux/${MES_ARCH} -o libtcc1.o lib/libtcc1.c >~/git/MES-replacement/out.txt
${BIN_DIFF} libtcc1.o libtcc1_g.o
objdump -xd libtcc1_g.o  >>~/git/MES-replacement/out_g.txt
objdump -xd libtcc1.o >>~/git/MES-replacement/out.txt
${BINDIR}/tcc_g -ar cr ${LIBDIR}/tcc/libtcc1_g.a libtcc1.o
${TCC_BOOT} -ar cr ${LIBDIR}/tcc/libtcc1.a libtcc1.o
${BIN_DIFF} ${LIBDIR}/tcc/libtcc1.a ${LIBDIR}/tcc/libtcc1_g.a


# libgetopt.a
${BINDIR}/tcc_g -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o getopt_g.o lib/posix/getopt.c
${TCC_BOOT} -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o getopt.o lib/posix/getopt.c
diff getopt_g.o getopt_g.o
${BINDIR}/tcc_g  -ar cr ${LIBDIR}/libgetopt_g.a getopt.o
${TCC_BOOT} -ar cr ${LIBDIR}/libgetopt.a getopt.o
${BIN_DIFF} ${LIBDIR}/libgetopt.a ${LIBDIR}/libgetopt_g.a

cd ../${TCC_PKG}

# boot0 (ref comments here for all boot*)
# compile
${BINDIR}/tcc_g \
    -g \
    -v \
    -static \
    -o tcc-boot0_g \
    -D BOOTSTRAP=1 \
    -D HAVE_FLOAT=0 \
    -D HAVE_BITFIELD=0 \
    -D HAVE_LONG_LONG=${HAVE_LONG_LONG} \
    -D HAVE_SETJMP=0 \
    -I . \
    -I ${PREFIX}/include \
    -I ${PREFIX}/include/mes \
    -D TCC_TARGET_${TCC_TARGET_ARCH}=1 \
    -D CONFIG_TCCDIR=\"${LIBDIR}/tcc\" \
    -D CONFIG_TCC_CRTPREFIX=\"${LIBDIR}\" \
    -D CONFIG_TCC_ELFINTERP=\"/mes/loader\" \
    -D CONFIG_TCC_LIBPATHS=\"${LIBDIR}:${LIBDIR}/tcc\" \
    -D CONFIG_TCC_SYSINCLUDEPATHS=\"${PREFIX}/include:${PREFIX}/include/mes\" \
    -D TCC_LIBGCC=\"${LIBDIR}/libc.a\" \
    -D TCC_LIBTCC1=\"libtcc1.a\" \
    -D CONFIG_TCCBOOT=1 \
    -D CONFIG_TCC_STATIC=1 \
    -D CONFIG_USE_LIBGCC=1 \
    -D TCC_VERSION=\"0.9.26\" \
    -D ONE_SOURCE=1 \
    -L . \
    -L ${LIBDIR} \
    tcc.c >/dev/null
${TCC_BOOT} \
    -g \
    -v \
    -static \
    -o tcc-boot0 \
    -D BOOTSTRAP=1 \
    -D HAVE_FLOAT=0 \
    -D HAVE_BITFIELD=0 \
    -D HAVE_LONG_LONG=${HAVE_LONG_LONG} \
    -D HAVE_SETJMP=0 \
    -I . \
    -I ${PREFIX}/include \
    -I ${PREFIX}/include/mes \
    -D TCC_TARGET_${TCC_TARGET_ARCH}=1 \
    -D CONFIG_TCCDIR=\"${LIBDIR}/tcc\" \
    -D CONFIG_TCC_CRTPREFIX=\"${LIBDIR}\" \
    -D CONFIG_TCC_ELFINTERP=\"/mes/loader\" \
    -D CONFIG_TCC_LIBPATHS=\"${LIBDIR}:${LIBDIR}/tcc\" \
    -D CONFIG_TCC_SYSINCLUDEPATHS=\"${PREFIX}/include:${PREFIX}/include/mes\" \
    -D TCC_LIBGCC=\"${LIBDIR}/libc.a\" \
    -D TCC_LIBTCC1=\"libtcc1.a\" \
    -D CONFIG_TCCBOOT=1 \
    -D CONFIG_TCC_STATIC=1 \
    -D CONFIG_USE_LIBGCC=1 \
    -D TCC_VERSION=\"0.9.26\" \
    -D ONE_SOURCE=1 \
    -L . \
    -L ${LIBDIR} \
    tcc.c >/dev/null
${BIN_DIFF} tcc-boot0 tcc-boot0_g

# Install
cp tcc-boot0 ${BINDIR}/
chmod 755 ${BINDIR}/tcc-boot0
cd ../${MES_PKG}
# Recompile libc: crt{1,n,i}, libtcc.a, libc.a
${BINDIR}/tcc-boot0 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crt1.o lib/linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/crt1.c
if ${BINDIR}/match ${ARCH} x86; then
    ${BINDIR}/tcc-boot0 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crtn.o lib/linux/${MES_ARCH}-mes-gcc/crtn.c
    ${BINDIR}/tcc-boot0 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crti.o lib/linux/${MES_ARCH}-mes-gcc/crti.c
fi

${BINDIR}/tcc-boot0 -c -D HAVE_CONFIG_H=1 -D HAVE_LONG_LONG=1 -D HAVE_FLOAT=1 -I include -I include/linux/${MES_ARCH} -o libtcc1.o lib/libtcc1.c
if ${BINDIR}/match ${ARCH} riscv64; then
    ${BINDIR}/tcc-boot0 -c -D HAVE_CONFIG_H=1 -D HAVE_LONG_LONG=1 -D HAVE_FLOAT=1 -I include -I include/linux/${MES_ARCH} -o lib-arm64.o ../${TCC_PKG}/lib/lib-arm64.c
    ${BINDIR}/tcc-boot0 -ar cr ${LIBDIR}/tcc/libtcc1.a libtcc1.o lib-arm64.o
else
    ${BINDIR}/tcc-boot0 -ar cr ${LIBDIR}/tcc/libtcc1.a libtcc1.o
fi

${BINDIR}/tcc-boot0 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o unified-libc.o unified-libc.c
${BINDIR}/tcc-boot0 -ar cr ${LIBDIR}/libc.a unified-libc.o
cd ../${TCC_PKG}

# Test boot0
${BINDIR}/tcc-boot0 -version

# boot1
${BINDIR}/tcc-boot0 \
    -g \
    -v \
    -static \
    -o tcc-boot1 \
    -D BOOTSTRAP=1 \
    -D HAVE_FLOAT=1 \
    -D HAVE_BITFIELD=1 \
    -D HAVE_LONG_LONG=1 \
    -D HAVE_SETJMP=1 \
    -I . \
    -I ${PREFIX}/include/mes \
    -I ${PREFIX}/include \
    -D TCC_TARGET_${TCC_TARGET_ARCH}=1 \
    -D CONFIG_TCCDIR=\"${LIBDIR}/tcc\" \
    -D CONFIG_TCC_CRTPREFIX=\"${LIBDIR}\" \
    -D CONFIG_TCC_ELFINTERP=\"/mes/loader\" \
    -D CONFIG_TCC_LIBPATHS=\"${LIBDIR}:${LIBDIR}/tcc\" \
    -D CONFIG_TCC_SYSINCLUDEPATHS=\"${PREFIX}/include/mes\" \
    -D TCC_LIBGCC=\"${LIBDIR}/libc.a\" \
    -D TCC_LIBTCC1=\"libtcc1.a\" \
    -D CONFIG_TCCBOOT=1 \
    -D CONFIG_TCC_STATIC=1 \
    -D CONFIG_USE_LIBGCC=1 \
    -D TCC_VERSION=\"0.9.26\" \
    -D ONE_SOURCE=1 \
    -L . \
    tcc.c
cp tcc-boot1 ${BINDIR}
chmod 755 ${BINDIR}/tcc-boot1
cd ../${MES_PKG}
${BINDIR}/tcc-boot1 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crt1.o lib/linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/crt1.c
if ${BINDIR}/match ${ARCH} x86; then
    ${BINDIR}/tcc-boot1 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crtn.o lib/linux/${MES_ARCH}-mes-gcc/crtn.c
    ${BINDIR}/tcc-boot1 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crti.o lib/linux/${MES_ARCH}-mes-gcc/crti.c
fi

${BINDIR}/tcc-boot1 -c -D HAVE_CONFIG_H=1 -D HAVE_LONG_LONG=1 -D HAVE_FLOAT=1 -I include -I include/linux/${MES_ARCH} -o libtcc1.o lib/libtcc1.c
if ${BINDIR}/match ${ARCH} riscv64; then
    ${BINDIR}/tcc-boot1 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o lib-arm64.o ../${TCC_PKG}/lib/lib-arm64.c
    ${BINDIR}/tcc-boot1 -ar cr ${LIBDIR}/tcc/libtcc1.a libtcc1.o lib-arm64.o
else
    ${BINDIR}/tcc-boot1 -ar cr ${LIBDIR}/tcc/libtcc1.a libtcc1.o
fi

${BINDIR}/tcc-boot1 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o unified-libc.o unified-libc.c
${BINDIR}/tcc-boot1 -ar cr ${LIBDIR}/libc.a unified-libc.o
cd ../${TCC_PKG}

# Test boot1
${BINDIR}/tcc-boot1 -version

# boot2
${BINDIR}/tcc-boot1 \
    -g \
    -v \
    -static \
    -o tcc-boot2 \
    -D BOOTSTRAP=1 \
    -D HAVE_BITFIELD=1 \
    -D HAVE_FLOAT=1 \
    -D HAVE_LONG_LONG=1 \
    -D HAVE_SETJMP=1 \
    -I . \
    -I ${PREFIX}/include/mes \
    -I ${PREFIX}/include \
    -D TCC_TARGET_${TCC_TARGET_ARCH}=1 \
    -D CONFIG_TCCDIR=\"${LIBDIR}/tcc\" \
    -D CONFIG_TCC_CRTPREFIX=\"${LIBDIR}\" \
    -D CONFIG_TCC_ELFINTERP=\"/mes/loader\" \
    -D CONFIG_TCC_LIBPATHS=\"${LIBDIR}:${LIBDIR}/tcc\" \
    -D CONFIG_TCC_SYSINCLUDEPATHS=\"${PREFIX}/include/mes\" \
    -D TCC_LIBGCC=\"${LIBDIR}/libc.a\" \
    -D TCC_LIBTCC1=\"libtcc1.a\" \
    -D CONFIG_TCCBOOT=1 \
    -D CONFIG_TCC_STATIC=1 \
    -D CONFIG_USE_LIBGCC=1 \
    -D TCC_VERSION=\"0.9.26\" \
    -D ONE_SOURCE=1 \
    -L . \
    tcc.c
cp tcc-boot2 ${BINDIR}
chmod 755 ${BINDIR}/tcc-boot2
cd ../${MES_PKG}
${BINDIR}/tcc-boot2 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crt1.o lib/linux/${MES_ARCH}-mes-${MES_LIBC_SUFFIX}/crt1.c
if ${BINDIR}/match ${ARCH} x86; then
    ${BINDIR}/tcc-boot2 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crtn.o lib/linux/${MES_ARCH}-mes-gcc/crtn.c
    ${BINDIR}/tcc-boot2 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o ${LIBDIR}/crti.o lib/linux/${MES_ARCH}-mes-gcc/crti.c
fi

${BINDIR}/tcc-boot2 -c -D HAVE_CONFIG_H=1 -D HAVE_LONG_LONG=1 -D HAVE_FLOAT=1 -I include -I include/linux/${MES_ARCH} -o libtcc1.o lib/libtcc1.c
if ${BINDIR}/match ${ARCH} riscv64; then
    ${BINDIR}/tcc-boot2 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o lib-arm64.o ../${TCC_PKG}/lib/lib-arm64.c
    ${BINDIR}/tcc-boot2 -ar cr ${LIBDIR}/tcc/libtcc1.a libtcc1.o lib-arm64.o
else
    ${BINDIR}/tcc-boot2 -ar cr ${LIBDIR}/tcc/libtcc1.a libtcc1.o
fi

${BINDIR}/tcc-boot2 -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} -o unified-libc.o unified-libc.c
${BINDIR}/tcc-boot2 -ar cr ${LIBDIR}/libc.a unified-libc.o
cd ../${TCC_PKG}

# Test boot2
${BINDIR}/tcc-boot2 -version

# We have our final tcc 0.9.26!
cp ${BINDIR}/tcc-boot2 ${BINDIR}/tcc
chmod 755 ${BINDIR}/tcc
rm ${BINDIR}/tcc-boot2
cp ${BINDIR}/tcc ${BINDIR}/tcc-0.9.26
chmod 755 ${BINDIR}/tcc-0.9.26

# Also recompile getopt, we don't need to do this during the boot* stages
# because nothing is linked against it
cd ../${MES_PKG}
${BINDIR}/tcc -c -D HAVE_CONFIG_H=1 -I include -I include/linux/${MES_ARCH} lib/posix/getopt.c
${BINDIR}/tcc -ar cr ${LIBDIR}/libgetopt.a getopt.o

cd ..

pwd

# Checksums
if ${BINDIR}/match x${UPDATE_CHECKSUMS} xTrue; then
    ${BINDIR}/sha256sum -o ${pkg}.${ARCH}.checksums \
        ${BINDIR}/tcc_s \
        ${BINDIR}/tcc-boot0 \
        ${BINDIR}/tcc-boot1 \
        ${BINDIR}/tcc \
        ${LIBDIR}/mes/libc.a \
        ${LIBDIR}/mes/libgetopt.a \
        ${LIBDIR}/mes/crt1.o \
        ${LIBDIR}/mes/crti.o \
        ${LIBDIR}/mes/crtn.o \
        ${LIBDIR}/mes/tcc/libtcc1.a

    cp ${pkg}.${ARCH}.checksums ${SRCDIR}
else
    ${BINDIR}/sha256sum -c ${pkg}.${ARCH}.checksums
fi
