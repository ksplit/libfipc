rpc test
========

Overview
--------

This tests libfipc between two cores, in a caller-callee interaction.
One core, the caller, sends a message to the other core, the callee,
with a function identifier and arguments. The callee core invokes 
the corresponding function with the arguments and passes back the
return value to the caller in another message.

The caller does an null_invocation rpc TRANSACTIONS/2 times. This function
just returns a value and does no arithmetic on the callee side.

The caller does an add_6_nums rpc TRANASCTIONS/2 times. This function adds
6 numbers, but it does so by calling other functions (add_3_nums, add_nums).

Setup
-----

This example was ran on an Intel Xeon E5530 2.4 GHz machines. Make sure
you turn off TurboBoost in the BIOS, and make sure you turn up the
processor frequencies to 2.4 GHz.

We saw averages of 430 cycles for the null invocation round trip times
between cpu1 and cpu3.

To run this example, after building the .ko, just insmod it. The round trip
times will be printed to the kernel logs.
