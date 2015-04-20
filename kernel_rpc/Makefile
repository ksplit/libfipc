obj-m :=  betaModule_RPC1.o 
#CFLAGS_betaModule1.o = -DDEBUG -DDEBUG_MWAIT_RETRY  -DTIMING -g -Wall
#CFLAGS_betaModule.o = -DDEBUG -DDEBUG_MWAIT_RETRY   -DTIMING -g -Wall
#CFLAGS_ring-channel.o = -DDEBUG -DDEBUG_MWAIT_RETRY -DTIMING -g -Wall


CFLAGS_betaModule_RPC1.o =  -g -O2  -DPOLL -DTIMING -DUSE_FLOOD #-DDEBUG #-DDEBUG_MWAIT_RETRY 
CFLAGS_betaModule_RPC.o =   -g -O2 -DPOLL -DTIMING -DUSE_FLOOD #-DDEBUG #-DDEBUG_MWAIT_RETRY #-DDEBUG
CFLAGS_ring-channel.o = -g -O2
#CFLAGS_functions.o = -O2

betaModule_RPC1-objs := ./ring-chan/ring-channel.o ./betaModule_RPC.o ./functions.o

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
