#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/irqflags.h>
#include <linux/kthread.h>
#include <linux/cpumask.h>
#include <linux/preempt.h>
#include <asm/uaccess.h>
#include <asm/mwait.h>
#include <asm/page_types.h>
#include <asm/cpufeature.h>
#include <linux/ktime.h>
#include <linux/sort.h>
#include <asm/tsc.h>



#include "ring-chan/ring-channel.h"
#include "betaModule.h"

MODULE_LICENSE("GPL");

static int CPU_NUM;


static	unsigned int pTok = 0xC1346BAD;
static	unsigned int cTok = 0xBADBEEF;


/* 124 byte message */
#if 0
static char *msg = "12345678123456781234567812345678123456781234567812345678" \
	"1234567";
#endif

static unsigned long start;
static unsigned long end;

static volatile int should_stop;

#if defined(TIMING)
static u64 *timekeeper;
#endif


#if 0
/* Stolen and slightly modified from http://rosettacode.org/wiki/Rot-13 */
static char *rot13(char *s, int amount)
{
	char *p = s;
	int upper;
	int count = 0;

	while (*p && count < amount) {
		upper = *p;
		if ((upper >= 'a' && upper <= 'm') ||
		    (upper >= 'A' && upper <= 'M'))
			*p += 13;
		else if ((upper >= 'n' && upper <= 'z') ||
			 (upper >= 'A' && upper <= 'Z'))
			*p -= 13;
		++p;
		count++;
	}
	return s;
}
#endif


#if defined(DEBUG_ASSERT_EXPECT)
static void assert_expect_and_zero(struct ipc_message *i_msg, int need_rot)
{
	if (need_rot)
		rot13(i_msg->message, BUF_SIZE);

	if (strncmp(i_msg->message, msg, BUF_SIZE) != 0)
		pr_err("STRINGS DIFFERED IN CPU %d\n", CPU_NUM);

	i_msg->monitor = 0;
}
#endif


#if defined(USE_MWAIT)
static unsigned int find_target_mwait(void)
{
        unsigned int eax, ebx, ecx, edx;
        unsigned int highest_cstate = 0;
        unsigned int highest_subcstate = 0;
        int i;

        if (boot_cpu_data.cpuid_level < CPUID_MWAIT_LEAF)
                return 0;

        cpuid(CPUID_MWAIT_LEAF, &eax, &ebx, &ecx, &edx);

        if (!(ecx & CPUID5_ECX_EXTENSIONS_SUPPORTED) ||
            !(ecx & CPUID5_ECX_INTERRUPT_BREAK))
                return 0;

        edx >>= MWAIT_SUBSTATE_SIZE;
        for (i = 0; i < 7 && edx; i++, edx >>= MWAIT_SUBSTATE_SIZE) {
                if (edx & MWAIT_SUBSTATE_MASK) {
                        highest_cstate = i;
                        highest_subcstate = edx & MWAIT_SUBSTATE_MASK;
                        printk(KERN_DEBUG "Found cstate at %d and highest_subcstate %d\n",
                               i, highest_subcstate);
                        printk(KERN_DEBUG "IF WE WERE TO RETURN NOW IT WOUDL LOOK LIKE %x\n", (highest_cstate << MWAIT_SUBSTATE_SIZE) | (highest_subcstate -1));
                }
        }
        return (highest_cstate << MWAIT_SUBSTATE_SIZE) |
                (highest_subcstate - 1);

}


static inline void monitor_mwait(unsigned long rcx, volatile uint32_t *rax,
				 unsigned long wait_type)
{

	__monitor((void *)rax, 0, 0);
	/* TODO comment for memory barrier, why is this necessary? */
	mb();
	__mwait(wait_type, rcx);
}
#endif

	//unsigned long flags;
	//int cpu;
	/* smp is supposed to be used under "lock", however one can use it if
	 * you have pegged your thread to a CPU, which we have.
	 */
	/* we know we're noot ona buggy cpu when we release we'll re-enable this */
	/*cpu = smp_processor_id();

	  if (cpu_has_bug(&cpu_data(cpu), X86_BUG_CLFLUSH_MONITOR)) {
	  mb();
	  clflush(rax);
	  mb();
	  }*/

	//	local_irq_save(flags);
	//	local_irq_restore(flags);



static inline int check_cons_slot_available(struct ipc_message *loc, unsigned int token)
{
	return (likely(loc->monitor != token));
}

static inline int check_prod_slot_available(struct ipc_message *loc, unsigned int token)
{
	return (unlikely(loc->monitor != token));
}


static struct ipc_message * get_next_available_slot(struct ttd_ring_channel *chan,
					     unsigned long bucket)
{
	return (struct ipc_message *) ttd_ring_channel_get_rec_slow(chan,bucket);
}


static int wait_for_producer_slot(struct ipc_message *imsg, unsigned int token)
{


	while (check_prod_slot_available(imsg, token)) {

#if defined(USE_MWAIT)
		monitor_mwait(ecx, &imsg->monitor, cstate_wait);
#endif//usemwait
#if defined(POLL)
		cpu_relax();
		//	asm volatile("pause" ::: "memory");
			//__builting_ia32_pause();
#endif
	}
	return 0;
}

static  int wait_for_consumer_slot(struct ipc_message *imsg, unsigned int token)
{


	while (check_cons_slot_available(imsg, token)) {

#if defined(USE_MWAIT)
		monitor_mwait(ecx, &imsg->monitor, cstate_wait);
#endif//usemwait
#if defined(POLL)
		cpu_relax();
		//asm volatile("pause" ::: "memory");
#endif
	}
	return 0;
}


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


static int ipc_thread_func(void *input)
{

#if defined(USE_FLOOD)
	int i;
#endif
 	struct file *filep = input;
	struct ipc_container *container = NULL;
	struct ttd_ring_channel *prod_channel;
	struct ttd_ring_channel *cons_channel;
	unsigned long flags, count = 0;
	unsigned int local_prod, local_cons;
	struct ipc_message *prod_msg, *cons_msg;
	unsigned long  start64, end64;
	if (filep == NULL) {
		pr_debug("Thread was sent a null filepointer!\n");
		return -EINVAL;
	}

	container = filep->private_data;

	if (container == NULL || container->channel_tx == NULL
	    || container->channel_rx == NULL) {
		pr_debug("container was null in thread!\n");
		return -EINVAL;
	}


	prod_channel = container->channel_tx;
	cons_channel = container->channel_rx;
	local_prod = 1;
	local_cons = 1;
	ttd_ring_channel_set_prod(prod_channel, 1);
	ttd_ring_channel_set_cons(prod_channel, 0);

	prod_msg = get_next_available_slot(prod_channel, local_prod);
	cons_msg = get_next_available_slot(cons_channel, local_cons);

	local_irq_save(flags);
	preempt_disable();

	while (count < NUM_LOOPS) {
		start64 = RDTSC_START();

#if defined(USE_FLOOD)
		for (i = 0; i < FLOOD_SIZE; i++) {
#endif

			//ptok = 0xC1346BAD;
			//__builtin_prefetch(cons_msg, 1, 1);
			//wait_for_producer_slot(prod_msg, pTok);
			while(prod_msg->monitor != pTok)
				cpu_relax();


			//imsg->message[0] = 'b';
			//imsg->message[1] = 'e';
			//imsg->message[2] = 't';
			//imsg->message[3] = '1';

			prod_msg->monitor = cTok;
			//local_prod++;
#if defined(USE_FLOOD)
		        prod_msg = get_next_available_slot(prod_channel, local_prod);
		}

	       for (i = 0; i < FLOOD_SIZE; i++) {
#endif

		       //wait_for_consumer_slot(cons_msg, cTok);
		       while(cons_msg->monitor != cTok)
			       cpu_relax();

			/* ack the msg */
			//cons_msg->monitor = pTok;
			//			local_cons++;
#if defined(USE_FLOOD)
  		        cons_msg = get_next_available_slot(cons_channel, local_cons);
	      }
#endif

	       end64 = RDTSCP();
#if !defined(USE_FLOOD)
	       //prod_msg = get_next_available_slot(prod_channel, local_prod);
	       //cons_msg = get_next_available_slot(cons_channel, local_cons);
	       prod_msg++;
	       cons_msg++;
#endif

		count++;



#if defined(TIMING)
		timekeeper[count] = (end64 - start64);
#endif

	}
	preempt_enable();
	local_irq_restore(flags);

	return 1;
}

static inline unsigned long beta_ret_cpu(unsigned long __arg)
{

	unsigned long __user *arg = (void *) __arg;

	return put_user(CPU_NUM, arg);
}


static unsigned long beta_unpark_thread(struct ipc_container *container)
{
	if (container->thread == NULL)
		return -EINVAL;

	pr_debug("waking up process on CPU %d\n", CPU_NUM);
	//kthread_unpark(container->thread);
	if (wake_up_process(container->thread) == 1)
		pr_debug("Woke up process on cpu %d\n", CPU_NUM);

	return 0;
}


static unsigned long beta_connect_mem(struct ipc_container *container,
				      unsigned long __arg)
{

	unsigned long  __user *ubuf = (void *) __arg;
	unsigned long kland_real;
	unsigned long *kland;

	if (get_user(kland_real, ubuf)) {
		pr_debug("get_user failed connect_mem with ptr %p\n", ubuf);
		return -EFAULT;
	}

	kland = (unsigned long *) kland_real;

	if (kland == NULL) {
		pr_err("returning efault in beta_connect_mem\n");
		return -EFAULT;
	}

	/* todo talk about this bootstrap issue while we're beta testing */
	/* perhaps, we can use extern and export syms? */
	container->channel_rx = (struct ttd_ring_channel *) kland;
	return 0;
}

static unsigned long beta_alloc_mem(struct ipc_container *container)
{
	int ret,i;
	if (container->channel_tx == NULL)
		return -EINVAL;

	ret = ttd_ring_channel_alloc(container->channel_tx,
				     CHAN_NUM_PAGES,
				     sizeof(struct ipc_message));

	if (ret != 0) {
		pr_err("Failed to alloc/Init ring channel\n");
		return -ENOMEM;
	}
	pr_debug("Channel is at %p, recs are %p to %p\n", (void*)container->channel_tx,
		 container->channel_tx->recs,
		 container->channel_tx->recs + (CHAN_NUM_PAGES * PAGE_SIZE));
	start = (unsigned long) container->channel_tx->recs;
	end = (unsigned long) container->channel_tx->recs + (CHAN_NUM_PAGES * PAGE_SIZE);

	for(i = 0; i < (CHAN_NUM_PAGES * PAGE_SIZE)/sizeof(int); i++)
		*((int *)container->channel_tx->recs+i) = 0xC1346BAD;
	//memset(container->channel_tx->recs, 0xFF, (CHAN_NUM_PAGES * PAGE_SIZE));

	return 0;
}

static int beta_open(struct inode *nodp, struct file *filep)
{

	struct ipc_container *container;
	struct cpumask cpu_core;
	should_stop = 0;
#if defined(TIMING)
	timekeeper = kzalloc(sizeof(u64) * NUM_LOOPS, GFP_KERNEL);
	if(!timekeeper) {
		pr_err("could not alloc space for time keeping");
		return -ENOMEM;
	}
#endif
	container = kzalloc(sizeof(*container), GFP_KERNEL);

	if (!container) {
		pr_err("Could not alloc space for container\n");
		return -ENOMEM;
	}

	container->channel_tx = kzalloc(sizeof(*container->channel_tx),
					GFP_KERNEL);

	if (!container->channel_tx) {
		pr_err("Could not alloc space for ring channel\n");
		return -ENOMEM;
	}

	container->thread = kthread_create(&ipc_thread_func, (void *)filep,
					   "betaIPC.%u", CPU_NUM);


	/*container->thread = kthread_create_on_cpu(&ipc_thread_func,
	  (void *)filep, CPU_NUM ,"betaIPC.%u");*/

	if (IS_ERR(container->thread)) {
		pr_err("Error while creating kernel thread\n");
		return PTR_ERR(container->thread);
	}


	get_task_struct(container->thread);

	cpumask_clear(&cpu_core);
	cpumask_set_cpu(CPU_NUM,&cpu_core);

	set_cpus_allowed_ptr(container->thread, &cpu_core);

	filep->private_data = container;

	return 0;
}

static int beta_close(struct inode *nodp, struct file *filep)
{

	struct ipc_container *container;
	should_stop = 1;
	container = filep->private_data;

	put_task_struct(container->thread);

	if (container->channel_tx)
		ttd_ring_channel_free(container->channel_tx);

	kfree(container);
#if defined(TIMING)
	kfree(timekeeper);
#endif

	return 0;
}

static long beta_return_mem(struct ipc_container *container,
			    unsigned long __arg)
{
	unsigned long __user  *save = (unsigned long *) __arg;

	return put_user((unsigned long)container->channel_tx, save);
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
        u64 min;
	u64 max;
	if (timekeeper == NULL) {
		pr_err("Time keeper was null, ret");
		return;
	}
	for (i = 1; i < NUM_LOOPS; i++) {
		pr_err("%llu\n", timekeeper[i]);
		counter+= timekeeper[i];
	}
	
	sort(timekeeper, NUM_LOOPS, sizeof(u64), compare, NULL);

	min = timekeeper[1];
	max = timekeeper[9999];
	counter = min;
	for (i = 1; i < NUM_LOOPS; i++) {
		//pr_err("%llu\n", timekeeper[i]);
		counter+= timekeeper[i];
	}
	pr_err("MIN\tMAX\tAVG\tMEDIAN\n");
	pr_err("%llu & %llu & %llu & %llu\n", min, max, counter/NUM_LOOPS, timekeeper[4999]);

}

static long beta_ioctl(struct file *filep, unsigned int cmd,
		       unsigned long __arg)
{
	struct ipc_container *container = filep->private_data;
	long ret = 0;

	switch (cmd) {
	case BETA_ALLOC_MEM:
		ret = beta_alloc_mem(container);
		break;
	case BETA_GET_CPU_AFFINITY:
		ret = beta_ret_cpu(__arg);
		break;
	case BETA_CONNECT_SHARED_MEM:
		ret = beta_connect_mem(container, __arg);
		break;
	case BETA_UNPARK_THREAD:
		ret = beta_unpark_thread(container);
		break;
	case BETA_GET_MEM:
		ret = beta_return_mem(container, __arg);
		break;
	case BETA_DUMP_TIME:
		dump_time();
		break;
	default:
		pr_debug("No such ioctl %d\n", cmd);
		break;
	}
	return ret;
}

static const struct file_operations betaIPC_fops = {
	.owner	 = THIS_MODULE,
	.open    = beta_open,
	.release  = beta_close,
	.unlocked_ioctl   = beta_ioctl,
};

static struct miscdevice dev = {
	MISC_DYNAMIC_MINOR,
	"betaIPC",
	&betaIPC_fops,
};



static int __init bIPC_init(void)
{
	int ret = 0;

	CPU_NUM = 1;
	if (this_cpu_has(X86_FEATURE_MWAIT))
		printk(KERN_DEBUG "HAS MWAIT\n");

	/* reading through the source of misc.c it looks like register
	   will init everything else for us */
	pr_debug("hello from bIPC with pr_debug\n");
	printk(KERN_DEBUG "Hello from bIPC with printk\n");
	ret = misc_register(&dev);
	if (ret) {
		pr_debug("Failed to register dev for BetaIPC\n");
		return ret;
	}

	return ret;
}
static int __exit bIPC_rmmod(void)
{
	int ret = 0;

	ret = misc_deregister(&dev);
	if (ret) {
		pr_debug("Failed to de-reg dev in eudy!\n");
		return ret;
	}

	return 0;
}

module_init(bIPC_init);
module_exit(bIPC_rmmod);
