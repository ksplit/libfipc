#include "thc.h"
#include "ipc.h"
#include "thcinternal.h"
#include "ipc_dispatch.h"
#include "thread_fn_util.h"
#include <linux/delay.h>
#include <awe-mapper.h>

#define THREAD3_FNS_LENGTH 1


static int add_10_fn(struct ttd_ring_channel_group* group, int channel_index, struct ipc_message* msg)
{
    msleep(1000);
    unsigned long result = msg->reg1 + msg->reg2 + 10;
    struct ttd_ring_channel* chan = group->chans[channel_index];
	struct ipc_message* out_msg = get_send_slot(chan);
    printk(KERN_ERR "got to thread3\n");
    out_msg->reg1     = result;
    out_msg->msg_id   = msg->msg_id;
    out_msg->fn_type  = ADD_10_FN;
    out_msg->msg_type = msg_type_response;
    send(chan, out_msg);
    
    return 0;
}


static ipc_local_fn_t functions[THREAD3_FNS_LENGTH] = {
    {ADD_10_FN, add_10_fn}
}; 


int thread3_fn1(void* group)
{
    struct ttd_ring_channel_group* rx_group = group;
    thc_init();
    ipc_dispatch_loop(functions, THREAD3_FNS_LENGTH, rx_group, 1);
    thc_done();

    return 1;
}
