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
#include <lcd-domains/awe-mapper.h>
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
EXPORT_SYMBOL(get_awe_from_msg_id);

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

static int wait_for_rx_slot(struct ipc_message *imsg)
{
	while (check_rx_slot_available(imsg)) { //while a slot is not available
			#if defined(USE_MWAIT)
				monitor_mwait(ecx, &imsg->msg_status, cstate_wait);
			#endif//usemwait
			#if defined(POLL)
				cpu_relax();
			#endif
		}
	return 0;
}

static struct task_struct *attach_data_to_channel(void *chan_data,
                                             int CPU_PIN,
                                             int (*threadfn)(void *data)) {
	struct cpumask cpu_core;

        if (!chan_data)
                return NULL;

        if (CPU_PIN > num_online_cpus()) {
                pr_err("Trying to pin on cpu > than avail # cpus\n");
                return NULL;
        }

	struct task_struct* thread = kthread_create(threadfn, chan_data,
					   "AsyncIPC.%u", CPU_PIN);

	if (IS_ERR(thread)) {
		pr_err("Error while creating kernel thread\n");
		return NULL;
	}

	get_task_struct(thread);

	cpumask_clear(&cpu_core);
	cpumask_set_cpu(CPU_PIN , &cpu_core);

	set_cpus_allowed_ptr(thread, &cpu_core);

    return thread;
}



struct task_struct * attach_channels_to_thread(ttd_ring_channel_group_t *chan_group, 
                                                int CPU_PIN,
                                                int (*threadfn)(void *data))
{
    return attach_data_to_channel((void *)chan_group, CPU_PIN, threadfn);
}
EXPORT_SYMBOL(attach_channels_to_thread);



struct task_struct *attach_thread_to_channel(struct ttd_ring_channel *chan,
                                             int CPU_PIN,
                                             int (*threadfn)(void *data)) {

    return attach_data_to_channel((void *)chan, CPU_PIN, threadfn);
}
EXPORT_SYMBOL(attach_thread_to_channel);

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
	kfree(channel);
}
EXPORT_SYMBOL(free_channel);

void free_thread(struct task_struct *thread)
{
	put_task_struct(thread);
}
EXPORT_SYMBOL(free_thread);


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
	wait_for_rx_slot(recv_msg);
	return recv_msg;
}
EXPORT_SYMBOL(recv);

/*
Takes an array of rx channels to iterate over. This function does one
loop over the array and populates 'msg' with a received message and returns true
if there is a message, else it returns false.
curr_ind: the index to start iterating and wrap around to. The value of this when
the function is finished will be the index of the ipc that has a message.
NOTE: right now this just checks the first rx slot for each channel that previously didn't have a message.
To make this check for everything where there could be a message, it would need to check the interval [rx, tx]
*/
bool poll_recv(struct ttd_ring_channel** rx_chans, int chans_num, int* curr_ind, struct ipc_message** msg)
{
    struct ttd_ring_channel* curr_chan;
	struct ipc_message *recv_msg;
    int i;
    for( i = 0; i < chans_num; i++ )
    {
        *curr_ind  = ((*curr_ind) + i) % chans_num;
        curr_chan = rx_chans[*curr_ind];
	    recv_msg  = get_rx_rec(curr_chan, sizeof(struct ipc_message));

        if( !check_rx_slot_available(recv_msg) ) //if message exists
        {
            *msg = recv_msg;
	        inc_rx_slot(curr_chan);
            return true;
        }
    }

    return false;
}
EXPORT_SYMBOL(poll_recv);

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
				//printk(KERN_ERR "MESSAGE ID RECEIVED IS: %lx\n", recv_msg->msg_id);	
				//printk(KERN_ERR "MESSAGE ID FOR CONTEXT IS: %lx\n", msg_id);	
		        printk(KERN_ERR "CALLING YIELD TO\n");
                if( recv_msg->msg_type == msg_type_response )
                {
				    THCYieldToId((uint32_t) recv_msg->msg_id, (uint32_t) msg_id);
                }
                else
                {
                    THCYieldAndSave((uint32_t) msg_id);
                }
			}
		}
		else
		{
			//printk(KERN_ERR "spin yield\n");
			THCYieldAndSave((uint32_t) msg_id);
		}
	}	
	awe_mapper_remove_id((uint32_t)msg_id);
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


int ipc_start_thread(struct task_struct* thread)
{
	return wake_up_process(thread);
}
EXPORT_SYMBOL(ipc_start_thread);
