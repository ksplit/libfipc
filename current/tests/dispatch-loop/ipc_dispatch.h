#ifndef IPC_DISPATCH_H
#define IPC_DISPATCH_H


typedef struct ipc_local_fn{
    int fn_type;
    int (*local_fn)(struct ttd_ring_channel*,struct ipc_message*, void*);
} ipc_local_fn_t;

int ipc_dispatch_loop(ipc_local_fn_t* fns, int fns_length,struct ttd_ring_channel** rx_chans, int chans_num);
#endif
