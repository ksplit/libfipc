
#ifndef _INCL_GUARD
#define _INCL_GUARD

#include <linux/types.h>
#include "../ring-chan/ring-channel.h"

#define CHAR_BITS 8

#if defined(USE_MWAIT)
<<<<<<< HEAD:current/IPC/ipc.h
	unsigned long ecx = 1; /*break of interrupt flag */
	unsigned long cstate_wait = 0x1; /* 4 states, 0x0, 0x1 0x10 0x20 */
=======
	unsigned long ecx = 1; /\*break of interrupt flag *\/ */
	unsigned long cstate_wait = 0x0; /* 4 states, 0x0, 0x1 0x10 0x20 */
>>>>>>> lcd:old2/IPC/ipc.h
#endif

/*Don't let gcc do anything cute, we need this to be 64 bytes */

/* TODO CONFIRM ALIGNMENT REQUIREMENTS FOR ULONG! */


typedef enum {
    msg_type_unspecified,
    msg_type_request,
    msg_type_response,
} msg_type_t;

struct ipc_message{
	int fn_type; /* looks like standard converts ENUM members to ints */
	unsigned long reg1;
	unsigned long reg2;
	unsigned long reg3;
	unsigned long reg4;
	unsigned long reg5;
<<<<<<< HEAD:current/IPC/ipc.h
	unsigned long reg6;
	unsigned long reg7;
	volatile uint32_t msg_status;
}__attribute__((packed));

=======
    #ifdef USE_ASYNC
        unsigned long msg_type;
    #else
	    unsigned long reg6;
    #endif
	unsigned long msg_id;
	volatile uint32_t msg_status;
}__attribute__((packed));

typedef struct ttd_ring_channel_group ttd_ring_channel_group_t;
>>>>>>> lcd:old2/IPC/ipc.h

struct ttd_ring_channel *create_channel(unsigned long size_pages);
struct task_struct *attach_thread_to_channel(struct ttd_ring_channel *chan,
                                             int CPU_PIN,
                                             int (*threadfn)(void *data));
struct task_struct *attach_channels_to_thread(ttd_ring_channel_group_t *chan_group,
                                             int CPU_PIN,
                                             int (*threadfn)(void *data));
void free_channel(struct ttd_ring_channel *channel);
void send(struct ttd_ring_channel *tx, struct ipc_message *trans);
struct ipc_message *recv(struct ttd_ring_channel *rx);
bool poll_recv(struct ttd_ring_channel_group* rx_group, int* curr_ind, struct ipc_message** msg);
struct ipc_message *async_recv(struct ttd_ring_channel *rx, unsigned long msg_id);
struct ipc_message *get_send_slot(struct ttd_ring_channel *tx);
void transaction_complete(struct ipc_message *msg);
int ipc_start_thread(struct task_struct* thread);
void connect_channels(struct ttd_ring_channel *c1, struct ttd_ring_channel *t2);
void prefetch_rx(struct ttd_ring_channel *rx);
void prefetch_tx(struct ttd_ring_channel *rx);
void prefetch_tx_range(struct ttd_ring_channel *rx, unsigned long range);
#endif
