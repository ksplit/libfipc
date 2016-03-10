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

We saw averages of 424-428 cycles for the round trip times between cpu1 and 
cpu3.

To run this example, after building the .ko, just insmod it. The round trip
times will be printed to the kernel logs.
