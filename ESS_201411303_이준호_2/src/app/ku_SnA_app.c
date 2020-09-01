#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "../lib/ku_header.h"

int main(int argc, char *argv[]) {
	int dev;
	int i, n, op;

	if(argc != 3) {
		printf("Insert two arguments \n");
		printf("First argument = (1 : read, 2 : write)\n");
		printf("Second argument = number of iterations \n");
		return -1;
	}

	op = atoi(argv[1]);
	n = atoi(argv[2]);

	dev = open("/dev/ku_dev", O_RDWR);

	for(i = 1; i <= n; ++i) {
		if(op == RUN){
			ioctl(dev, IOCTL_RUN, NULL);
		} else if(op == END) {
			ioctl(dev, IOCTL_END, (unsigned long) i);
		} else {
			printf("Invalid Operation\n");
			close(dev);
			return -1;
		}
	}
	close(dev);

	return 0;
}

