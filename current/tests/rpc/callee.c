#include <linux/random.h>
#include <linux/slab.h>
#include "rpc.h"
#include <thc.h>
#include <thcinternal.h>
#include <awe-mapper.h>
 
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

static unsigned long null_invocation(void)
{
	struct ipc_message *msg;
	unsigned long result;

	msg = get_send_slot(channel);
	msg->fn_type = NULL_INVOCATION;
	send(channel,msg);
	msg = recv(channel);
	result = msg->reg1;
	transaction_complete(msg);
	return result;
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

static unsigned long add_nums_async(unsigned long trans, unsigned long res1, unsigned long msg_id)
{
	struct ipc_message *msg;
	unsigned long result;
	//printk(KERN_ERR "MESSAGE ID IS: %lx\n", msg_id);	
	msg = get_send_slot(channel);
	msg->fn_type = ADD_NUMS;
	msg->reg1    = trans;
	msg->reg2    = res1;
	msg->reg3    = res1;
	msg->reg4    = res1;
	msg->reg5    = res1;
    //msg->reg6    = res1;
	msg->msg_id  = msg_id;
	send(channel,msg);
	msg = async_recv(channel, msg_id);
	//printk(KERN_ERR "result = %lx\n", msg->reg1);
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
	msg->pts  = res5;
	send(channel,msg);
	msg = recv(channel);
	if (msg->fn_type != ADD_6_NUMS)
		pr_err("Response was not ADD_6_NUMS");

	result = msg->reg1;
	transaction_complete(msg);
	return result;
}



void test_async_ipc(void* chan)
{
	unsigned long num_transactions = 0;
	unsigned long res1, res2, res3, res4, res5, res6;
	unsigned long start,end;

	channel = chan;
	
	thc_init();

	res1 = 1;
	res2 = res1+res1;
	res3 = res1 + res2;
	res4 = res3 + res2;
	res5 = res4 + res3;
	DO_FINISH(
		uint32_t id_num;
		while (num_transactions < TRANSACTIONS / 3) {
		if((num_transactions * 3) % 300 == 0)
		{
			printk(KERN_ERR "num_transactions: %lu\n", num_transactions * 3);
		}
	//	start = RDTSC_START();

		ASYNC(
			id_num = awe_mapper_create_id();
//			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_nums_async(num_transactions, 1,(unsigned long) id_num);
		     );

		ASYNC(
			id_num = awe_mapper_create_id();
//			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_nums_async(num_transactions, 2,(unsigned long) id_num);
		     );

		ASYNC(
			id_num = awe_mapper_create_id();
//			printk(KERN_ERR "ID_NUM: %d\n", id_num);
			add_nums_async(num_transactions, 3,(unsigned long) id_num);
		     );


	//	end = RDTSCP();
	//	pr_err("%lu\n", end-start);
		num_transactions++;
	});
	pr_err("Complete\n");
	printk(KERN_ERR "lcd async exiting module and deleting ptstate");
	thc_done();
}

static void test_sync_ipc(void* chan)
{
	unsigned long num_transactions = 0;
	unsigned long res1, res2, res3, res4, res5, res6;
	unsigned long start,end;
	channel = chan;

	get_random_bytes(&res1, sizeof(res1));
	res2 = res1+res1;
	res3 = res1 + res2;
	res4 = res3 + res2;
	res5 = res4 + res3;

	while (num_transactions < TRANSACTIONS/2) {
		start = RDTSC_START();
		null_invocation();
		end = RDTSCP();
		pr_err("%lu\n", end-start);
		num_transactions++;
	}
	pr_err("6 regis\n");

	while (num_transactions < TRANSACTIONS) {
		start = RDTSC_START();
		res6 = add_6_nums(num_transactions,res1,res2,res3,res4,res5);
		end = RDTSCP();
		pr_err("%lu\n", end-start);
		num_transactions++;
	}
	pr_err("Complete\n");
}

int callee(void *chan)
{
	//NOTE: Setting the return address like below is a hack
	//if async is run inside a kernel thread, it is important to do this,
	//then restore it when async finishes.
	volatile void ** frame = (volatile void**)__builtin_frame_address(0);
	volatile void *ret_addr = *(frame + 1);

	*(frame + 1) = NULL;

	test_async_ipc(chan);

	*(frame + 1) = ret_addr;

	return 1;
}
