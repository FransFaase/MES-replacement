#!/bin/sh

set -x

cp -f task3/kaem.x86 rootfs/kaem.x86
cp -f task3/tools-seed-kaem.kaem rootfs/x86
cp -f task3/tools-mini-kaem.kaem rootfs/x86
cp -f task3/kaem.run rootfs/x86

sudo chroot --userspec=${USER}:${USER} rootfs /bootstrap-seeds/POSIX/x86/kaem-optional-seed
#sudo strace -f -o trace2.txt -e trace=open,openat,close,chmod,chdir,dup,fcntl,link,linkat,unlink,fork,execve chroot --userspec=${USER}:${USER} rootfs /bootstrap-seeds/POSIX/x86/kaem-optional-seed

