#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/rwlock.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include "ku_header.h"
MODULE_LICENSE("GPL");
// library
//#include <linux/rculist.h>
//#include <sys/time.h>
// 	structure
// data list
struct ku_list {
	struct list_head *list;
	//struct tm timestamp;
	long value;
	int mode;
};

// timer 
struct my_timer_info {
	struct timer_list timer;
	long delay_jiffies;
};

//	static value


// lock value
spinlock_t my_lock;
//rwlock_t my_lock;

// irq value
static int irq_num;
static int irq_num2;
static int echo_valid_flag = 3;
//static int act_flag = 3;
static int timer_run_flag = 0;

// ultrasonic value
static ktime_t echo_start;
static ktime_t echo_stop;

// list head
static struct ku_list mylist;

// tasklet
static struct tasklet_struct my_tasklet;

// workqueue
static struct workqueue_struct *my_wq;

// timer 
static struct my_timer_info my_timer;



/////////////////////////           sensor	////////////////////////////////////////

static irqreturn_t ku_ultra_isr(int irq, void* dev_id) { /* ultrasonic function */

	ktime_t tmp_time;
	s64 time;
	int cm;
	//struct list_head* new;
	struct ku_list* tmp;
	//struct timeval t;
	//struct tm *broken;
	//unsigned long flags;

	tmp->mode = 0;	// basic mode for act -> do nothing
	
	tmp_time = ktime_get();
	if(echo_valid_flag == 1) {

		//printk("ultra : Echo UP\n");
		if(gpio_get_value(ULTRA_ECHO) == 1) {
			echo_start = tmp_time;
			echo_valid_flag = 2;
		}

	} else if (echo_valid_flag == 2) {

		//printk("ultra : Echo Down\n");
		if(gpio_get_value(ULTRA_ECHO) == 0) {

			echo_stop = tmp_time;
			time = ktime_to_us(ktime_sub(echo_stop, echo_start));
			cm = (int) time / 58;
			printk("ultra : Detect %d cm\n", cm);

			tmp = (struct ku_list*)kmalloc(sizeof(struct ku_list), GFP_KERNEL);
			tmp->value = cm;
			//do_gettimeofday(&t);
			//broken = (struct tm*)kmalloc(sizeof(struct tm), GFP_KERNEL);
			//time_to_tm(t.tv_sec, 0, broken);
			//tmp->timestamp = broken;
			
			if(cm < 20 && cm > 0) {
				tmp->mode = 1;			// notice mode 		be careful it is close
				if(cm < 10) tmp->mode = 2;	// danger mode 		
				
				spin_lock(&my_lock);
				//write_lock_irqsave
				list_add(tmp->list, mylist.list);
				mylist.list = tmp->list;
				echo_valid_flag = 3;
				spin_unlock(&my_lock);
			} else {
				// delete tmp
				//kfree(broken);
				kfree(tmp);
			}
		}

	}
	return IRQ_HANDLED;
}

static void my_timer_func(struct timer_list *t) {	/* timer function for ultrasonic device */
	struct my_timer_info *info = from_timer(info, t, timer);
	int ret;

	printk("timer : jiffies %ld \n", jiffies);
	ret = request_irq(irq_num, ku_ultra_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "ULTRA_ECHO", NULL);
	if(ret) {
		printk("ultra : Unable to request IRQ : %d\n", ret);
		free_irq(irq_num, NULL);
	} else {
		if(echo_valid_flag == 3) {
			echo_start = ktime_set(0,1);
			echo_stop = ktime_set(0,1);
			echo_valid_flag = 0;

			gpio_set_value(ULTRA_TRIG, 1);
			udelay(10);
			gpio_set_value(ULTRA_TRIG, 0);

			echo_valid_flag = 1;
		}
	}
	
	mod_timer(&my_timer.timer, jiffies + info->delay_jiffies);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////	actuator	////////////////////////////////////////////////////

void tasklet_func(unsigned long recv_data) {
	struct ku_list *tmp;
	tmp = (struct ku_list *) recv_data;
	unsigned int note = 0;
	int i = 0;
	printk("LED ON\n");
	// turn on the led
	
	if(tmp->mode == 1) {
		note = 1911;
	} else if(tmp->mode == 2) {
		note = 1012;
	}
	
	if(note != 0) {
		gpio_set_value(LED1, 1);
	} else {
		gpio_set_value(LED1, 0);
	}

	for(i = 0; i < 100; i++) {
		gpio_set_value(SPEAKER, 1);
		udelay(note);
		gpio_set_value(SPEAKER, 0);
		udelay(note);
	}
}


static irqreturn_t tasklet_isr(int irq, void* data) {
	tasklet_schedule(&my_tasklet);
	return IRQ_HANDLED;
}



