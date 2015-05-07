#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/irqflags.h>
#include <linux/kthread.h>
#include <linux/cpumask.h>
#include <linux/preempt.h>
#include <asm/uaccess.h>
#include <asm/mwait.h>
#include <asm/page_types.h>
#include <asm/cpufeature.h>
#include <linux/ktime.h>
#include <linux/sort.h>
#include <asm/tsc.h>

#include "rpc.h"

static struct ttd_ring_channel *chan1;
static struct ttd_ring_channel *chan2;

MODULE_LICENSE("GPL");

static void setup_tests(void)
{
	chan1 = create_channel(4,0);
	if (!chan1) {
		pr_err("Failed to create channel 1");
		return;
	}
	chan2 = create_channel(4,3);
	if (!chan2) {
		pr_err("Failed to create channel 2");
		free_channel(chan1);
		kfree(chan1);
		return;
	}
	connect_channels(chan1,chan2);
	ipc_start_thread(chan1);
	ipc_start_thread(chan2);
}

static int __init rpc_init(void)
{
	int ret = 0;

	if (!this_cpu_has(X86_FEATURE_MWAIT))
		return -EPERM;

	setup_tests();

	return ret;
}
static int __exit rpc_rmmod(void)
{

	return 0;
}

module_init(rpc_init);
module_exit(rpc_rmmod);
