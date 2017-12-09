INCLUDE_DIRS:=-I$(src)/../../../include -I$(src)/../../../include/kernel
TARGET:=$(notdir $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
DEP:=main.c queue.c ../../../src/ipc.c
KDIR:=/lib/modules/$(shell uname -r)/build

obj-m += $(TARGET).o
$(TARGET)-y := ../../../src/ipc.o main.o queue.o
ccflags-y := $(INCLUDE_DIRS) -std=gnu99 -Wno-declaration-after-statement

compile: $(DEP)
	make -C $(KDIR) M=$(PWD)

clean:
	make -C $(KDIR) M=$(PWD) clean
