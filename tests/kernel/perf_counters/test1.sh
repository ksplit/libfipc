insmod perf_counters.ko transactions=10000000 ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=10000000 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=10000000 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=10000000 ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=10000000 ev_idx=0x40,0x40 ev_msk=0x01,0x02
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=10000000 ev_idx=0x40,0x40 ev_msk=0x04,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=10000000 ev_idx=0x41,0x41 ev_msk=0x02,0x04
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=10000000 ev_idx=0x41 ev_msk=0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x40,0x40 ev_msk=0x01,0x02
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x40,0x40 ev_msk=0x04,0x08
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x41,0x41 ev_msk=0x02,0x04
rmmod perf_counters
dmesg -c
insmod perf_counters.ko transactions=1000000 ev_idx=0x41 ev_msk=0x08
rmmod perf_counters
dmesg -c

