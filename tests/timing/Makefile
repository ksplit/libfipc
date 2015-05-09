obj-m := timing.o

ccflags-y += -O2 -I/users/sbauer/ipc/ramstore/fast-ipc-module/IPC

INC_DIR = ../../IPC

CFLAGS_ipc.o = -O2 -DPOLL 
CFLAGS_ring-channel.o = -I$(INC_DIR)
DEPS= $(INC_DIR)/ipc.h


timing-objs :=  ../../IPC/ipc.o ../../ring-chan/ring-channel.o timing_setup.o

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm ../../IPC/ipc.o
	rm ../../ring-chan/ring-channel.o
