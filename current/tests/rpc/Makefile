obj-m := rpc_testing.o

ccflags-y += -O0 -I/users/mquigley/fast-ipc-module/current/IPC -fno-ipa-cp -fno-ipa-sra
EXTRA_CFLAGS=-DDEBUG_OUTPUT
INC_DIR = ../../IPC
CFLAGS_ipc.o = -O2 -DPOLL
CFLAGS_ring-channel.o = -I$(INC_DIR)
CFLAGS_awe-mapper.o = -I/users/mquigley/fast-ipc-module/current/awe-mapper 
DEPS= $(INC_DIR)/ipc.h

rpc_testing-objs :=  ../../IPC/ipc.o ../../ring-chan/ring-channel.o ../../awe-mapper/awe-mapper.o callee.o caller.o rpc_test.o

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm ../../IPC/ipc.o
	rm ../../ring-chan/ring-channel.o
	rm ../../awe-mapper/awe-mapper.o
