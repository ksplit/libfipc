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

#define CHAN1_CPU 1
#define CHAN2_CPU 3
#define CHAN_NUM_PAGES 4

static struct ttd_ring_channel *chan1;
static struct ttd_ring_channel *chan2;

MODULE_LICENSE("GPL");

static void setup_tests(void)
{
	chan1 = create_channel(CHAN_NUM_PAGES);
	if (!chan1) {
		pr_err("Failed to create channel 1");
		return;
	}
	chan2 = create_channel(CHAN_NUM_PAGES);
	if (!chan2) {
		pr_err("Failed to create channel 2");
		free_channel(chan1);
		return;
	}
	connect_channels(chan1, chan2);

        /* Create a thread for each channel to utilize, pin it to a core.
         * Pass a function pointer to call on wakeup.
         */
        if (attach_thread_to_channel(chan1, CHAN1_CPU, callee) == NULL ||
            attach_thread_to_channel(chan2, CHAN2_CPU, caller) == NULL ) {
                ttd_ring_channel_free(chan1);
                ttd_ring_channel_free(chan2);
                kfree(chan1);
                kfree(chan2);
                return;
        }

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
