#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define SENSOR1 17
#define PROC_BUF_SIZE 256

static int irq_num;

struct time_list{
	struct list_head list;
	struct timespec time;
};

static struct proc_dir_entry* ent;
static struct time_list time_list_head;
spinlock_t spinlock;

static int sensor_proc_show(struct seq_file* seq, void* v){
	struct time_list* node = 0;
	struct list_head* pos = 0;
	struct list_head* q = 0;
	int index = 0;
	unsigned long flags;
	static char proc_buf[PROC_BUF_SIZE];

	printk("sensor_proc: sensor_proc_show\n");
	memset(proc_buf, 0, PROC_BUF_SIZE);

	if(!list_empty(&time_list_head.list)){
		spin_lock_irqsave(&spinlock, flags);
		list_for_each_safe(pos, q, &time_list_head.list) {
			node = list_entry(pos, struct time_list, list);
			sprintf(proc_buf, "[%d] Time : %ld", index++, node->time.tv_sec);
			seq_printf(seq, "%s\n", proc_buf);
		}
		spin_unlock_irqrestore(&spinlock, flags);
	} else {
		strcpy(proc_buf, "Empty List\n");
		seq_printf(seq, "%s\n", proc_buf);
	}

	return 0;
}

static int sensor_proc_open(struct inode* inode, struct file* file) {
	return single_open(file, sensor_proc_show, NULL);
}

struct file_operations sensor_proc_fops = {
	.open = sensor_proc_open,
	.read = seq_read,
};

static irqreturn_t sensor_proc_isr(int irq, void* dev_id){
	unsigned long flags;
	struct time_list* node = 0;

	node = (struct time_list*)kmalloc(sizeof(struct time_list), GFP_ATOMIC);

	spin_lock_irqsave(&spinlock, flags);
	getnstimeofday(&node->time);
	printk("sensor_proc : Detect\n");
	list_add_tail(&node->list, &time_list_head.list);
	spin_unlock_irqrestore(&spinlock, flags);

	return IRQ_HANDLED;
}

static int __init sensor_proc_init(void) {
	int ret;

	printk("sensor_proc : Init Module \n");

	INIT_LIST_HEAD(&time_list_head.list);
	spin_lock_init(&spinlock);
	ent = proc_create("sensor_proc", 0666, NULL, &sensor_proc_fops);

	gpio_request_one(SENSOR1, GPIOF_IN, "sensor1");
	irq_num = gpio_to_irq(SENSOR1);
	ret = request_irq(irq_num, sensor_proc_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if(ret) {
		printk("sensor_proc : Unable to request IRQ : %d\n", irq_num);
		free_irq(irq_num, NULL);
	}

	return 0;

}

static void __exit sensor_proc_exit(void) {
	struct time_list* node = 0;
	struct list_head* pos = 0;
	struct list_head* q = 0;

	printk("sensor_proc : Exit Module \n");
	proc_remove(ent);

	free_irq(irq_num, NULL);
	gpio_free(SENSOR1);

	list_for_each_safe(pos, q, &time_list_head.list) {
		node = list_entry(pos, struct time_list, list);
		printk("sensor_proc : Delete time : %ld", node->time.tv_sec);

		list_del(pos);
		kfree(node);
	}
}

module_init(sensor_proc_init);
module_exit(sensor_proc_exit);
