#define DEV_NAME "ku_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_RUN	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_END   	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
	
#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26
#define SWITCH 18
#define SPEAKER 12

#define ONEROUND 512

#define RUN 1
#define END 2
