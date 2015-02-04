obj-m :=  betaModule.o
betaModule-objs := ./ring-chan/ring-channel.o
CFLAGS_betaModule.o = -DDEBUG -DDEBUG_MWAIT_RETRY -g -Wall

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
