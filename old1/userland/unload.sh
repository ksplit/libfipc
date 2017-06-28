#!/bin/bash

echo "UNLOADING DRIVERS"
rmmod betaModule1
rmmod test

cd /users/sbauer/ipc/xcap/xcap/kernel
make clean;
make | grep "error" 

if [ $? == 0 ]; then
    echo "error"
    exit
fi

insmod betaModule1.ko;

cd 2
make clean

make | grep "error" 

if [ $? == 0 ]; then
    echo "error 2"
    exit
fi

insmod test.ko
cd ../

ls /dev/ --color | grep beta

../userland/uland 1
