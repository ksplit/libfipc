CC:=gcc
CFLAGS:=-Wall -D_GNU_SOURCE -pthread -lm -O2 -lnuma
INCLUDE_DIRS:=-I../../../include -I../../../include/userland
TARGET:=$(notdir $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
DEP:=race.c ../queue_mpmc_mcslock/mcs.c ../queue_mpmc_spinlock/spinlock.c ../queue_mpmc_ticketlock/ticketlock.c

compile: $(DEP)
	$(CC) $(INCLUDE_DIRS) $(DEP) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)

