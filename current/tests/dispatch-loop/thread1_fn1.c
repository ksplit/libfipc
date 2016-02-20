#include "thc.h"
#include "ipc.h"
#include "thcinternal.h"
#include "ipc_dispatch.h"
#include "thread_fn_util.h"
#include <awe-mapper.h>

static struct ttd_ring_channel* channel;

static unsigned long add_nums_async(unsigned long lhs, unsigned long rhs, unsigned long msg_id, int fn_type)
{
	struct ipc_message *msg;
	unsigned long result;
	msg = get_send_slot(channel);
	msg->fn_type  = fn_type;
	msg->reg1     = lhs;
	msg->reg2     = rhs;
	msg->msg_id   = msg_id;
    msg->msg_type = msg_type_request;
	send(channel,msg);
	msg = async_recv(channel, msg_id);
	result = msg->reg1;
    printk(KERN_ERR "result is %lu\n", result);
	transaction_complete(msg);
	
	return result;
}



int thread1_fn1(void* group)
{
	volatile void ** frame = (volatile void**)__builtin_frame_address(0);
	volatile void *ret_addr = *(frame + 1);
    int num_transactions = 0;
	uint32_t id_num;
	*(frame + 1) = NULL;
    struct ttd_ring_channel_group *rcg = (struct ttd_ring_channel_group*)group;
    channel = rcg->chans[0];

    thc_init();
 	DO_FINISH_(1,{
		while (num_transactions < TRANSACTIONS) {
		printk(KERN_ERR "num_transactions: %d\n", num_transactions);

		ASYNC(
			id_num = awe_mapper_create_id();
			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_nums_async(num_transactions, 1,(unsigned long) id_num, ADD_2_FN);
		     );

		ASYNC(
			id_num = awe_mapper_create_id();
			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_nums_async(num_transactions, 2,(unsigned long) id_num, ADD_10_FN);
		     );

		ASYNC(
			id_num = awe_mapper_create_id();
			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_nums_async(num_transactions, 3,(unsigned long) id_num, ADD_2_FN);
		     );



		num_transactions++;
	}});
	pr_err("Complete\n");
	printk(KERN_ERR "lcd async exiting module and deleting ptstate");
	thc_done();

	*(frame + 1) = ret_addr;

    return 1;
}



