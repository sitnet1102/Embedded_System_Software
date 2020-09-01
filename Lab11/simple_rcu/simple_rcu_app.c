#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#define READ 1
#define WRITE 2

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_READ 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long*)
#define IOCTL_WRITE     _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long*)

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

	dev = open("/dev/simple_rcu_dev", O_RDWR);

	for(i = 1; i <= n; ++i) {
		if(op == READ){
			ioctl(dev, IOCTL_READ, NULL);
		} else if(op == WRITE) {
			ioctl(dev, IOCTL_WRITE, (unsigned long) i);
		} else {
			printf("Invalid Operation\n");
			close(dev);
			return -1;
		}
	}
	close(dev);

	return 0;
}

