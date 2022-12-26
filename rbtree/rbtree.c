#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rbtree.h>

struct student {
	struct rb_node rbn;
	int id;
	char name[0];
};

static struct rb_root root = RB_ROOT;

static struct student *stu_search(int id)
{
	struct rb_node *n = root.rb_node;

	while (n) {
		struct student *s = rb_entry(n, struct student, rbn);

		if (s->id == id)
			return s;

		if (s->id < id)
			n = n->rb_right;
		else
			n = n->rb_left;
	}

	return NULL;
}

static bool stu_insert(struct student *stu)
{
	struct rb_node **pn = &(root.rb_node), *parent = NULL;

	while (*pn) {
		struct student *s = rb_entry(*pn, struct student, rbn);

		if (s->id == stu->id)
			return false;

		parent = *pn;
		if (s->id < stu->id)
			pn = &((*pn)->rb_right);
		else
			pn = &((*pn)->rb_left);
	}

	rb_link_node(&stu->rbn, parent, pn);
	rb_insert_color(*pn, &root);

	return true;
}

static void stu_cleanup(void)
{
	struct rb_node *next = root.rb_node;

	while (next) {
		struct student *s = rb_entry(next, struct student, rbn);
		next = rb_next(next);
		rb_erase(&s->rbn, &root);
		kfree(s);
	}
}

static int __init test_init(void)
{
	int i;
	struct student *s;
	struct rb_node *n;

	pr_info("%s\n", __func__);

	for (i = 0; i < 5; i++) {
		s = kmalloc(sizeof(struct student) + 64, GFP_KERNEL);
		s->id = i;
		snprintf(s->name, 64, "stu_%d", i);

		if (!stu_insert(s)) {
			pr_info("Failed to insert node: %d,%s\n", s->id, s->name);
			kfree(s);
		}
	}

	for (n = rb_first(&root); n; n = rb_next(n)) {
		s = rb_entry(n, struct student, rbn);
		pr_info("%d: %s\n", s->id, s->name);
	}

	for (i = 0; i < 5; i += 2) {
		s = stu_search(i);
		if (s)
			pr_info("%d: %s\n", s->id, s->name);
	}

	return 0;
}

static void __exit test_exit(void)
{
	pr_info("%s\n", __func__);

	stu_cleanup();
}

MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);

