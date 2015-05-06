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


#include "../ring-chan/ring-channel.h"
#include "IPC.h"

static unsigned int tx_slot_avail = 0xC1346BAD;
static unsigned int send_message = 0xBADBEEF;
static unsigned int rx_msg_avail = 0xBADBEEF;
static unsigned int transaction_complete = 0xC1346BAD;


static inline void monitor_mwait(unsigned long rcx, volatile uint32_t *rax,
				 unsigned long wait_type)
{

	__monitor((void *)rax, 0, 0);
	/* TODO comment for memory barrier, why is this necessary? */
	mb();
	__mwait(wait_type, rcx);
}

static inline int check_rx_slot_available(struct ipc_message *loc, unsigned int token)
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

static int wait_for_rx_slot(struct ipc_message *imsg, unsigned int token)
{


	while (check_rx_slot_available(imsg)) {

#if defined(USE_MWAIT)
		monitor_mwait(ecx, &imsg->msg_status, cstate_wait);
#endif//usemwait
#if defined(POLL)
		cpu_relax();

#endif
	}
	return 0;
}


/*
  Create a channel with a ring-buffer of size pages
  Create a communication thread on CPU
*/

struct ttd_ring_channel *create_channel(unsigned long size_pages, unsigned CPU)
{

	int i,ret;
	struct cpumask cpu_core;
	struct ttd_ring_channel *channel;



	if ((__builtin_clzl(size_pages*PAGE_SIZE)-1) % 2 != 0) {
		pr_err("buffers _MUST_ be on order 2 size, ie 2^2 or 2^4 etc");
		return NULL;
	}

	channel = kzalloc(sizeof(*channel), GFP_KERNEL);

	if (!channel) {
		pr_err("could not alloc space for channel");
		return NULL;
	}

	ret = ttd_ring_channel_alloc(channel,
				     CHAN_NUM_PAGES,
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

	channel->thread = kthread_create(&dispatch, (void *)channel,
					   "betaIPC.%u", CPU);

	if (IS_ERR(channel->thread)) {
		ttd_ring_channel_free(channel);
		kfree(channel);
		pr_err("Error while creating kernel thread\n");
		return NULL;
	}


	get_task_struct(channel->thread);

	cpumask_clear(&cpu_core);
	cpumask_set_cpu(CPU,&cpu_core);

	set_cpus_allowed_ptr(channel->thread, &cpu_core);


	return channel;
}
EXPORT_SYMBOL(create_channel);

void free_channel(struct ttd_ring_channel *channel)
{
	put_task_struct(channel->thread);
	ttd_ring_channel_free(channel);
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

	recv_msg = get_rx_rec(rx);
	inc_rx_slot(rx);
	wait_for_rx_slot(recv_msg);
	return recv_msg;
}
EXPORT_SYMBOL(recv);

struct ipc_message *get_send_slot(struct ttd_ring_channel *tx)
{
	struct ipc_message *msg =
		(struct ipc_message *) get_tx_rec(tx);
	wait_for_tx_slot(msg);
	return msg;
}
EXPORT_SYMBOL(get_send_slot);

void connect_channels(struct ttd_ring_channel *chan1,
		      struct ttd_ring_channel *chan2)
{
	/* exchange pointers and sizes */
	memcpy(&chan1->rx_buf, &chan2->tx_buf, sizeof(struct ttd_buf));
	memcpy(&chan2->rx_buf, &chan1->tx_buf, sizeof(struct ttd_buf));
}
EXPORT_SYMBOL(connect_channels);


/* Notify the buffer that the message slot is available and can be re-used */
void complete_transaction(struct ipc_message *msg)
{
	msg->msg_satus = transaction_complete;
}
