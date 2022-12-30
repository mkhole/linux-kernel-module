#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kfifo.h>

struct student {
	int id;
	char name[0];
};

#define FIFO_SIZE 4

static DECLARE_KFIFO(stu_fifo, struct student *, FIFO_SIZE);

static int __init _kfifo_init(void)
{
	int i = 0;
	struct student *s;

	pr_info("%s\n", __func__);

	INIT_KFIFO(stu_fifo);

	do {
		s = kmalloc(sizeof(struct student) + 32, GFP_KERNEL);
		s->id = i;
		snprintf(s->name, 32, "student_%d", i);
	} while (kfifo_put(&stu_fifo, s));

	pr_info("kfifo_is_full: %d\n", kfifo_is_full(&stu_fifo));
	pr_info("queue len: %u\n", kfifo_len(&stu_fifo));

	if (kfifo_peek(&stu_fifo, &s)) {
		pr_info("%d: %s\n", s->id, s->name);
		kfifo_skip(&stu_fifo);
		kfree(s);
	}

	while (kfifo_get(&stu_fifo, &s)) {
		pr_info("%d: %s\n", s->id, s->name);
		kfree(s);
	}

	pr_info("kfifo_is_empty: %d\n", kfifo_is_empty(&stu_fifo));

	return 0;
}

static void __exit _kfifo_exit(void)
{
	pr_info("%s\n", __func__);
}

MODULE_LICENSE("GPL");

module_init(_kfifo_init);
module_exit(_kfifo_exit);

