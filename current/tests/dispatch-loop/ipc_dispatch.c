#include <stdio.h>
#include "ipc.h"
#include "ipc_dispatch.h"

static int foo(unsigned long* data, int length)
{
    for(int i = 0; i < length; i++)
    {
        printf("%lu\n", data[i]);
    }
}

int ipc_dispatch_loop(ipc_local_fn_t* fns, int fns_length,struct ttd_ring_channel** rx_chans, int chans_num)
    //ipc_channels, channel_length
{
    int start_ind = 0;
    struct ipc_message* curr_msg;
    DO_FINISH(
    while( true )
        {
            if( poll_recv(rx_chans, chans_num, start_ind, curr_msg) )
            {
                //check if curr_msg corresponds to existing awe,
                //else find corresponding function and execute.
            }
        }
    );
    return 0;
}

int main(void)
{
    ipc_local_fn_t test_fn = {1, foo};
    unsigned long data[] = {0,1,2,3};
    test_fn.local_fn(&data[1], 2);
    return 0;
}
