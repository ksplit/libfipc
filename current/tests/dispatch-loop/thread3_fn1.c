#include "thc.h"
#include "ipc.h"
#include "thcinternal.h"
#include "ipc_dispatch.h"
#include "thread_fn_util.h"
#include <linux/delay.h>
#include <awe-mapper.h>

#define THREAD3_FNS_LENGTH 1

static struct ttd_ring_channel_group* rx_group;


static int add_10_fn(struct ttd_ring_channel* chan, struct ipc_message* msg)
{
    unsigned long msg_id = msg->msg_id;
    unsigned long reg1 = msg->reg1;
    unsigned long reg2 = msg->reg2;
    transaction_complete(msg);
    msleep(5);
    unsigned long result = reg1 + reg2 + 10;
	struct ipc_message* out_msg = get_send_slot(chan);
    printk(KERN_ERR "got to thread3\n");
    out_msg->reg1     = result;
    out_msg->msg_id   = msg_id;
    out_msg->fn_type  = ADD_10_FN;
    out_msg->msg_type = msg_type_response;
    send(chan, out_msg);
    
    return 0;
}


static int thread2_dispatch_fn(struct ttd_ring_channel* chan, struct ipc_message* msg)
{
    switch( msg->fn_type )
    {
        case ADD_10_FN:
            return add_10_fn(chan, msg);
        default:
            printk(KERN_ERR "FN: %d is not a valid function type\n", msg->fn_type);
    }
    return 1;
}

int thread3_fn1(void* group)
{
    thc_init();
    rx_group = (struct ttd_ring_channel_group*)group;
    rx_group->chans[0]->dispatch_fn = thread2_dispatch_fn;
    ipc_dispatch_loop(rx_group, TRANSACTIONS / THD3_INTERVAL);
    thc_done();

    return 1;
}
