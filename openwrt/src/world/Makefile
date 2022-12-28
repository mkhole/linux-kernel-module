
ifneq ($(KERNELRELEASE),)
obj-m += world.o
else
KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
endif
