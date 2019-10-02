devname-objs := devname_drv.o devname_ioctl_dma.o	
obj-m := devname.o 


KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build V=1 M=$(PWD) modules
clean:
	test ! -d /lib/modules/$(KVERSION) || make -C /lib/modules/$(KVERSION)/build V=1 M=$(PWD) clean

EXTRA_CFLAGS += -I/usr/include
KBUILD_EXTRA_SYMBOLS = /lib/modules/$(KVERSION)/upciedev/Upciedev.symvers
