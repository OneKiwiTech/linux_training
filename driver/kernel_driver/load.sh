#!/bin/sh

module="raspy_io.ko"
device="io_dev"

insmod $module 

major=`cat /proc/devices | awk "{if(\\$2==\"$device\")print \\$1}"`
rm -f /dev/${device} c $major 0

mknod /dev/${device} c $major 0

chmod 666 /dev/${device}
