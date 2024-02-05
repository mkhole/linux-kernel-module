#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>

struct kprobe kp;

static int __kprobes handle_pre(struct kprobe *p, struct pt_regs *regs)
{
	pr_info("[%s] symbol name: %s\n", __func__, p->symbol_name);

#ifdef CONFIG_ARM
	pr_info("[%s] addr: %pF, pc: %lx, lr: %lx\n", __func__, p->addr, regs->ARM_pc, regs->ARM_lr);
#endif

#ifdef CONFIG_ARM64
	pr_info("<%s> p->addr = 0x%p, pc = 0x%lx, pstate = 0x%lx\n", p->symbol_name, p->addr, (long)regs->pc,
		(long)regs->pstate);
#endif

	dump_stack();
	return 0;
}

static void __kprobes handle_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	pr_info("[%s] symbol name: %s\n", __func__, p->symbol_name);
#ifdef CONFIG_ARM
	pr_info("[%s] addr: %pF, pc: %lx, lr: %lx\n", __func__, p->addr, regs->ARM_pc, regs->ARM_lr);
#endif

#ifdef CONFIG_ARM64
	pr_info("<%s> p->addr = 0x%p, pc = 0x%lx, pstate = 0x%lx\n", p->symbol_name, p->addr, (long)regs->pc,
		(long)regs->pstate);
#endif
}

static int __init kprobe_init(void)
{
	int ret;

	pr_info("%s\n", __func__);

	kp.symbol_name = "netif_receive_skb";
	kp.pre_handler = handle_pre;
	kp.post_handler = handle_post;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		pr_info("register_kprobe() failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static void __exit kprobe_exit(void)
{
	pr_info("%s\n", __func__);
	unregister_kprobe(&kp);
}

module_init(kprobe_init);
module_exit(kprobe_exit);

MODULE_LICENSE("GPL");
