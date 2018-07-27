# To run with default args
./queue_spsc_fipc

# To specify number of transactions
./queue_spsc_fipc <transactions count>

# To specify producer and consumer count
./queue_spsc_fipc <consumer count> <producer count>

#Examples

for i in 1000 10000 100000 1000000 10000000 1000000000; do ./queue_spsc_fipc $i; done
