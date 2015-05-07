obj-m := rpc_testing.o

ccflags-y += -O2 -I/users/sbauer/ipc/ramstore/fast-ipc-module/IPC

INC_DIR = ../../IPC

CFLAGS_ipc.o = -O2 -DPOLL
CFLAGS_ring-channel.o = -I$(INC_DIR)
DEPS= $(INC_DIR)/ipc.h


rpc_testing-objs :=  ../../IPC/ipc.o ../../ring-chan/ring-channel.o callee.o caller.o rpc_setup.o rpc_test.o

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
