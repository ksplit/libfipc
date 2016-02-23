#ifndef IPC_DISPATCH_H
#define IPC_DISPATCH_H

int ipc_dispatch_loop(struct ttd_ring_channel_group* rx_group, int max_recv_ct);
#endif
