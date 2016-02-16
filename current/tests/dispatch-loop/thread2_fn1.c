#include "thc.h"
#include "ipc.h"
#include "thcinternal.h"
#include "ipc_dispatch.h"
#include "thread_fn_util.h"

#define FNS_LENGTH 1
#define CHANS_NUM  1

int add_2_fn(struct ttd_ring_channel* chan, struct ipc_message* msg, void* params)
{
    unsigned long result = msg->reg1 + msg->reg2;
	struct ipc_message* out_msg = get_send_slot(chan);
    out_msg->reg1    = result;
    out_msg->msg_id  = msg->msg_id;
    out_msg->fn_type = ADD_2_FN;
    out_msg->pts     = msg->pts;
    send(chan, out_msg);
    
    return 0;
}

ipc_local_fn_t functions[FNS_LENGTH] = {
    {ADD_2_FN, add_2_fn}
}; 

int thread2_fn1(void* chan)
{
    struct ttd_ring_channel* rx_chan = chan;
    thc_init();
    ipc_dispatch_loop(functions, FNS_LENGTH, &rx_chan, CHANS_NUM);
    thc_done();

    return 1;
}
