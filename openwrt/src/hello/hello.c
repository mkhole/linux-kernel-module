#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	pr_info("%s\n", __func__);
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("%s\n", __func__);
}

MODULE_LICENSE("GPL");

module_init(hello_init);
module_exit(hello_exit);

