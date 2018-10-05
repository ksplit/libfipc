CC:=gcc
CFLAGS:=-Wall -D_GNU_SOURCE -pthread -lm -O2 -lnuma
INCLUDE_DIRS:=-I../../../include -I../../../include/userland
TARGET:=$(notdir $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
DEP:=main.c ../../../src/ipc.c ../libfipc_numa.c queue.c spinlock.c

PERF_CACHE:='cache-misses,cache-references,alignment-faults,major-faults, \
				minor-faults,L1-dcache-load-misses,L1-dcache-loads, \
				L1-dcache-prefetch-misses,L1-dcache-store-misses, \
				L1-dcache-stores,LLC-load-misses,LLC-loads,LLC-prefetch-misses, \
				LLC-prefetches,LLC-store-misses,LLC-stores'

PERF_CPU:='cycles,instructions,migrations,stalled-cycles-frontend, \
			stalled-cycles-backend,branches,faults'

compile: $(DEP)
	$(CC) $(INCLUDE_DIRS) $(DEP) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)

perf:
	perf stat -e  ${PERF_CACHE},${PERF_CPU} ./$(TARGET)

debug: $(DEP)
	$(CC) $(INCLUDE_DIRS) $(DEP) -o $(TARGET) $(CFLAGS) -g
