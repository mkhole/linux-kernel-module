#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/random.h>

static struct task_struct *reader;
static struct task_struct *writer;

struct book {
	uint8_t id;
	char name[16];
	struct rcu_head rcu;
};

static struct book __rcu *book_ptr;

static int reader_func(void *data)
{
	struct book *new;

	while (!kthread_should_stop()) {
		rcu_read_lock();
		new = rcu_dereference(book_ptr);
		if (new) {
			pr_info("%s: Get %d, %s\n", __func__, new->id, new->name);
		}
		rcu_read_unlock();

		msleep_interruptible(300);
	}

	return 0;
}

void book_rcu_free(struct rcu_head *head)
{
	struct book *old = container_of(head, struct book, rcu);
	pr_info("%s: Freeing %u\n", __func__, old->id);
	kfree(old);
}

static int writer_func(void *data)
{
	struct book *old, *new;

	while (!kthread_should_stop()) {
		new = kvmalloc(sizeof(struct book), GFP_KERNEL);
		if (!new) {
			msleep_interruptible(100);
			continue;
		}

		get_random_bytes(&new->id, 1);
		sprintf(new->name, "Book %u", new->id);

		old = rcu_dereference_protected(book_ptr, 1);
		rcu_assign_pointer(book_ptr, new);

		pr_info("%s: Update: %u\n", __func__, new->id);

		if (old)
			call_rcu(&old->rcu, book_rcu_free);

		msleep_interruptible(1000);
	}

	return 0;
}

static int __init _rcu_init(void)
{
	pr_info("%s\n", __func__);

	reader = kthread_run(reader_func, NULL, "reader");
	if (IS_ERR(reader))
		return PTR_ERR(reader);

	writer = kthread_run(writer_func, NULL, "writer");
	if (IS_ERR(writer)) {
		kthread_stop(reader);
		return PTR_ERR(writer);
	}

	return 0;
}

static void __exit _rcu_exit(void)
{
	pr_info("%s\n", __func__);

	kthread_stop(writer);
	kthread_stop(reader);
}

MODULE_LICENSE("GPL");

module_init(_rcu_init);
module_exit(_rcu_exit);

