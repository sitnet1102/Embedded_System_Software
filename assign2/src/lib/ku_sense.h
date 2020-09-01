#include "ku_common.h"

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
				list_add(tmp->list, mylist->list);
				mylist->list = tmp->list;
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


