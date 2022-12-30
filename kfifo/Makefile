
ifneq ($(KERNELRELEASE),)
obj-m += kfifo.o
ccflags-y += -save-temps
else
KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
endif
