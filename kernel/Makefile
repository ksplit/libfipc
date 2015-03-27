obj-m :=  betaModule1.o
#CFLAGS_betaModule1.o = -DDEBUG -DDEBUG_MWAIT_RETRY  -DTIMING -g -Wall
#CFLAGS_betaModule.o = -DDEBUG -DDEBUG_MWAIT_RETRY   -DTIMING -g -Wall
#CFLAGS_ring-channel.o = -DDEBUG -DDEBUG_MWAIT_RETRY -DTIMING -g -Wall

CFLAGS_betaModule1.o =  -g -O2 -DUSE_POLL -DTIMING #-DDEBUG #-DDEBUG_MWAIT_RETRY 
CFLAGS_betaModule.o =   -g -O2 -DUSE_POLL -DTIMING #-DDEBUG #-DDEBUG_MWAIT_RETRY #-DDEBUG
CFLAGS_ring-channel.o = -g -O2

betaModule1-objs := ./ring-chan/ring-channel.o ./betaModule.o

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
