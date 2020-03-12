#include <linux/init.h>
#include <linux/module.h>

static int scull_init(void)
{
	printk("Hello from Scull\n");
	return 0;
}

static void scull_exit(void)
{
	printk("Goodbye from Scull\n");
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(scull_init);
module_exit(scull_exit);
