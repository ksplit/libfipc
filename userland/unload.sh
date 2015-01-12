#!/bin/bash

echo "UNLOADING DRIVERS"
rmmod betaModule
rmmod betaModule2

cd /home/scotty/research/xcap/norepo/kernel;
make clean;
make | grep "error" 

if [ $? == 0 ]; then
    echo "error"
    exit
fi

insmod betaModule.ko;

cd 2
make clean

make | grep "error" 

if [ $? == 0 ]; then
    echo "error 2"
    exit
fi

insmod betaModule2.ko


ls /dev/ --color | grep beta
