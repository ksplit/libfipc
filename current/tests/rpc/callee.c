#include "rpc.h"

static struct ttd_ring_channel *channel;

static unsigned long RDTSC_START(void)
{

	unsigned cycles_low, cycles_high;

	asm volatile ("CPUID\n\t"
		      "RDTSC\n\t"
		      "mov %%edx, %0\n\t"
		      "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
		      "%rax", "%rbx", "%rcx", "%rdx");
	return ((unsigned long) cycles_high << 32) | cycles_low;

}


static unsigned long RDTSCP(void)
{
	unsigned long tsc;
	__asm__ __volatile__(
        "rdtscp;"
        "shl $32, %%rdx;"
        "or %%rdx, %%rax"
        : "=a"(tsc)
        :
        : "%rcx", "%rdx");

	return tsc;
}



static unsigned long add_constant(unsigned long trans)
{
	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = ADD_CONSTANT;
	msg->reg1 = trans;
	send(channel,msg);
	msg = recv(channel);
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
	send(channel,msg);
	msg = recv(channel);
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
	send(channel,msg);
	msg = recv(channel);
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
	send(channel,msg);
	msg = recv(channel);
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
	msg->fn_type = ADD_5_NUMS;
	msg->reg1 = trans;
	msg->reg2 = res1;
	msg->reg3 = res2;
	msg->reg4 = res3;
	msg->reg5 = res4;
	send(channel,msg);
	msg = recv(channel);
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
	msg->fn_type = ADD_6_NUMS;
	msg->reg1 = trans;
	msg->reg2 = res1;
	msg->reg3 = res2;
	msg->reg4 = res3;
	msg->reg5 = res4;
	msg->reg6 = res5;
	send(channel,msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_6_NUMS)
		pr_err("Response was not ADD_6_NUMS");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}

void callee(struct ttd_ring_channel *chan)
{
	unsigned long num_transactions = 0;
	unsigned long res1, res2, res3, res4, res5, res6;
	unsigned long start,end;
	channel = chan;
	while (num_transactions < TRANSACTIONS) {
		start = RDTSC_START();
		res1 = add_constant(num_transactions);
		res2 = add_nums(num_transactions,res1);
		res3 = add_3_nums(num_transactions,res1,res2);
		res4 = add_4_nums(num_transactions,res1,res2,res3);
		res5 = add_5_nums(num_transactions,res1,res2,res3,res4);
		res6 = add_6_nums(num_transactions,res1,res2,res3,res4,res5);
		end = RDTSCP();
		pr_err("%lu\n", end-start);
		num_transactions += 6;
		//pr_err("res6 is %lu\n on iteration, %lu",res6, num_transactions);
	}
	pr_err("Complete\n");
}