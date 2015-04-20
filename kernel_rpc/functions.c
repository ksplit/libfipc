
#include "betaModule.h"


static struct ttd_ring_channel *prod_channel;
static struct ttd_ring_channel *cons_channel;

extern unsigned int recv_ack;
extern unsigned int send_slot_avail;
extern unsigned int send_message;
extern unsigned int recv_message;
extern unsigned long send_slot;
extern unsigned long recv_slot;


extern struct ipc_message  *get_next_available_slot(struct ttd_ring_channel*, unsigned long);
extern int wait_for_producer_slot(struct ipc_message*, unsigned int);
extern void send(struct ipc_message *);
extern struct ipc_message *recv(struct ttd_ring_channel *);

void register_chans(struct ttd_ring_channel *prod, struct ttd_ring_channel *cons)
{
	prod_channel = prod;
	cons_channel = cons;
}

unsigned long foo(unsigned long a, unsigned long b, unsigned long c)
{
	struct ipc_message *send_msg, *recv_msg;
	unsigned long ret;
	send_msg = get_next_available_slot(prod_channel, send_slot);
	wait_for_producer_slot(prod_channel, send_slot_avail);
	send_msg->type = FOO;
	send_msg->reg1 = a;
	send_msg->reg6 = b;
	send_msg->reg4 = c;
	send(send_msg);
	recv_msg = recv(cons_channel);
	ret = recv_msg->reg1;
	recv_msg->msg_status = recv_ack;
	return ret;
}
