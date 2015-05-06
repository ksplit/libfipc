#include <../../IPC.h>
#include "rpc.h"

static ttd_ring_channel *channel;

static unsigned long add_constant(unsigned long trans)
{
	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = ADD_CONSTANT;
	msg->reg1 = trans;
	send_message(msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_CONSTANT)
		pr_err("Response was not ADD_CONSTANT");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}

static unsigned long add_nums(unsigned long trans, unsigned long res1)
{
	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = ADD_NUMS;
	msg->reg1 = trans;
	msg->reg2 = res1;
	send_message(msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_NUMS)
		pr_err("Response was not ADD_NUMS");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}

static unsigned long add_3_nums(unsigned long trans, unsigned long res1,
				unsigned long res2)
{

	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = ADD_3_NUMS;
	msg->reg1 = trans;
	msg->reg2 = res1;
	msg->reg3 = res2;
	send_message(msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_3_NUMS)
		pr_err("Response was not ADD_3_NUMS");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}
static unsigned long add_4_nums(unsigned long trans, unsigned long res1,
				unsigned long res2, unsigned long res3)
{

	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = ADD_4_NUMS;
	msg->reg1 = trans;
	msg->reg2 = res1;
	msg->reg3 = res2;
	msg->reg4 = res3;
	send_message(msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_4_NUMS)
		pr_err("Response was not ADD_4_NUMS");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}

static unsigned long add_5_nums(unsigned long trans, unsigned long res1,
				unsigned long res2, unsigned long res3,
				unsigned long res4)
{
	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = ADD_4_NUMS;
	msg->reg1 = trans;
	msg->reg2 = res1;
	msg->reg3 = res2;
	msg->reg4 = res3;
	msg->reg5 = res4;
	send_message(msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_5_NUMS)
		pr_err("Response was not ADD_5_NUMS");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}


static unsigned long add_6_nums(unsigned long trans, unsigned long res1,
				unsigned long res2, unsigned long res3,
				unsigned long res4, unsigned long res5)
{
	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = ADD_4_NUMS;
	msg->reg1 = trans;
	msg->reg2 = res1;
	msg->reg3 = res2;
	msg->reg4 = res3;
	msg->reg5 = res4;
	msg->reg6 = res5;
	send_message(msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_5_NUMS)
		pr_err("Response was not ADD_5_NUMS");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}

void callee(struct ttd_ring_channel *chan)
{
	unsigned long num_transactions = 0;
	unsigned long res1, res2, res3, res4, res5, res6;
	channel = chan;
	while (num_transactions < TRANSACTIONS) {
		res1 = add_constant(num_transactions);
		res2 = add_nums(num_transactions,res1);
		res3 = add_3_nums(num_transactions,res1,res2);
		res4 = add_4_nums(num_transactions,res1,res2,res3);
		res5 = add_5_nums(num_transactions,res1,res2,res3,res4);
		res6 = add_6_nums(num_transactions,res1,res2,res3,res4,res5);
		num_transactions += 6;
		pr_err("res6 is %lu\n",res6);
	}
}
