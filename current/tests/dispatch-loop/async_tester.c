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

#include "thread_fn_util.h"
#include "../../IPC/ipc.h"
#define CHAN_NUM_PAGES 4
#define THREAD1_CPU 1
#define THREAD2_CPU 3
#define THREAD3_CPU 4

MODULE_LICENSE("GPL");

static struct ttd_ring_channel_group group1;
static struct ttd_ring_channel_group group2;
static struct ttd_ring_channel_group group3;


static void alloc_chans(struct ttd_ring_channel_group* chan_group, int num_pages)
{
    int i;
    for(i = 0; i < chan_group->chans_length; i++)
    {
        chan_group->chans[i] = create_channel(num_pages);
	    if ( !chan_group->chans[i] ) 
        {
		    pr_err("Failed to create channel\n");
		    return;
	    }
    }
}


static void setup_tests(void)
{
    const size_t thread1_chans = 1;
    const size_t thread2_chans = 2;
    const size_t thread3_chans = 1;

    channel_group_alloc(&group1, thread1_chans);
    channel_group_alloc(&group2, thread2_chans);
    channel_group_alloc(&group3, thread3_chans);

    alloc_chans(&group1, CHAN_NUM_PAGES);
    alloc_chans(&group2, CHAN_NUM_PAGES);
    alloc_chans(&group3, CHAN_NUM_PAGES);

	connect_channels(group1.chans[0], group2.chans[0]);
	connect_channels(group2.chans[1], group3.chans[0]);

    /* Create a thread for each channel to utilize, pin it to a core.
     * Pass a function pointer to call on wakeup.
     */
    attach_channels_to_thread(&group1, 
                              THREAD1_CPU, 
                              thread1_fn1);
    attach_channels_to_thread(&group2, 
                              THREAD2_CPU, 
                              thread2_fn1);
    attach_channels_to_thread(&group3, 
                              THREAD3_CPU, 
                              thread3_fn1);

    if ( group1.thread == NULL || group2.thread == NULL || group3.thread == NULL) {
            ttd_ring_channel_free(group1.chans[0]);
            ttd_ring_channel_free(group2.chans[0]);
            ttd_ring_channel_free(group3.chans[0]);
            channel_group_free(&group1);
            channel_group_free(&group2);
            channel_group_free(&group3);
            return;
    }
	ipc_start_thread(group1.thread);
	ipc_start_thread(group2.thread);
	ipc_start_thread(group3.thread);
}

static int __init async_dispatch_start(void)
{
	#if defined(USE_MWAIT)
        	if (!this_cpu_has(X86_FEATURE_MWAIT))
		{
			printk(KERN_ERR "CPU does not have X86_FEATURE_MWAIT ");
                	return -EPERM;
		}
	#endif
        setup_tests();

        return 0;
}

static int __exit async_dispatch_end(void)
{
    printk(KERN_ERR "done\n");
    return 0;
}

module_init(async_dispatch_start);
module_exit(async_dispatch_end);
