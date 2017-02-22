all:
	gcc -Wall -g -D_GNU_SOURCE main.c ../../../../common/ipc.c -I../../../../include -I../../include -o ping_pong -pthread -O3

perf:
	perf stat -e cache-references,cache-misses,cycles,instructions,branches,faults ./ping_pong
