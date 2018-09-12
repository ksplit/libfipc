Multi-Producer Multi-Consumer(MPMC) Queue with Spinlock
=================================================

In this test, we time enqueue and dequeue process of a queue protected with spinlocks.

A queue has two spinlocks which protect the enqueue process(tail) and the dequeue process(head) each.

The number of producers, consumers, and transaction can be set by command line arguemnts as mentioned below. 

```
# To run with default args
./queue_mpmc_spinlock

# To specify number of transactions
./queue_mpmc_spinlock <transactions count>

# To specify producer and consumer count
./queue_mpmc_spinlock <consumer count> <producer count>

#Examples

for i in 1000 10000 100000 1000000 10000000 1000000000; do ./queue_mpmc_spinlock $i; done
```

**Test Procedures:**
0. Set up producers and responders thread on separate processor, Px and Py.
1. Producers enqueue messages(nodes) into the queue.
2. Producers enqueue a halt message at the end of each queue to indicate the end of experiment to consumers.
3. Consumers dequeue messages(nodes) until they get halt messages.
4. Print out the latency which is a total enqueue/dequeue time divided by number of transactions.
