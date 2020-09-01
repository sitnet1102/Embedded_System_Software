#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define LED1 5

#define SWITCH 12

struct ch7_timer_info {
	struct timer_list timer;
	long delay_jiffies;
};

static struct ch7_timer_info ch7_timer;

static void ch7_timer_func(struct timer_list *t) {
	// check switch every 500 msec 
	struct ch7_timer_info *info = from_timer(info, t, timer);
	int check;
	printk("ch7_timer : Jiffies %ld\n", jiffies);
	check = gpio_get_value(SWITCH);
	//msleep(1000);
	if(check){
		gpio_set_value(LED1, 1);
	} else {
		gpio_set_value(LED1, 0);
	}
	
	mod_timer(&ch7_timer.timer, jiffies + info->delay_jiffies);
}

static int __init ch7_init(void) {
	printk("ch7 : init modules \n");
	
	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");

	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");

	ch7_timer.delay_jiffies = msecs_to_jiffies(500);
	timer_setup(&ch7_timer.timer, ch7_timer_func, 0);
	ch7_timer.timer.expires = jiffies + ch7_timer.delay_jiffies;
	add_timer(&ch7_timer.timer);
	
	return 0;
}

static void __exit ch7_exit(void) {
	printk("ch7 : exit modules\n");
	gpio_set_value(LED1, 0);
	gpio_free(LED1);
	gpio_free(SWITCH);
	del_timer(&ch7_timer.timer);
}

module_init(ch7_init);
module_exit(ch7_exit);
