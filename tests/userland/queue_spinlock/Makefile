CC:=gcc
CFLAGS:=-Wall -D_GNU_SOURCE -pthread -lrt -lm
INCLUDE_DIRS:=-I../../../include -I../../../include/userland
TARGET:=$(notdir $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
DEP:=main.c ../../../src/ipc.c queue.c spinlock.c

compile: $(DEP)
	$(CC) $(INCLUDE_DIRS) $(DEP) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)
