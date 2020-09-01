obj-m := ch2_mod_201411303.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MKAE) -C $(KERNELDIR) M=$(PWD) clean

