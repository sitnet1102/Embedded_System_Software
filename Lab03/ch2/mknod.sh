#!/bin/sh

MODULE="ch2_dev"
MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)

mknod /dev/$MODULE c $MAJOR 0 0
mknod /dev/$MODULE c $MAJOR 0 1
mknod /dev/$MODULE c $MAJOR 0 2
