USE_ASYNC='y'

obj-m := timing.o


ASYNC_DIR=/../../../../libasync/src

ccflags-y += -O2 -I$(PWD)/../../IPC -I$(PWD)/$(ASYNC_DIR)/include/ 

INC_DIR = ../../IPC

CFLAGS_ipc.o = -O2 -DPOLL 
CFLAGS_ring-channel.o = -I$(INC_DIR)
DEPS= $(INC_DIR)/ipc.h


timing-objs = timing_util.o ../../IPC/ipc.o ../../ring-chan/ring-channel.o

ifeq ($(USE_ASYNC),'y')
	timing-objs += async_timing_setup.o $(ASYNC_DIR)/common/thc.o $(ASYNC_DIR)/common/thcsync.o $(ASYNC_DIR)/common/awe-mapper.o
else
	timing-objs += timing_setup.o
endif

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm ../../IPC/ipc.o
	rm ../../ring-chan/ring-channel.o
