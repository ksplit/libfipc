## ##########################################
## Experiment 5 Test Script
##
##
## Results found at:
## https://docs.google.com/spreadsheets/d/1wGxyS6dr4uzxQdbUtih1LnAezv6lDzGh0fN_5jkIBzQ/edit?usp=sharing
## ##########################################

outfile=exp5.out

run()
{
	module=$1
	shift
	insmod $module/$module.ko $*
	rmmod $module
	dmesg -c >> $outfile
}

dmesg -C
rm -f $outfile

echo "perf_bare_1" >> $outfile
run perf_bare_1 ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_1 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_1 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
run perf_bare_1 ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
run perf_bare_1 ev_idx=0x40,0x40 ev_msk=0x01,0x02
run perf_bare_1 ev_idx=0x40,0x40 ev_msk=0x04,0x08
run perf_bare_1 ev_idx=0x41,0x41 ev_msk=0x02,0x04
run perf_bare_1 ev_idx=0x41 ev_msk=0x08

echo "perf_bare_1_reset" >> $outfile
run perf_bare_1_reset ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_1_reset ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_1_reset ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
run perf_bare_1_reset ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
run perf_bare_1_reset ev_idx=0x40,0x40 ev_msk=0x01,0x02
run perf_bare_1_reset ev_idx=0x40,0x40 ev_msk=0x04,0x08
run perf_bare_1_reset ev_idx=0x41,0x41 ev_msk=0x02,0x04
run perf_bare_1_reset ev_idx=0x41 ev_msk=0x08

echo "perf_bare_2" >> $outfile
run perf_bare_2 ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_2 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_2 ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
run perf_bare_2 ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
run perf_bare_2 ev_idx=0x40,0x40 ev_msk=0x01,0x02
run perf_bare_2 ev_idx=0x40,0x40 ev_msk=0x04,0x08
run perf_bare_2 ev_idx=0x41,0x41 ev_msk=0x02,0x04
run perf_bare_2 ev_idx=0x41 ev_msk=0x08

echo "perf_bare_fipc" >> $outfile
run perf_bare_fipc ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_fipc ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_fipc ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
run perf_bare_fipc ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
run perf_bare_fipc ev_idx=0x40,0x40 ev_msk=0x01,0x02
run perf_bare_fipc ev_idx=0x40,0x40 ev_msk=0x04,0x08
run perf_bare_fipc ev_idx=0x41,0x41 ev_msk=0x02,0x04
run perf_bare_fipc ev_idx=0x41 ev_msk=0x08

echo "perf_bare_fipc_w" >> $outfile
run perf_bare_fipc_w ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_fipc_w ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
run perf_bare_fipc_w ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
run perf_bare_fipc_w ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
run perf_bare_fipc_w ev_idx=0x40,0x40 ev_msk=0x01,0x02
run perf_bare_fipc_w ev_idx=0x40,0x40 ev_msk=0x04,0x08
run perf_bare_fipc_w ev_idx=0x41,0x41 ev_msk=0x02,0x04
run perf_bare_fipc_w ev_idx=0x41 ev_msk=0x08

echo "perf_rpc" >> $outfile
run perf_rpc ev_idx=0x24,0x24,0x24,0x24 ev_msk=0x01,0x02,0x04,0x08
run perf_rpc ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x01,0x02,0x04,0x08
run perf_rpc ev_idx=0x26,0x26,0x26,0x26 ev_msk=0x10,0x20,0x40,0x80
run perf_rpc ev_idx=0x27,0x27,0x27,0x27 ev_msk=0x01,0x02,0x0e,0x08
run perf_rpc ev_idx=0x40,0x40 ev_msk=0x01,0x02
run perf_rpc ev_idx=0x40,0x40 ev_msk=0x04,0x08
run perf_rpc ev_idx=0x41,0x41 ev_msk=0x02,0x04
run perf_rpc ev_idx=0x41 ev_msk=0x08
