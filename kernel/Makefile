obj-m := betaModule.o
CFLAGS_betaModule.o = -DDEBUG

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
