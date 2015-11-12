#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sort.h>
#include <asm/tsc.h>

#include "ipc.h"

static struct ttd_ring_channel *prod;
static struct ttd_ring_channel *cons;


static unsigned long *time;

MODULE_LICENSE("GPL");

#define TRANSACTIONS 10000
#define _35_MILLION 35000000

//void prefetch_tx(struct ttd_ring_channel *rx);
//void prefetch_rx(struct ttd_ring_channel *rx);

/* Timing is used via serializing instructios because this white paper says so:
 *
 *  http://www.intel.com/content/www/us/en/intelligent-systems/embedded-systems-training/ia-32-ia-64-benchmark-code-execution-paper.html
 */
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


static void  producer_iops(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	unsigned long start,end;
	struct ipc_message *msg;

	start = RDTSC_START();
	while (count < _35_MILLION) {
		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);
		msg = recv(chan);
		transaction_complete(msg);
		count++;
	}
	end = RDTSCP();
	pr_err("35 million ping pongs in %lu cyles\n", end-start);
}

static void consumer_iops(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	struct ipc_message *msg;
	struct ipc_message *sen;

	while (count < _35_MILLION) {
		msg = recv(chan);
		transaction_complete(msg);
		sen = get_send_slot(chan);
		sen->fn_type = 0x1C0DEBAD;
		sen->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		sen->reg2 = 0x0;
		sen->reg3 = 0x9999999999999999;
		sen->reg4 = 0x8888888888888888;
		sen->reg5 = 0x7777777777777777;
		sen->reg6 = 0x6666666666666666;
		sen->reg7 = 0x5555555555555555;
		send(chan,sen);
		count++;
	}
}


static void producer(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	unsigned long start,end;
	struct ipc_message *msg;

	while (count < TRANSACTIONS) {
		start = RDTSC_START();
		msg = get_send_slot(chan);
		prefetch_tx(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);
		/*		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);
		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);
		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg); 
		

		msg = get_send_slot(chan);
		//		prefetch_tx(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);
		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);
		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);
		msg = get_send_slot(chan);
		msg->fn_type = 0x31337;
		msg->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		msg->reg2 = 0x0;
		msg->reg3 = 0x9999999999999999;
		msg->reg4 = 0x8888888888888888;
		msg->reg5 = 0x7777777777777777;
		msg->reg6 = 0x6666666666666666;
		msg->reg7 = 0x5555555555555555;
		send(chan,msg);*/
		/* 8 */

		/*		msg = recv(chan);
		transaction_complete(msg);
		msg = recv(chan);
		transaction_complete(msg);
		msg = recv(chan);
		transaction_complete(msg);
		msg = recv(chan);
		transaction_complete(msg);
		msg = recv(chan);
		transaction_complete(msg);
		msg = recv(chan);
		transaction_complete(msg);
		msg = recv(chan);
		transaction_complete(msg);*/
		msg = recv(chan);
		transaction_complete(msg);


		end = RDTSCP();
		time[count] = end-start;
		count++;
	}
}

static void consumer(struct ttd_ring_channel *chan)
{
	unsigned long count = 0;
	struct ipc_message *msg;
	struct ipc_message *sen;

	while (count < TRANSACTIONS) {
		msg = recv(chan);
		transaction_complete(msg);
		sen = get_send_slot(chan);
		prefetch_tx(chan);
		sen->fn_type = 0x1C0DEBAD;
		sen->reg1 = 0xAAAAAAAAAAAAAAAA; //414141
		sen->reg2 = 0x0;
		sen->reg3 = 0x9999999999999999;
		sen->reg4 = 0x8888888888888888;
		sen->reg5 = 0x7777777777777777;
		sen->reg6 = 0x6666666666666666;
		sen->reg7 = 0x5555555555555555;
		send(chan, sen);
		count++;
	}
}

static int compare(const void *_a, const void *_b){

	u64 a = *((u64 *)_a);
	u64 b = *((u64 *)_b);

	if(a < b)
		return -1;
	if(a > b)
		return 1;
	return 0;
}


static void dump_time(void)
{

	int i;
	unsigned long long counter = 0;
        unsigned long min;
	unsigned long max;

	for (i = 0; i < TRANSACTIONS; i++) {
		counter+= time[i];
	}

	sort(time, TRANSACTIONS, sizeof(unsigned long), compare, NULL);

	min = time[0];
	max = time[(TRANSACTIONS) - 1];
	counter = min;

	pr_err("MIN\tMAX\tAVG\tMEDIAN\n");
	pr_err("%lu & %lu & %llu & %lu \n", min, max, counter/TRANSACTIONS, time[((TRANSACTIONS))/2]);

}

int dispatch(void *data)
{
	if(data == prod) {
		producer(data);
		dump_time();
		kfree(time);
		//	        producer_iops(data);
	}
	else {
		consumer(data);
		//consumer_iops(data);
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
	prod = create_channel(4);
	if (!prod) {
		pr_err("Failed to create channel 1");
		return;
	}
	cons = create_channel(4);
	if (!cons) {
		pr_err("Failed to create channel 2");
		free_channel(prod);
		return;
	}
	connect_channels(prod,cons);

        if (attach_thread_to_channel(prod, 28, dispatch) == NULL ||
            attach_thread_to_channel(cons, 31, dispatch) == NULL ) {
                ttd_ring_channel_free(prod);
                ttd_ring_channel_free(cons);
                kfree(prod);
                kfree(cons);
                return;
        }


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
