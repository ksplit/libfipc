obj-m := async_dispatch.o

INC_BASE_PATH=/local/sda4/xcap-async-module
ASYNC_SRC=../../../../libasync/src/common
ccflags-y += -O0 -I$(INC_BASE_PATH)/fast-ipc-module/current/IPC -I$(INC_BASE_PATH)/libasync/src/include -fno-ipa-cp -fno-ipa-sra -DUSE_ASYNC
EXTRA_CFLAGS=-DDEBUG_OUTPUT
INC_DIR = ../../IPC
CFLAGS_ipc.o = -O2 -DPOLL
CFLAGS_ring-channel.o = -I$(INC_DIR)
DEPS= $(INC_DIR)/ipc.h

async_dispatch-objs := $(ASYNC_SRC)/awe-mapper.o ../../IPC/ipc.o ../../ring-chan/ring-channel.o  $(ASYNC_SRC)/thc.o $(ASYNC_SRC)/thcsync.o ipc_dispatch.o thread1_fn1.o thread2_fn1.o async_tester.o 

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm ../../IPC/ipc.o
	rm ../../ring-chan/ring-channel.o
