#!/bin/sh

set -x

mkdir -p rootfs/foreign
cp /usr/bin/bash rootfs/foreign
cp /usr/bin/echo rootfs/foreign
cp /usr/bin/ls rootfs/foreign
cp /lib/x86_64-linux-gnu/libtinfo.so.6 rootfs/foreign
cp /lib/x86_64-linux-gnu/libc.so.6 rootfs/foreign
cp /lib64/ld-linux-x86-64.so.2 rootfs/foreign


