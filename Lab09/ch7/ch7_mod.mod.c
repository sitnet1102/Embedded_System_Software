#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xad1a7def, "module_layout" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0xfe990052, "gpio_free" },
	{ 0x24d273d1, "add_timer" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0xffb0dcd9, "gpiod_set_raw_value" },
	{ 0x16e02b99, "gpiod_get_raw_value" },
	{ 0x350a004, "gpio_to_desc" },
	{ 0x7c32d0f0, "printk" },
	{ 0x526c3a6c, "jiffies" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "85C9D1F1936E8B6F5801293");
