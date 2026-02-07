#!/bin/sh

set -x

cp -f task3/kaem.x86 rootfs/kaem.x86
cp -f task3/tools-seed-kaem.kaem rootfs/x86
cp -f task3/tools-mini-kaem.kaem rootfs/x86
cp -f task3/check-tools.kaem rootfs/x86
cp -f task3/tools-kaem.kaem rootfs/x86
cp -f task3/after.kaem rootfs/x86
cp -f task3/seed.kaem rootfs

sudo chroot --userspec=${USER}:${USER} rootfs /bootstrap-seeds/POSIX/x86/kaem-optional-seed
#sudo strace -f -o trace2.txt -e trace=open,openat,close,chmod,chdir,dup,fcntl,link,linkat,unlink,fork,execve chroot --userspec=${USER}:${USER} rootfs /bootstrap-seeds/POSIX/x86/kaem-optional-seed

