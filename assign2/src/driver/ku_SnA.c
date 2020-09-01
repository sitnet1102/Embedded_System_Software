//#include "../lib/ku_sense.h"
//#include "../lib/ku_act.h"
#include "../lib/ku_common.h"

static long ku_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	struct list_head *pos = mylist.list;
	struct ku_list *entry;
	unsigned long flags;
	int ret = 0;
	int check = 0;
	switch(cmd) {
		case IOCTL_READ:	/* read data from queue and interrupt LED and SPEAKER */
			//rcu_read_lock();
			spin_lock(&my_lock);
			printk("IOCTL : Read data\n");
			entry = list_entry(pos, struct ku_list, list);
			if(entry == NULL){
				printk("IOCTL : Queue is empty\n");
				ret = FAIL;
			}else{
				check = request_irq(irq_num2, tasklet_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
				if(check) {
					printk("IOCTL(On) : Unable to reset request IRQ : %d\n", irq_num2);
					free_irq(irq_num2, NULL);
					ret = FAIL;
				} else {
					printk("IOCTL(On) : Enable to st request IRQ : %d\n", irq_num2);
					ret = SENSED;
					// list data to workqueue 	xx
					// delete data from list	xx	
				} 




/*
				if(entry->value < 20) {
					/// LED, SPEAKER ON
					check = request_irq(irq_num2, tasklet_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
					if(check) {
						printk("IOCTL(On) : Unable to reset request IRQ : %d\n", irq_num2);
						free_irq(irq_num2, NULL);
						ret = FAIL;
					} else {
						printk("IOCTL(On) : Enable to st request IRQ : %d\n", irq_num2);
						ret = SENSED;
						// list data to workqueue
						// delete data from list
					} 
					
				}else{
					printk("IOCTL : Nothing happened\n");
					/// LED, SPEAKER OFF
					check = request_irq(irq_num2, tasklet_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
					if(check) {
						printk("IOCTL(Off) : Unable to reset request IRQ : %d\n", irq_num2);
						free_irq(irq_num2, NULL);
						ret = FAIL;
					} else {
						printk("IOCTL(Off) : Enable to st request IRQ : %d\n", irq_num2);
						ret = NOTHING;
					} 

				}
*/
			}
			//rcu_read_unlock();
			spin_unlock(&my_lock);
			return ret;
		case IOCTL_RUN:
			if(timer_run_flag = 1) break;
			// timer init
			//my_timer.delay_jiffies = msecs_to_jiffies(500); // 0.5 seconds 
			my_timer.delay_jiffies = msecs_to_jiffies(50000); // 50 seconds 
			timer_setup(&my_timer.timer, my_timer_func, 0);
			my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
			add_timer(&my_timer.timer);
			timer_run_flag = 1;
			return ret;
/* 
		case IOCTL_WRITE:
			spin_lock_irqsave(&my_lock, flags);
			printk("IOCTL : WRITE\n");
			list_for_each_entry_rcu(pos, &mylist.list, list) {
				if(pos->id == arg) {
					old = pos;
					new = (struct test_list*)kmalloc(sizeof(struct test_list), GFP_KERNEL);
					memcpy(new, old, sizeof(struct test_list));
					new->data++;
					list_replace_rcu(&pos->list, &new->list);
					synchronize_rcu();
					kfree(old);
					printk("IOCTL : List [id = %ld] = %ld\n", arg, new->data);
					spin_unlock_irqrestore(&my_lock, flags);
					return 0;
				}
			}
			printk("simple_rculist : Update Not Found Id = %ld \n", arg);
			spin_unlock_irqrestore(&my_lock, flags);
			break;
*/
		default:
			return -1;
	}
	return 0;
}

static int simple_ioctl_open(struct inode *inode, struct file *file) {
	//enable_irq(irq_num);
	return 0;
}

static int simple_ioctl_release(struct inode *inode, struct file *file) {
	//disable_irq(irq_num);
	return 0;
}

struct file_operations ku_cdev_fops = {
	.unlocked_ioctl = ku_ioctl,
	.open = simple_ioctl_open,
	.release = simple_ioctl_release
};


// device value
static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_201411303_init(void) {
	
	int ret;
	struct ku_list *pos;
	//struct timeval t;
	//struct tm *broken;

	printk("Module : Init\n");

	// character device driver
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_cdev_fops);
	cdev_add(cd_cdev, dev_num, 1);
	
	// lock init
//	rwlock_init(&my_lock);
	spin_lock_init(&my_lock);
	
	// list init
	INIT_LIST_HEAD(&mylist.list);	
	pos = (struct ku_list*)kmalloc(sizeof(struct ku_list), GFP_KERNEL);
	pos->value = -1;
	pos->mode = 0;
	//do_gettimeofday(&t);
	//broken = (struct tm*)kmalloc(sizeof(struct tm), GFP_KERNEL);
	//time_to_tm(t.tv_sec, 0, broken);
	//pos->timestamp = broken;
	list_add_tail(&(pos->list), &mylist.list);
	
	// tasklet init
	tasklet_init(&my_tasklt, tasklet_func, (unsigned long) &mylist);
	
	// workqueue init
	my_wq = create_workqueue_init("my_workqueue");

	// gpio init
	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	gpio_request_one(ULTRA_TRIG, GPIOF_OUT_INIT_LOW, "ULTRA_TRIG");
	gpio_request_one(ULTRA_ECHO, GPIOF_IN, "ULTRA_ECHO");
	gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "SPEAKER");
	irq_num = gpio_to_irq(ULTRA_ECHO);
	
	// irq init
/*
	ret = request_irq(irq_num, ku_ultra_isr, IRQF_TRIGGER_RISING, "ultrasonic_irq", NULL);
	if(ret) {
		printk("ku_irq : Unable to request IRQ: %d\n", irq_num);
		free_irq(irq_num, NULL);
	} else {
		disable_irq(irq_num);
	}
*/
/*
	// timer init
	//my_timer.delay_jiffies = msecs_to_jiffies(500); // 0.5 seconds 
	my_timer.delay_jiffies = msecs_to_jiffies(50000); // 50 seconds 
	timer_setup(&my_timer.timer, my_timer_func, 0);
	my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
	add_timer(&my_timer.timer);
*/
	return 0;
}

static void __exit ku_20411303_exit(void) {
	struct ku_list *pos;
	struct ku_list *tmp;
	unsigned long flags;

	printk("Module : Exit\n");

	// list delete
	spin_lock_irqsave(&my_lock, flags);
	list_for_each_entry_safe(tmp, pos, &mylist.list, list){
		list_del_rcu(&(tmp->list));
		synchronize_rcu();
		kfree(tmp->timestamp);
		kfree(tmp);
	}
	
	spin_unlock_irqrestore(&my_lock, flags);
	
	// character device driver delete
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	
	// tasklet delete
	tasklet_kill(&my_tasklet);
	
	// workqueue delete
	flush_workqueue(my_wq);
	destroy_workqueue(my_wq);

	// gpio free
	gpio_set_value(LED1, 0);
	gpio_set_value(ULTRA_TRIG, 0);
	gpio_set_value(ULTRA_ECHO, 0);
	gpio_set_value(SPEAKER, 0);
	gpio_free(LED1);
	gpio_free(ULTRA_TRIG);
	gpio_free(ULTRA_ECHO);
	gpio_free(SPEAKER);
	
	// irq free
	free_irq(irq_num, NULL);

	// timer delete
	del_timer(&my_timer.timer);
	
}

module_init(ku_201411303_init);
module_exit(ku_201411303_exit);
