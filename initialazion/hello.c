#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("Dual BSD/GPL");

static int __init hello_init(void){
	printk(KERN_NOTICE "Hello, World #2\n");
	return 0; /*return zero on successful loading*/
}

static void __exit hello_exit(void){
	printk(KERN_NOTICE "Goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);
