#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <asm/tsc.h>

#include "ipc.h"
#include "timing_util.h"


static struct ttd_ring_channel *prod;
static struct ttd_ring_channel *cons;


static unsigned long *time;

MODULE_LICENSE("GPL");

#define TRANSACTIONS 10000
#define _35_MILLION 35000000

static void  producer_iops(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	unsigned long start,end;
	struct ipc_message *msg;

	start = timing_util_RDTSC_START();
	while (count < _35_MILLION) {
		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->msg_id = 0x5555555555555555;
		send(chan,msg);
		msg = recv(chan);
		transaction_complete(msg);
		count++;
	}
	end = timing_util_RDTSCP();
	pr_err("35 million ping pongs in %lu cyles\n", end-start);
}

static void consumer_iops(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	struct ipc_message *msg;

	while (count < _35_MILLION) {
		msg = recv(chan);
		transaction_complete(msg);
		msg = get_send_slot(chan);
		msg->fn_type = 0x1C0DEBAD;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->msg_id = 0x5555555555555555;
		send(chan,msg);
		count++;
	}
}


static void producer(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	unsigned long start,end;
	struct ipc_message *msg;

	while (count < TRANSACTIONS) {
		start = timing_util_RDTSC_START();

		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->msg_id = 0x5555555555555555;
		send(chan,msg);
		msg = recv(chan);
		transaction_complete(msg);

		end = timing_util_RDTSCP();
		time[count] = end-start;
		count++;
	}
}

static void consumer(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	struct ipc_message *msg;

	while (count < TRANSACTIONS) {
		msg = recv(chan);
		transaction_complete(msg);
		msg = get_send_slot(chan);
		msg->fn_type = 0x1C0DEBAD;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->msg_id = 0x5555555555555555;
		send(chan,msg);
		count++;
	}
}


int dispatch(void *data)
{
	if(data == prod) {
		producer(data);
		timing_util_dump_time(time, (unsigned long)TRANSACTIONS);
		kfree(time);
	        producer_iops(data);
	}
	else {
		consumer(data);
		consumer_iops(data);
	}

	free_channel(data);
	return 1;
}


static void setup_tests(void)
{

	time = kzalloc(sizeof(unsigned long)*TRANSACTIONS, GFP_KERNEL);

	if(!time)
		return;

	pr_err("mem fine\n");
	prod = create_channel(4,0);
	if (!prod) {
		pr_err("Failed to create channel 1");
		return;
	}
	cons = create_channel(4,3);
	if (!cons) {
		pr_err("Failed to create channel 2");
		free_channel(prod);
		return;
	}
	connect_channels(prod,cons);
	ipc_start_thread(prod);
	ipc_start_thread(cons);
}

static int __init timing_init(void)
{
	int ret = 0;

	if (!this_cpu_has(X86_FEATURE_MWAIT))
		return -EPERM;

	setup_tests();

	return ret;
}
static int __exit timing_rmmod(void)
{

	return 0;
}

module_init(timing_init);
module_exit(timing_rmmod);
