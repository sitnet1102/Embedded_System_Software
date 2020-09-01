#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define SENSOR1 17
#define LED1 5
#define PROC_BUF_SIZE 256

static int irq_num;
struct ch8_timer_info {
	struct timer_list timer;
	long delay_jiffies;
};

static struct ch8_timer_info ch8_timer;
spinlock_t spinlock;

static irqreturn_t ch8_isr(int irq, void* dev_id){
	unsigned long flags;
	spin_lock_irqsave(&spinlock, flags);
	
	printk("sensor_proc : Detect\n");
	// led on
	add_timer(&ch8_timer.timer);
	spin_unlock_irqrestore(&spinlock, flags);

	return IRQ_HANDLED;
}
static void ch8_timer_func(struct timer_list *t) {
	struct ch8_timer_info *info = from_timer(info, t, timer);
	int check;
	printk("ch8_timer : Jiffies %ld\n", jiffies);
	check = gpio_get_value(SENSOR1);
	gpio_set_value(LED1, 0);
	
	if(check){
		gpio_set_value(LED1, 1);
	} else {
		gpio_set_value(LED1, 0);
	}
	
	mod_timer(&ch8_timer.timer, jiffies + info->delay_jiffies);
}

static int __init ch8_init(void) {
	int ret;

	printk("ch8 : Init Module \n");

	
	spin_lock_init(&spinlock);
	

	gpio_request_one(SENSOR1, GPIOF_IN, "sensor1");
	gpio_request_one(LED1, GPIOF_IN, "led1");
	irq_num = gpio_to_irq(SENSOR1);
	
	ch8_timer.delay_jiffies = msecs_to_jiffies(2000);
	timer_setup(&ch8_timer.timer, ch8_timer_func, 0);
	ch8_timer.timer.expires = jiffies + ch8_timer.delay_jiffies;
		
	
	ret = request_irq(irq_num, ch8_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if(ret) {
		printk("ch8 : Unable to request IRQ : %d\n", irq_num);
		free_irq(irq_num, NULL);
	}

	return 0;

}

static void __exit ch8_exit(void) {
	printk("ch8 : Exit Module \n");

	free_irq(irq_num, NULL);
	gpio_free(SENSOR1);
	gpio_free(LED1);
	del_timer(&ch8_timer.timer);
}

module_init(ch8_init);
module_exit(ch8_exit);
