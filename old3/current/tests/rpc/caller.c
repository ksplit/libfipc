#include "rpc.h"


/* don't let gcc opt this into a  mov 9 memaddr */
static unsigned long noinline null_invocation(void)
{
	return 9;
}

static unsigned long add_constant(unsigned long trans)
{
	return trans + 50;
}

static unsigned long add_nums(unsigned long trans, unsigned long res1)
{
	return trans+res1;
}

static unsigned long add_3_nums(unsigned long trans, unsigned long res1,
				unsigned long res2)
{
	return add_nums(trans, res1) + res2;
}

static unsigned long add_4_nums(unsigned long trans, unsigned long res1,
				unsigned long res2, unsigned long res3)
{

	return add_nums(trans, res1) + add_nums(res2, res3);
}

static unsigned long add_5_nums(unsigned long trans, unsigned long res1,
				unsigned long res2, unsigned long res3,
				unsigned long res4)
{
	return add_4_nums(trans,res1,res2,res3) + res4;
}

static unsigned long add_6_nums(unsigned long trans, unsigned long res1,
				unsigned long res2, unsigned long res3,
				unsigned long res4, unsigned long res5)
{
	return add_3_nums(trans,res1,res2) + add_3_nums(res3,res4,res5);
}

int caller(void *channel)
{

	unsigned long num_transactions = 0;
	unsigned long temp_res = 0;
        struct ttd_ring_channel *chan = channel;
	struct ipc_message *msg;
	while (num_transactions < TRANSACTIONS) {
		msg = recv(chan);

		switch(msg->fn_type) {
		case NULL_INVOCATION:
			temp_res = null_invocation();
			transaction_complete(msg);
			msg = get_send_slot(chan);
			msg->fn_type = NULL_INVOCATION;
			msg->reg1 = temp_res;
			send(chan, msg);
			break;
		case ADD_CONSTANT:
			temp_res = add_constant(msg->reg1);
			transaction_complete(msg);
			msg = get_send_slot(chan);
			msg->fn_type = ADD_CONSTANT;
			msg->reg1 = temp_res;
			send(chan, msg);
			break;
		case ADD_NUMS:
			temp_res = add_nums(msg->reg1, msg->reg2);
			transaction_complete(msg);
			msg = get_send_slot(chan);
			msg->fn_type = ADD_NUMS;
			msg->reg1 = temp_res;
			send(chan, msg);
			break;
		case ADD_3_NUMS:
			temp_res = add_3_nums(msg->reg1, msg->reg2, msg->reg3);
			transaction_complete(msg);
			msg = get_send_slot(chan);
			msg->fn_type = ADD_3_NUMS;
			msg->reg1 = temp_res;
			send(chan, msg);
			break;
		case ADD_4_NUMS:
			temp_res = add_4_nums(msg->reg1, msg->reg2, msg->reg3,
					      msg->reg4);
			transaction_complete(msg);
			msg = get_send_slot(chan);
			msg->fn_type = ADD_4_NUMS;
			msg->reg1 = temp_res;
			send(chan, msg);
			break;
		case ADD_5_NUMS:
			temp_res = add_5_nums(msg->reg1, msg->reg2, msg->reg3,
					      msg->reg4, msg->reg5);
			transaction_complete(msg);
			msg = get_send_slot(chan);
			msg->fn_type = ADD_5_NUMS;
			msg->reg1 = temp_res;
			send(chan, msg);
			break;
		case ADD_6_NUMS:
			temp_res = add_6_nums(msg->reg1, msg->reg2, msg->reg3,
					      msg->reg4, msg->reg5, msg->reg6);
			transaction_complete(msg);
			msg = get_send_slot(chan);
			msg->fn_type = ADD_6_NUMS;
			msg->reg1 = temp_res;
			send(chan, msg);
			break;
		}
		num_transactions++;
	}
        return 1;
}
