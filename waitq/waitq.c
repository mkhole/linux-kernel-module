#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/random.h>

static struct task_struct *producer;
static struct task_struct *consumer[8];
static DECLARE_WAIT_QUEUE_HEAD(wait_q);
static DEFINE_SPINLOCK(cmdq_lock);
static LIST_HEAD(cmdq);

struct cmd {
	int number;
	struct list_head list;
};

static int pro_worker(void *unused)
{
	struct cmd *c;

	while (!kthread_should_stop()) {
		c = kvmalloc(sizeof(struct cmd), GFP_KERNEL);
		if (!c)
			continue;

		get_random_bytes(&c->number, sizeof(c->number));

		spin_lock(&cmdq_lock);
		list_add_tail(&c->list, &cmdq);
		pr_info("%s [cpu %d]: Add 0x%x\n", current->comm, smp_processor_id(), c->number);
		spin_unlock(&cmdq_lock);

		wake_up_interruptible(&wait_q);
		msleep_interruptible(1000);
	}

	return 0;
}

static int con_worker(void *unused)
{
	struct cmd *c;

	while (!kthread_should_stop()) {
		wait_event_interruptible(wait_q, !list_empty(&cmdq) || kthread_should_stop());

		spin_lock(&cmdq_lock);
		if (!list_empty(&cmdq)) {
			c = list_first_entry(&cmdq, struct cmd, list);
			list_del(&c->list);
			pr_info("%s [cpu %d]: Got 0x%x\n", current->comm, smp_processor_id(), c->number);
		}
		spin_unlock(&cmdq_lock);
	}

	return 0;
}

static int __init waitq_init(void)
{
	int i;
	int errno;

	pr_info("%s\n", __func__);

	producer = kthread_run(pro_worker, NULL, "producer");
	if (IS_ERR(producer))
		goto err;

	for (i = 0; i < 8; i++) {
		consumer[i] = kthread_run(con_worker, NULL, "consumer%d", i);
		if (IS_ERR(consumer))
			goto err;
	}

	return 0;

err:
	if (IS_ERR(producer))
		return PTR_ERR(producer);

	kthread_stop(producer);

	for (i = 0; i < 8; i++) {
		if (IS_ERR(consumer[i])) {
			errno = PTR_ERR(consumer[i]);
			break;
		}
		kthread_stop(consumer[i]);
	}

	return errno;
}

static void __exit waitq_exit(void)
{
	int i;

	pr_info("%s\n", __func__);

	if (!IS_ERR(producer))
		kthread_stop(producer);

	wake_up_interruptible_all(&wait_q);

	for (i = 0; i < 8; i++) {
		if (IS_ERR_OR_NULL(consumer[i]))
			break;

		kthread_stop(consumer[i]);
	}
}

MODULE_LICENSE("GPL");

module_init(waitq_init);
module_exit(waitq_exit);

