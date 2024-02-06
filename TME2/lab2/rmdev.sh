#!/bin/sh
MODULE=$1
/sbin/rmmod $MODULE || exit 1
rm -f /dev/$MODULE

echo "=> Device /dev/$MODULE removed\n"