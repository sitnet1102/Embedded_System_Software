// definition
#define LED1 			4
#define SPEAKER 		12
#define ULTRA_TRIG 		17
#define ULTRA_ECHO		18

#define DEV_NAME "ku_dev"

#define IOCTL_START_NUM 	0x80
#define IOCTL_NUM1 		IOCTL_START_NUM+1
#define IOCTL_NUM2 		IOCTL_START_NUM+2

#define KU_IOCTL_NUM 		'z'
#define IOCTL_READ		_IOWR(KU_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_RUN		_IOWR(KU_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

#define FAIL 			0xFF	//1111 1111
#define SENSED			0xF0	//1111 0000
#define NOTHING			0x00	//0000 0000
