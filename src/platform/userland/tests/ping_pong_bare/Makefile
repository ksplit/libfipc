perfCacheEvents='cache-misses,cache-references,alignment-faults,major-faults,minor-faults,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-prefetch-misses,L1-dcache-store-misses,L1-dcache-stores,LLC-load-misses,LLC-loads,LLC-prefetch-misses,LLC-prefetches,LLC-store-misses,LLC-stores'
perfCPUEvents='cycles,instructions,migrations,stalled-cycles-frontend,stalled-cycles-backend,branches,faults'
all:
	gcc -Wall -g -D_GNU_SOURCE main.c ../../../../common/ipc.c -I../../../../include -I../../include -o ping_pong -pthread -O3 -lm

perf:
	perf stat -e  ${perfCacheEvents},${perfCPUEvents} ./ping_pong
