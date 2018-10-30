# To run with default args
./queue_spsc_fipc

# To specify number of transactions
./queue_spsc_fipc <transactions count>

# To specify producer and consumer count
./queue_spsc_fipc <consumer count> <producer count>

#Examples

for i in 1000 10000 100000 1000000 10000000 1000000000; do ./queue_spsc_fipc $i; done

This code is an implementation of the non-blocking queue introduced in:

Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms:
	https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf
Pseudo code:
	https://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
