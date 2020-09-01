#include <linux/init.h>
#include <linux/module.h>

static int my_id = 000000000; /* Global Variable */

static int __init ch1_mod1_init(void)
{
	printk(KERN_NOTICE "Hello, Mod1\n");
	return 0;
}

static void __exit ch1_mod1_exit(void)
{
	printk(KERN_NOTICE "Goodbye, Mod1\n");
}

int get_my_id(void)
{
	return my_id;
}
EXPORT_SYMBOL(get_my_id);

int set_my_id(int id)
{
	my_id = id;
	if(id == my_id)
	{
		return 1;
	}
	return 0;
}
EXPORT_SYMBOL(set_my_id);

module_init(ch1_mod1_init);
module_exit(ch1_mod1_exit);

