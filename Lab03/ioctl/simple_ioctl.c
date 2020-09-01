#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_ioctl_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define SIMPLE_IOCTL1 _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long)
#define SIMPLE_IOCTL2 _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

static int simple_ioctl_open(struct inode *inode, struct file *file) {
	printk("simple_ioctl : open\n");
	return 0;
}

static int simple_ioctl_release(struct inode *inode, struct file *file) {
	printk("simple_ioctl : release\n");
	return 0;
}

static long simple_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	switch(cmd) {
		case SIMPLE_IOCTL1:
			printk("simple_ioctl : IOCTL1. %lu\n", arg);
			break;
		case SIMPLE_IOCTL2:
			printk("simple_ioctl : IOCTL2. %p\n", (unsigned long *)arg);
			break;
		default:
			return -1;
	}

	return 0;
}

struct file_operations simple_ioctl_fops = {
	.open = simple_ioctl_open,
	.release = simple_ioctl_release,
	.unlocked_ioctl = simple_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_ioctl_init(void) {
	printk("simple_ioctl : init module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &simple_ioctl_fops);
	cdev_add(cd_cdev, dev_num, 1);

	return 0;
}

static void __exit simple_ioctl_exit(void) {
	printk("simple_ioctl : exit module\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(simple_ioctl_init);
module_exit(simple_ioctl_exit);
