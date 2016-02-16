obj-m := rpc_testing.o
ccflags-y += -O0 -I../../IPC -fno-ipa-cp -fno-ipa-sra -DUSE_ASYNC
EXTRA_CFLAGS=-DDEBUG_OUTPUT
INC_DIR = ../../IPC
CFLAGS_ipc.o = -O2 -DPOLL
CFLAGS_ring-channel.o = -I$(INC_DIR)
DEPS= $(INC_DIR)/ipc.h

rpc_testing-objs :=  ../../IPC/ipc.o ../../ring-chan/ring-channel.o callee.o caller.o rpc_test.o

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm ../../IPC/ipc.o
	rm ../../ring-chan/ring-channel.o
