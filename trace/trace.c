#include <linux/module.h>
#include <linux/kthread.h>

#define CREATE_TRACE_POINTS
#include "trace.h"

static int reg_cnt;
static DEFINE_MUTEX(tsk_lock);
static struct task_struct *normal_tsk;
static struct task_struct *reg_tsk;

static int normal_fn(void *unused)
{
	int cnt = 0;

	while (!kthread_should_stop()) {
		trace_trace(cnt, "normal");
		trace_trace_class("normal");
		schedule_timeout_interruptible(HZ);
		cnt++;
	}

	return 0;
}

static int reg_fn(void *unused)
{
	int cnt = 0;

	while (!kthread_should_stop()) {
		trace_trace_fn("reg");
		trace_trace_class_fn("reg");
		schedule_timeout_interruptible(HZ);
		cnt++;
	}

	return 0;
}

int trace_reg(void)
{
	pr_info("%s\n", __func__);

	mutex_lock(&tsk_lock);

	if (reg_cnt++ > 0)
		goto out;

	reg_tsk = kthread_run(reg_fn, NULL, "reg_tsk");

out:
	mutex_unlock(&tsk_lock);

	if(!reg_tsk)
		return PTR_ERR(reg_tsk);

	return 0;
}

void trace_unreg(void)
{
	pr_info("%s\n", __func__);

	mutex_lock(&tsk_lock);

	if (--reg_cnt > 0)
		goto out;

	if (reg_tsk) {
		kthread_stop(reg_tsk);
		reg_tsk = NULL;
	}

out:
	mutex_unlock(&tsk_lock);
}

static int __init _trace_init(void)
{
	pr_info("%s\n", __func__);

	normal_tsk = kthread_run(normal_fn, NULL, "normal_tsk");
	if (!normal_tsk)
		return PTR_ERR(normal_tsk);

	return 0;
}

static void __exit _trace_exit(void)
{
	pr_info("%s\n", __func__);

	if (normal_tsk)
		kthread_stop(normal_tsk);

	mutex_lock(&tsk_lock);

	if (reg_tsk) {
		kthread_stop(reg_tsk);
		reg_tsk = NULL;
	}

	mutex_unlock(&tsk_lock);
}

module_init(_trace_init);
module_exit(_trace_exit);

MODULE_LICENSE("GPL");
