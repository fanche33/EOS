#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it
# set -e #Exit immediately if a command exits with a non-zero status

rmmod -f mydev
insmod mydev.ko

mknod /dev/mydev c 240 0

./writer FENG_FAN_CHE & #run in subshell
./reader 192.168.10.11 8000 /dev/mydev
