
ifneq ($(KERNELRELEASE),)
obj-m += dma.o
dma-m += dma_memcpy.o
else
KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
endif
