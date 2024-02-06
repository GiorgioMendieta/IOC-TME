#!/bin/sh
MODULE=$1
shift
/sbin/insmod ./$MODULE.ko $* || exit 1
rm -f /dev/$MODULE
MAJOR=$(awk "\$2==\"$MODULE\" {print \$1;exit}" /proc/devices)
mknod /dev/$MODULE c $major 0
chmod 666 /dev/$MODULE

echo "=> Device /dev/$MODULE created with major=$MAJOR\n"