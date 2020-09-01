#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("DAUL BSD/GPL"); /* optinal */

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...) \
		printk(KERN_DEBUG "MYDEBUG: " fmt, ##args);
#else 
	#define DEBUG_MSG(fmt, args...)
#endif

static int __init debug_init(void)
{
	printk(KERN_NOTICE "Debug init\n");
	DEBUG_MSG("Hello debug world!\n");
	return 0; /* return zero on successful loading */
}

static void __exit debug_exit(void)
{
	printk(KERN_NOTICE "Debug exit\n");
}

module_init(debug_init);
module_exit(debug_exit);
