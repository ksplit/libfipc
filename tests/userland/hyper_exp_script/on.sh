#!/bin/bash
a=32
cd /sys/devices/system/cpu
for a in $(seq 32 63)
	do
		cd cpu$a
		sudo chmod 777 online
		echo 1 > online
		sudo chmod 644 online
		cd ..
	done

