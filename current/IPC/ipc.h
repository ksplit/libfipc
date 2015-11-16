
#ifndef _INCL_GUARD
#define _INCL_GUARD

#include <linux/types.h>
#include <../ring-chan/ring-channel.h>

#define CHAR_BITS 8

#if defined(USE_MWAIT)
	unsigned long ecx = 1; /*break of interrupt flag */
	unsigned long cstate_wait = 0x1; /* 4 states, 0x0, 0x1 0x10 0x20 */
#endif

/*Don't let gcc do anything cute, we need this to be 64 bytes */

/* TODO CONFIRM ALIGNMENT REQUIREMENTS FOR ULONG! */

struct ipc_message{
	int fn_type; /* looks like standard converts ENUM members to ints */
	unsigned long reg1;
	unsigned long reg2;
	unsigned long reg3;
	unsigned long reg4;
	unsigned long reg5;
	unsigned long reg6;
	unsigned long reg7;
	volatile uint32_t msg_status;
}__attribute__((packed));


struct ttd_ring_channel *create_channel(unsigned long size_pages);
struct task_struct *attach_thread_to_channel(struct ttd_ring_channel *chan,
                                             int CPU_PIN,
                                             int (*threadfn)(void *data));
void free_channel(struct ttd_ring_channel *channel);
void send(struct ttd_ring_channel *tx, struct ipc_message *trans);
struct ipc_message *recv(struct ttd_ring_channel *rx);
struct ipc_message *async_recv(struct ttd_ring_channel *rx, unsigned long msg_id);
struct ipc_message *get_send_slot(struct ttd_ring_channel *tx);
void transaction_complete(struct ipc_message *msg);
int ipc_start_thread(struct ttd_ring_channel *chan);
void connect_channels(struct ttd_ring_channel *c1, struct ttd_ring_channel *t2);
void prefetch_rx(struct ttd_ring_channel *rx);
void prefetch_tx(struct ttd_ring_channel *rx);
void prefetch_tx_range(struct ttd_ring_channel *rx, unsigned long range);
#endif
