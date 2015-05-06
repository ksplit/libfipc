
#ifndef _INCL_GUARD
#define _INCL_GUARD

#include <linux/types.h>
#include <../ring-chan/ring-channel.h>


#if defined(USE_MWAIT)
	unsigned long ecx = 1; /*break of interrupt flag */
	unsigned long cstate_wait = 0x0; /* 4 states, 0x0, 0x1 0x10 0x20 */
#endif

/*Don't let gcc do anything cute, we need this to be 128 bytes */

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

struct ttd_ring_channel *create_channel(unsigned long size_pages, unsigned CPU);
void free_channel(struct ttd_ring_channel *channel);
void send(struct ttd_ring_channel *tx, struct ipc_message *trans);
struct ipc_message *recv(struct ttd_ring_channel *rx);
struct ipc_message *get_send_slot(struct ttd_ring_channel *tx);
void transaction_complete(struct ipc_message *msg);
void start_thread(struct ttd_ring_channel *chan);

#endif
