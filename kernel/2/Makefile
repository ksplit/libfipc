obj-m := betaModule2.o
CFLAGS_betaModule2.o = -DDEBUG -O2 -Wall

KDIR := /lib/modules/`uname -r`/build

default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
