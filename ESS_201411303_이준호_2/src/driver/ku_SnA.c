#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include "../lib/ku_header.h"

MODULE_LICENSE("GPL");

/*
struct ku_data {
	struct list_head *list;
	struct tm timestamp;
	int value;
};

struct ku_data *mydata;
*/
spinlock_t my_lock;
static int irq_num;
static int state = 0;
static int app_state = 0;

int blue[8] = {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8] = {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};

static int c_step = 0;	// 0 ~ 7 current step

void setstep(int p1, int p2, int p3, int p4) {
	gpio_set_value(PIN1, p1);
	gpio_set_value(PIN2, p2);
	gpio_set_value(PIN3, p3);
	gpio_set_value(PIN4, p4);
}

void backward(int round, int delay, int step) {
	int i = 0, j = 0;

	for(i = 0; i< ONEROUND * round; i++) {
		for(j = step; j > 0; j--) {
			c_step --;
			if(c_step < 0) {
				c_step = 7;
			}

			setstep(blue[c_step], pink[c_step], yellow[c_step], orange[c_step]);
			udelay(delay);
		}
	}
}

void forward(int round, int delay, int step) {
	int i = 0, j = 0;

	for(i = 0; i< ONEROUND * round; i++) {
                for(j = step; j > 0; j--) {
			c_step ++;
			if(c_step > 7){
				c_step = 0;
			}
                        setstep(blue[c_step], pink[c_step], yellow[c_step], orange[c_step]);
                        udelay(delay);
                }
        }
}
void moveDegree(int degree, int delay, int direction) {
	// Direction : 0 = forward, 1 = backward
	// calculate degree to how many step
	
	int step;
	step = degree/45;

	// forward & backward
	if(!direction){
		// forward
		forward(1, delay, step);
	} else if(direction == 1) {
		// backward
		backward(1, delay, step);
	} else {
		printk("wrong direction\n");
	}

}

static void play(int note) {
	int i = 0;
	for(i = 0; i < 100; i++) {
		gpio_set_value(SPEAKER, 1);
		udelay(note);
		gpio_set_value(SPEAKER, 0);
		udelay(note);
	}
}

static void spk(int note_num) {	// note_num is index of notes
	int notes[] = { 1911, 1702, 1516, 1431, 1275, 1136, 1012, 955 };
	play(notes[note_num]);
	gpio_set_value(SPEAKER, 0);
}

static void func(void) {
	int i;
	unsigned long flags;
	if(!state){
		for(i = 0; i < 8; i++) {
			moveDegree(45, 1000, 0);
			spk(i);
		}
		spin_lock_irqsave(&my_lock, flags);
		state = 1;
		spin_unlock_irqrestore(&my_lock, flags);
		return;
	} else {		
		for(i = 7; i > -1; i--) {
			moveDegree(45, 1000, 1);
			spk(i);
		}
		spin_lock_irqsave(&my_lock, flags);
		state = 0;
		spin_unlock_irqrestore(&my_lock, flags);
		return;
	}
	mdelay(2000);
}

static irqreturn_t switch_irq_isr(int irq, void* dev_id) {
	// save sensing value and timestamp

//	struct ku_data *tmp = (struct ku_data*)kmalloc(sizeof(struct ku_data), GFP_KERNEL);
	struct timeval t;
	struct tm broken;
//	unsigned long flags;
	
//	tmp->value = 1;
	do_gettimeofday(&t);
	time_to_tm(t.tv_sec, 0, &broken);
	
	
	/*
	spin_lock_irqsave(&my_lock, flags);
	list_add(tmp->list, mydata->list);
	spin_unlock_irqrestore(&my_lock, flags);
	*/
	if(app_state == 1) {
		printk("%d:%d:%d:%ld\n", broken.tm_hour, broken.tm_min, broken.tm_sec, t.tv_usec);
		func();
	}
	return IRQ_HANDLED;
}

static dev_t dev_num;
static struct cdev *cd_cdev;

static long ku_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	unsigned long flags;	

	switch(cmd) {
		case IOCTL_RUN:
			spin_lock_irqsave(&my_lock, flags);
			app_state = 1;
			spin_unlock_irqrestore(&my_lock, flags);
			break;
		case IOCTL_END:
			spin_lock_irqsave(&my_lock, flags);
			app_state = 0;
			spin_unlock_irqrestore(&my_lock, flags);
			break;
		default:
			return -1;
	}
	return 0;
}

static int ku_open(struct inode *inode, struct file *file) {
	return 0;
}

static int ku_release(struct inode *inode, struct file *file) {
	return 0;
}

struct file_operations ku_fops = {
	.unlocked_ioctl = ku_ioctl,
	.open = ku_open,
	.release = ku_release
};

static int __init ku_init(void) {
//	struct ku_data *tmp = 0;
//	struct timeval t;
	//struct tm broken;	
	int ret = 0;
	
/*	
	// list init
	tmp = (struct ku_data*)kmalloc(sizeof(struct ku_data), GFP_KERNEL);
	INIT_LIST_HEAD(tmp->list);
	do_gettimeofday(&t);
	time_to_tm(t.tv_sec, 0, &tmp->timestamp);
	tmp->value = 0;
	mydata = tmp;
*/
	
	// character device init
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_fops);
	cdev_add(cd_cdev, dev_num, 1);

	// gpio init
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");
 	gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "SPEAKER");
	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");
	
	// spin lock init
	spin_lock_init(&my_lock);

	// switch setting
	irq_num = gpio_to_irq(SWITCH);
	ret = request_irq(irq_num, switch_irq_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if(ret) {
		printk("irq : Unable [%d]\n",irq_num);
		free_irq(irq_num, NULL);
	} else {
		printk("irq : Enable [%d]\n",irq_num);
	}
	

	return 0;
}

static void __exit ku_exit(void) {
	
	// character device free
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	// irq free
	disable_irq(irq_num);
	free_irq(irq_num, NULL);
	
	// gpio free
	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);
	gpio_set_value(SPEAKER, 0);
	gpio_free(SPEAKER);
	gpio_free(SWITCH);
}

module_init(ku_init);
module_exit(ku_exit);
