#include <linux/init.h>
#include <linux/module.h>

static int __init world_init(void)
{
	pr_info("%s\n", __func__);
	return 0;
}

static void __exit world_exit(void)
{
	pr_info("%s\n", __func__);
}

MODULE_LICENSE("GPL");

module_init(world_init);
module_exit(world_exit);

