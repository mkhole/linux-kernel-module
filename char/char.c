#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/mutex.h>

#define DEV_NAME "test_char"
#define MSG_LEN 128UL

struct char_pdata {
	struct mutex lock;
	int major_id;
	struct class *cls;
	char msg[];
};

struct char_pdata *pdata;

static int char_open(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);
	return 0;
}

static ssize_t char_read(struct file *file, char __user *buff, size_t size, loff_t *off)
{
	int ret;

	pr_info("%s\n", __func__);

	if (*off >= MSG_LEN || !size)
		return 0;

	if (size > MSG_LEN - *off)
		size = MSG_LEN - *off;

	mutex_lock(&pdata->lock);

	ret = copy_to_user(buff, pdata->msg + *off, size);
	if (ret == size) {
		pr_info("copy_to_user() failed\n");
		return -EFAULT;
	}

	mutex_unlock(&pdata->lock);

	size -= ret;

	return size;
}

static ssize_t char_write(struct file *file, const char __user *buff, size_t size, loff_t *off)
{
	int ret;

	pr_info("%s\n", __func__);

	if (*off >= MSG_LEN || !size)
		return 0;

	if (size > MSG_LEN - *off)
		size = MSG_LEN - *off;

	mutex_lock(&pdata->lock);

	ret = copy_from_user(pdata->msg + *off, buff, size);
	if (ret) {
		pr_info("copy_from_user() failed\n");
		return -EFAULT;
	}

	mutex_unlock(&pdata->lock);

	size -= ret;
	*off += size;

	return size;
}

static int char_release(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);
	return 0;
}

static struct file_operations fops = {
	.open = char_open,
	.read = char_read,
	.write = char_write,
	.release = char_release,
};

static int __init char_init(void)
{
	int ret;
	struct device *dev;

	pr_info("%s\n", __func__);

	pdata = kzalloc(sizeof(struct char_pdata) + MSG_LEN, GFP_KERNEL);
	if (IS_ERR(pdata)) {
		pr_info("kzalloc() failed\n");
		return PTR_ERR(pdata);
	}

	mutex_init(&pdata->lock);

	pdata->major_id = register_chrdev(0, DEV_NAME, &fops);
	if (pdata->major_id < 0) {
		pr_info("register_chrdev() failed\n");
		return -1;
	}

	pr_info("Major ID: %d\n", pdata->major_id);

	pdata->cls = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(pdata->cls)) {
		pr_info("class_create() failed\n");
		ret = PTR_ERR(pdata->cls);
		goto err_cls;
	}

	dev = device_create(pdata->cls, NULL, MKDEV(pdata->major_id, 0), NULL, DEV_NAME);
	if (IS_ERR(dev)) {
		pr_info("device_create() failed\n");
		ret = PTR_ERR(dev);
		goto err_dev;
	}

	return 0;

err_dev:
	class_destroy(pdata->cls);
err_cls:
	unregister_chrdev(pdata->major_id, DEV_NAME);

	return ret;
}

static void __exit char_exit(void)
{
	pr_info("%s\n", __func__);

	device_destroy(pdata->cls, MKDEV(pdata->major_id, 0));
	class_destroy(pdata->cls);
	unregister_chrdev(pdata->major_id, DEV_NAME);

	kfree(pdata);
}

MODULE_LICENSE("GPL");

module_init(char_init);
module_exit(char_exit);

