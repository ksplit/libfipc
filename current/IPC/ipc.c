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

#include <lcd-domains/thc.h>
#include <lcd-domains/thcinternal.h>

#include "../ring-chan/ring-channel.h"
#include "ipc.h"

static unsigned int tx_slot_avail = 0xC1346BAD;
static unsigned int send_message = 0xBADBEEF;
static unsigned int rx_msg_avail = 0xBADBEEF;
static unsigned int trans_complete = 0xC1346BAD;

awe_t* get_awe_from_msg_id(unsigned long msg_id)
{
	if( sizeof(unsigned long) != sizeof(awe_t*) )
		printk(KERN_WARNING "mismatched sizes in get_awe_from_msg_id\n");

	return (awe_t*)msg_id;
}

static inline void monitor_mwait(unsigned long rcx, volatile uint32_t *rax,
				 unsigned long wait_type)
{

	__monitor((void *)rax, 0, 0);
	/* TODO comment for memory barrier, why is this necessary? */
	mb();
	__mwait(wait_type, rcx);
}

static inline int check_rx_slot_available(struct ipc_message *loc)
{
	return (likely(loc->msg_status != rx_msg_avail));
}

static inline int check_tx_slot_available(struct ipc_message *loc)
{
	return (unlikely(loc->msg_status != tx_slot_avail));
}


static int wait_for_tx_slot(struct ipc_message *imsg)
{


	while (check_tx_slot_available(imsg)) {

#if defined(USE_MWAIT)
		cpu_relax();
		monitor_mwait(ecx, &imsg->msg_status, cstate_wait);
#endif//usemwait
#if defined(POLL)
		cpu_relax();
#endif
	}
	return 0;
}

static int wait_for_rx_slot(struct ipc_message *imsg, bool is_async)
{
	while (check_rx_slot_available(imsg)) { //while a slot is not available
		if( is_async )
		{
			THCYield();
		}	
		else
		{
			#if defined(USE_MWAIT)
				monitor_mwait(ecx, &imsg->msg_status, cstate_wait);
			#endif//usemwait
			#if defined(POLL)
				cpu_relax();
			#endif
		}
	}
	return 0;
}



struct task_struct *attach_thread_to_channel(struct ttd_ring_channel *chan,
                                             int CPU_PIN,
                                             int (*threadfn)(void *data)) {

	struct cpumask cpu_core;

        if (!chan)
                return NULL;

        if (CPU_PIN > num_online_cpus()) {
                pr_err("Trying to pin on cpu > than avail # cpus\n");
                return NULL;
        }

	chan->thread = kthread_create(threadfn, (void *)chan,
					   "AsyncIPC.%u", CPU_PIN);

	if (IS_ERR(chan->thread)) {
		pr_err("Error while creating kernel thread\n");
		return NULL;
	}

	get_task_struct(chan->thread);

	cpumask_clear(&cpu_core);
	cpumask_set_cpu(CPU_PIN , &cpu_core);

	set_cpus_allowed_ptr(chan->thread, &cpu_core);

        return chan->thread;
}

/*
 *  Create a channel with a ring-buffer of size pages
 */

struct ttd_ring_channel *create_channel(unsigned long size_pages)
{

	int i,ret;
	struct ttd_ring_channel *channel;



	if (((sizeof(unsigned long) * CHAR_BITS) -
	     (__builtin_clzl(size_pages*PAGE_SIZE)-1)) % 2 != 0) {
		pr_err("buffers _MUST_ be on order 2 size, ie 2^2 or 2^4 etc");
		return NULL;
	}

	channel = kzalloc(sizeof(*channel), GFP_KERNEL);

	if (!channel) {
		pr_err("could not alloc space for channel");
		return NULL;
	}

	ret = ttd_ring_channel_alloc(channel,
				     size_pages,
				     sizeof(struct ipc_message));

	if (ret != 0) {
		pr_err("Failed to alloc/Init ring channel\n");
		return NULL;
	}

	pr_debug("Channel is at %p, recs are %p to %p\n", (void*)channel,
		 channel->tx.recs,
		 channel->tx.recs + (size_pages * PAGE_SIZE));

	/* We init the buffer to say each slot is free */
	for (i = 0; i < (size_pages * PAGE_SIZE)/sizeof(int); i++)
		*((int *)channel->tx.recs+i) = tx_slot_avail;

	return channel;
}
EXPORT_SYMBOL(create_channel);

void free_channel(struct ttd_ring_channel *channel)
{

	ttd_ring_channel_free(channel);
	put_task_struct(channel->thread);
	kfree(channel);

}
EXPORT_SYMBOL(free_channel);


void send(struct ttd_ring_channel *tx, struct ipc_message *trans)
{
	trans->msg_status = send_message;
	inc_tx_slot(tx);
}
EXPORT_SYMBOL(send);

struct ipc_message *recv(struct ttd_ring_channel *rx)
{
	struct ipc_message *recv_msg;

	recv_msg = get_rx_rec(rx, sizeof(struct ipc_message));
	inc_rx_slot(rx);
	wait_for_rx_slot(recv_msg, false);
	return recv_msg;
}
EXPORT_SYMBOL(recv);

noinline struct ipc_message *async_recv(struct ttd_ring_channel *rx, unsigned long msg_id)
{
	struct ipc_message *recv_msg;


	while( true )
	{
		recv_msg = get_rx_rec(rx, sizeof(struct ipc_message));

		if( !check_rx_slot_available(recv_msg) ) //if slot is available
		{
			if( recv_msg->msg_id == msg_id )
			{
				break;
			}		
			else
			{
				awe_t* other_awe = get_awe_from_msg_id(msg_id);
				THCYield(); //THCYieldTo(other_awe);		
			}
		}
		else
		{
			THCYield();
		}
	}	

	inc_rx_slot(rx);

	return recv_msg;
}
EXPORT_SYMBOL(async_recv);

struct ipc_message *get_send_slot(struct ttd_ring_channel *tx)
{
	struct ipc_message *msg =
		(struct ipc_message *) get_tx_rec(tx, sizeof(struct ipc_message));
	wait_for_tx_slot(msg);
	return msg;
}
EXPORT_SYMBOL(get_send_slot);

void connect_channels(struct ttd_ring_channel *chan1,
		      struct ttd_ring_channel *chan2)
{
	/* exchange pointers and sizes */
	memcpy(&chan1->rx, &chan2->tx, sizeof(struct ttd_buf));
	memcpy(&chan2->rx, &chan1->tx, sizeof(struct ttd_buf));
}
EXPORT_SYMBOL(connect_channels);


/* Notify the buffer that the message slot is available and can be re-used */
void transaction_complete(struct ipc_message *msg)
{
	msg->msg_status = trans_complete;
}
EXPORT_SYMBOL(transaction_complete);


int ipc_start_thread(struct ttd_ring_channel *chan)
{
	return wake_up_process(chan->thread);
}
EXPORT_SYMBOL(ipc_start_thread);
