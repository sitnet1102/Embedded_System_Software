obj-m:= switch_irq.o

KDIR:=~/linux-rpi/
ARM:= ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
clean:
	$(MAEK) -C $(KDIR) M=$(PWD) $(ARM) clean
scp:
	scp $(obj-m:.o=.ko) pi@192.168.0.103:~/
