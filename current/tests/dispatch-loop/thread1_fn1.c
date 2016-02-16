#include "thc.h"
#include "ipc.h"
#include "thcinternal.h"
#include "ipc_dispatch.h"
#include "thread_fn_util.h"
#include <awe-mapper.h>

static struct ttd_ring_channel* channel;

static unsigned long add_2_nums_async(unsigned long lhs, unsigned long rhs, unsigned long msg_id)
{
	struct ipc_message *msg;
	unsigned long result;
	msg = get_send_slot(channel);
	msg->fn_type = ADD_2_FN;
	msg->reg1    = lhs;
	msg->reg2    = rhs;
	msg->msg_id  = msg_id;
    msg->pts     = (unsigned long)current->ptstate;
	send(channel,msg);
	msg = async_recv(channel, msg_id);
	result = msg->reg1;
    printk(KERN_ERR "result is %lu\n", result);
	transaction_complete(msg);
	
	return result;
}



int thread1_fn1(void* chan)
{
	volatile void ** frame = (volatile void**)__builtin_frame_address(0);
	volatile void *ret_addr = *(frame + 1);
	*(frame + 1) = NULL;
    int num_transactions = 0;
    channel = chan;

    thc_init();
 	DO_FINISH(
		uint32_t id_num;
		while (num_transactions < TRANSACTIONS / 3) {
		if((num_transactions * 3) % 10 == 0)
		{
			printk(KERN_ERR "num_transactions: %d\n", num_transactions * 3);
		}

		ASYNC(
			id_num = awe_mapper_create_id();
			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_2_nums_async(num_transactions, 1,(unsigned long) id_num);
		     );

		ASYNC(
			id_num = awe_mapper_create_id();
			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_2_nums_async(num_transactions, 2,(unsigned long) id_num);
		     );

		ASYNC(
			id_num = awe_mapper_create_id();
			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_2_nums_async(num_transactions, 3,(unsigned long) id_num);
		     );

		num_transactions++;
	});
	pr_err("Complete\n");
	printk(KERN_ERR "lcd async exiting module and deleting ptstate");
	thc_done();

	*(frame + 1) = ret_addr;

    return 1;
}



