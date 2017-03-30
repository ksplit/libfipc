empty message test
==================

Overview
--------

This tests libfipc between two cores, in a caller-callee interaction.
One core, the caller, sends an empty message to the other core, the callee,
who responds with an empty message (only the message status changes).

Setup
-----

This example was ran on an Intel Xeon E5530 2.4 GHz machines. Make sure
you turn off TurboBoost in the BIOS, and make sure you turn up the
processor frequencies to 2.4 GHz.

To disable frequency scaling: (1) disable power management options in
BIOS, (2) if your system has intel_pstate drivers, disable them by appending
"intel_pstate=disable" to the “GRUB_CMDLINE_LINUX_DEFAULT” option in
"/etc/default/grub", executing "sudo update-grub" and then rebooting.
(3) Install the cpufrequtils package: "sudo apt-get install cpufrequtils"
(4) In the "/etc/init.d/cpufrequtils" file find "GOVERNOR" and set that line
to: “GOVERNOR=”performance””

To test your system, you can use ccbench at
"https://github.com/trigonak/ccbench" run "./ccbench -c2 -t7 -e2 -x 0 -y 1"
We saw averages of 60 cycles one way, and around 40 cycles the other way.

We saw averages of 424-428 cycles for the round trip times between cpu1 and 
cpu3.

To run this example, after building the .ko, just insmod it. The round trip
times will be printed to the kernel logs.