#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../lib/ku_header.h"

void main(void) {
	int dev, cmd;
	
	dev = open("/dev/ku_dev", O_RDWR);
	
	switch (cmd) {
		case 1:
			ioctl(dev, IOCTL_READ, 0);
			break;
		case 2:
			ioctl(dev, IOCTL_RUN, 0);
			break;
	}
	
	printf("success : %d\n", cmd);

	close(dev);
}
	
