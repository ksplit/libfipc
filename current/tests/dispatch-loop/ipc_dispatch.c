#include <thc.h>
#include <thcinternal.h>
#include "ipc.h"
#include "ipc_dispatch.h"
#include <awe-mapper.h>
#include <linux/types.h>

#include "thread_fn_util.h"

//max_recv_ct just for testing
int ipc_dispatch_loop(struct ttd_ring_channel_group* rx_group, int max_recv_ct)
{
	volatile void ** frame = (volatile void**)__builtin_frame_address(0);
	volatile void *ret_addr = *(frame + 1);
    int recv_ct = 0;

    *(frame + 1) = NULL;
    //NOTE:recv_ct is just for testing
    DO_FINISH_(ipc_dispatch,{
        int curr_ind     = 0;
        int* curr_ind_pt = &curr_ind;
        struct ipc_message* curr_msg;

        uint32_t do_finish_awe_id = awe_mapper_create_id();
        while( recv_ct < max_recv_ct )
        {
            curr_ind = 0;
            if( poll_recv(rx_group, curr_ind_pt, &curr_msg) )
            {
                recv_ct++;

                //printk(KERN_ERR "poll_recv returned\n");
                //check if curr_msg corresponds to existing awe in this thread
                if( curr_msg->msg_type == msg_type_response )
                {            
                    printk(KERN_ERR "yielding to\n"); 
                    THCYieldToId(curr_msg->msg_id, do_finish_awe_id);
                }
                //else find corresponding function and execute.
                else
                {
                    if( rx_group->chans[curr_ind]->dispatch_fn )
                    {
                        ASYNC_({
                        rx_group->chans[curr_ind]->dispatch_fn(rx_group->chans[curr_ind],
                                                                curr_msg);
                        },ipc_dispatch);
                    }
                    else
                    {
                        printk(KERN_ERR "Channel %d function not allocated, message dropped\n", curr_ind_pt);
                    }
                }
            }
        }
    });

	*(frame + 1) = ret_addr;

    return 0;
}
EXPORT_SYMBOL(ipc_dispatch_loop);
