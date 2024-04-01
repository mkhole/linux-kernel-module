#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/seqlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define READER_NUM 10
#define WRITER_NUM 2

struct sample_data {
	int value;
	seqlock_t lock;
};

struct sample_data data;

static struct task_struct *reader_tasks[READER_NUM];
static struct task_struct *writer_tasks[WRITER_NUM];

static void read_sample(struct sample_data *data)
{
	unsigned int seq;
	int value;

	do {
		seq = read_seqbegin(&data->lock);
		value = data->value;
	} while (read_seqretry(&data->lock, seq));

	pr_info("Rader thread lockless: Sample data value: %d\n", value);
}

static void read_sample_lock(struct sample_data *data)
{
	unsigned int seq = 0;
	int value;

	do {
		read_seqbegin_or_lock(&data->lock, &seq);
		value = data->value;
	} while (need_seqretry(&data->lock, seq));

	done_seqretry(&data->lock, seq);

	pr_info("Reader thread spinlock: Sample data value: %d\n", value);
}

static int reader_thread(void *arg)
{
	struct sample_data *data = (struct sample_data *)arg;

	while (!kthread_should_stop()) {
		read_sample(data);
		msleep_interruptible(1000);

		if (kthread_should_stop())
			break;

		read_sample_lock(data);
		msleep_interruptible(1000);
	}

	return 0;
}

static void update_sample_data(struct sample_data *data)
{
	write_seqlock(&data->lock);
	data->value++;
	pr_info("Writer thread: Updated sample data value: %d\n", data->value);
	write_sequnlock(&data->lock);
}

static int writer_thread(void *arg)
{
	struct sample_data *data = (struct sample_data *)arg;

	while (!kthread_should_stop()) {
		update_sample_data(data);
		msleep_interruptible(2000);
	}

	return 0;
}

static int __init _seqlock_init(void)
{
	int i;
	bool err = false;

	seqlock_init(&data.lock);

	for (i = 0; i < READER_NUM; i++) {
		reader_tasks[i] = kthread_run(reader_thread, (void *)&data, "reader_thread");
		if (IS_ERR(reader_tasks[i])) {
			err = true;
			break;
		}
	}

	if (err)
		return -1;

	for (i = 0; i < WRITER_NUM; i++) {
		writer_tasks[i] = kthread_run(writer_thread, (void *)&data, "writer_thread");
		if (IS_ERR(writer_tasks[i])) {
			err = true;
			break;
		}
	}

	return err ? -1 : 0;
}

static void __exit _seqlock_exit(void)
{
	int i;

	for (i = 0; i < READER_NUM; i++)
		kthread_stop(reader_tasks[i]);

	for (i = 0; i < WRITER_NUM; i++)
		kthread_stop(writer_tasks[i]);
}

MODULE_LICENSE("GPL");

module_init(_seqlock_init);
module_exit(_seqlock_exit);

