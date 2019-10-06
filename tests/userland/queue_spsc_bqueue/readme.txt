# To run with default args
./queue_spsc_bqueue

# To specify number of transactions
./queue_spsc_bqueue <transactions count>

# To specify producer and consumer count
./queue_spsc_bqueue  <producer count> <consumer count>
#
./queue_spsc_bqueue <producer count> <consumer count> <number of transactions> <batch size>

#Examples
for i in 1000 10000 100000 1000000 10000000 1000000000; do ./queue_spsc_bqueue $i; done
