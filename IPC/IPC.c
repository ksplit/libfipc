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


#include "ring-chan/ring-channel.h"
#include "betaModule.h"
#include "num_configs.h"



static inline void monitor_mwait(unsigned long rcx, volatile uint32_t *rax,
				 unsigned long wait_type)
{

	__monitor((void *)rax, 0, 0);
	/* TODO comment for memory barrier, why is this necessary? */
	mb();
	__mwait(wait_type, rcx);
}

static inline int check_cons_slot_available(struct ipc_message *loc, unsigned int token)
{
	return (likely(loc->msg_status != token));
}

static inline int check_prod_slot_available(struct ipc_message *loc, unsigned int token)
{
	return (unlikely(loc->msg_status != token));
}


static struct ipc_message * get_next_available_slot(struct ttd_ring_channel *chan,
					     unsigned long bucket)
{
	return (struct ipc_message *) ttd_ring_channel_get_rec_slow(chan,bucket);
}


static int wait_for_producer_slot(struct ipc_message *imsg, unsigned int token)
{


	while (check_prod_slot_available(imsg, token)) {

#if defined(USE_MWAIT)
		cpu_relax();
		monitor_mwait(ecx, &imsg->msg_status, cstate_wait);
#endif//usemwait
#if defined(POLL)
		cpu_relax();
		//	asm volatile("pause" ::: "memory");
			//__builting_ia32_pause();
#endif
	}
	return 0;
}

static int wait_for_consumer_slot(struct ipc_message *imsg, unsigned int token)
{


	while (check_cons_slot_available(imsg, token)) {

#if defined(USE_MWAIT)
		monitor_mwait(ecx, &imsg->msg_status, cstate_wait);
#endif//usemwait
#if defined(POLL)
		cpu_relax();
		//asm volatile("pause" ::: "memory");
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
	struct ttd_ring_channel *channel = kzalloc(sizeof(*channel), GFP_KERNEL);

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
		 channel->recs,
		 channel->recs + (size_pages * PAGE_SIZE));

	for(i = 0; i < (size_pages * PAGE_SIZE)/sizeof(int); i++)
		*((int *)channel->recs+i) = send_slot_avail;

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
	ttd_ring_channel_set_prod(tx, ttd_ring_channel_get_prod(tx)++);
}
EXPORT_SYMBOL(send);

struct ipc_message *recv(struct ttd_ring_channel *rx)
{
	struct ipc_message *recv_msg;

	recv_msg = get_next_available_slot(rx, ttd_ring_channel_get_cons(rx));
	ttd_ring_channel_set_cons(rx,ttd_ring_channel_get_cons(rx)++);
	wait_for_consumer_slot(recv_msg, recv_message_token);
	return recv_msg;
}
EXPORT_SYMBOL(recv);

struct ipc_message *get_send_slot(struct ttd_ring_channel *tx)
{
	struct ipc_message *msg = get_next_available_slot(tx, ttd_channel_get_prod(tx));
	wait_for_producer_slot(msg, send_slot_avail);
	return msg;
}
EXPORT_SYMBOL(get_send_slot);
