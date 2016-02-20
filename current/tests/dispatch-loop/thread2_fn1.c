#include "thc.h"
#include "ipc.h"
#include "thcinternal.h"
#include "ipc_dispatch.h"
#include "thread_fn_util.h"
#include "awe-mapper.h"
#define THREAD2_FNS_LENGTH 2


//Just returns a value back to thread 1
static int add_2_fn(struct ttd_ring_channel_group* group, int channel_index, struct ipc_message* msg)
{
    struct ttd_ring_channel * chan = group->chans[channel_index];
    unsigned long result = msg->reg1 + msg->reg2;
	struct ipc_message* out_msg = get_send_slot(chan);
    out_msg->reg1     = result;
    out_msg->msg_id   = msg->msg_id;
    out_msg->fn_type  = ADD_2_FN;
    out_msg->msg_type = msg_type_response;
    send(chan, out_msg);
    
    return 0;
}


//Receives a value from thread1, then passes it to thread 3 and returns that result to thread 1
static int add_10_fn(struct ttd_ring_channel_group* group, int channel_index, struct ipc_message* msg)
{
    struct ttd_ring_channel * thread1_chan = group->chans[0]; 
    struct ttd_ring_channel * thread3_chan = group->chans[1]; 
 	struct ipc_message* thread3_msg = get_send_slot(thread3_chan);
 	struct ipc_message* thread1_result;
    unsigned long saved_msg_id = msg->msg_id;
    unsigned long new_msg_id   = awe_mapper_create_id();

    if( channel_index != 0 )
    {
        printk(KERN_ERR "CHANNEL INDEX WRONG\n");
    }
	thread3_msg->fn_type  = msg->fn_type;
	thread3_msg->reg1     = msg->reg1;
	thread3_msg->reg2     = msg->reg2;
	thread3_msg->msg_id   = new_msg_id;
    thread3_msg->msg_type = msg_type_request;
	send(thread3_chan,thread3_msg);

	msg = async_recv(thread3_chan, new_msg_id);

    thread1_result = get_send_slot(thread1_chan);
	thread1_result->fn_type  = msg->fn_type;
	thread1_result->reg1     = msg->reg1;
	thread1_result->reg2     = msg->reg2;
	thread1_result->msg_id   = saved_msg_id;
    thread1_result->msg_type = msg_type_response;
	send(thread1_chan,thread1_result);

    return 0;
}


static ipc_local_fn_t functions[THREAD2_FNS_LENGTH] = {
    {ADD_2_FN, add_2_fn},
    {ADD_10_FN, add_10_fn}
}; 


int thread2_fn1(void* group)
{
    struct ttd_ring_channel_group* rx_group = group;
    thc_init();
    ipc_dispatch_loop(functions, THREAD2_FNS_LENGTH, rx_group, 3);
    thc_done();

    return 1;
}
