obj-m := mod2.o

KBUILD_EXTRA_SYMBOLS := /home/juno/class/Lab2/symbol/mod1/Module/symvers
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

