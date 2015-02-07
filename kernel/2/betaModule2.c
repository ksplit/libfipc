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
#include <asm/uaccess.h>
#include <asm/mwait.h>
#include <asm/page_types.h>
#include <asm/cpufeature.h>
#include <linux/ktime.h>

#include "ring-chan/ring-channel.h"
#include "../betaModule.h"

MODULE_LICENSE("GPL");

static int CPU_NUM;


/* 124 byte message */
static char *msg = "12345678123456781234567812345678123456781234567812345678" \
	"1234567";

static unsigned long start;
static unsigned long end;


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

static void assert_expect_and_zero(struct ipc_message *i_msg, int need_rot)
{
	if (need_rot)
		rot13(i_msg->message, 60);

	if (strncmp(i_msg->message, msg, BUF_SIZE) != 0)
		pr_debug("STRINGS DIFFERED IN CPU %d\n", CPU_NUM);

	//	memset(i_msg->message, 0, 60);
	i_msg->monitor = 0xC1346BAD;
}

static inline void monitor_mwait(unsigned long rcx, volatile uint32_t *rax,
				 unsigned long wait_type)
{

	unsigned long flags;
	int cpu;

	/* TODO Figure out wtf the "extensions" and "hints" do for monitor */
	wbinvd();

	/* smp is supposed to be used under "lock", however one can use it if
	 * you have pegged your thread to a CPU, which we have.
	 */
	cpu = smp_processor_id();

	if (cpu_has_bug(&cpu_data(cpu), X86_BUG_CLFLUSH_MONITOR)) {
		mb();
		clflush(rax);
		mb();
	}

	//	local_irq_save(flags);
	__monitor((void *)rax, 0, 0);
	/* TODO comment for memory barrier, why is this necessary? */
	mb();
	__mwait(wait_type, rcx);
	//local_irq_restore(flags);
}


static inline int trample_imminent(struct ipc_message *loc)
{
	return (loc->monitor != 0xbadbeef);
}

static inline int trample_imminent_store(struct ttd_ring_channel *prod,
			    unsigned int prod_loc, struct ipc_message  **t_loc)
{
	struct ipc_message *imsg;
	imsg = (struct ipc_message *) ttd_ring_channel_get_rec_slow(prod, prod_loc);
	*t_loc = imsg;
	return (imsg->monitor != 0xbadbeef);
}


static void write_protected_slot(struct ttd_ring_channel *cons,
unsigned long location)
{

	unsigned long *write_loc;
	write_loc = (unsigned long*) ttd_ring_channel_get_rec_slow(cons, location);
	*write_loc = 0;
}


static int ipc_thread_func(void *input)
{
	/* This will be a while true loop with timing code and mwaits
	   CPU 1 WILL be the first to send info. It will ROT 13 some
	   message and inc on some pointer at 128 bytes.

	   CPU 0 will come in here and will monitor/mwait on the 128
	   byte boundary until it gets out of its OPT loop
	*/

	pr_debug("Hello from betaModule2\n");

 	struct file *filep = input;
	struct ipc_container *container = NULL;
	unsigned long ecx = 1; /*break of interrupt flag */
	unsigned long cstate_wait = 2;
	struct ttd_ring_channel *cons_channel;
	int count = 0;
	unsigned int local_cons;
	unsigned int *trample_loc;
	struct ipc_message *imsg;
	ktime_t start, end;

#if defined(DEBUG_MWAIT_RETRY)
	unsigned long retry_count = 0;
#endif

	if (filep == NULL) {
		pr_debug("Thread was sent a null filepointer!\n");
		return -EINVAL;
	}

	container = filep->private_data;

	if (container == NULL && container->channel_rx == NULL) {
		pr_debug("container was null in thread!\n");
		return -EINVAL;
	}

	cons_channel = container->channel_rx;

	local_cons = 1;
	/* 10 mil */
	start = ktime_get();
	while(count < 5000000) {

		/* POSSIBLE CACHE THRASHING WILE WAITING ??*/
		/* TODO LETS DO BOTH METHODS, LETS MWAIT ON THE NIL */
		/* AND LETS MWAIT ONT THE CONSUMER */
		if(trample_imminent_store(cons_channel, local_cons, &imsg)) {
			do{
				pr_debug("Waiting in CPU%d on %p\n",
					 CPU_NUM, &imsg->monitor);
				monitor_mwait(ecx, &imsg->monitor, cstate_wait);

#if defined(DEBUG_MWAIT_RETRY)
				if(retry_count > 50) {
					pr_err("RETRY COUNT FAILED! MORE THAN "\
						 "50 WAITS on CPU %d with count %d\n",
					       CPU_NUM, count);
					return -1;
				}
				retry_count++;
#endif
			}while(trample_imminent(imsg));
		}

#if defined(DEBUG_BOUNDS_CHECK)
		/* trample Location is now free for us to write */
		if((unsigned long)imsg > end || (unsigned long)imsg < start) {
			pr_err("OUT OF BOUNDS! with %p on CPU %d\n", imsg,
			       CPU_NUM);
			break;
		}
#endif
		pr_debug("assert_expecting on count %d at loc %p\n",
			 count, imsg->message);
		assert_expect_and_zero(imsg, 0);
		pr_debug("Wrote to voltile var in cpu %d at loc %p\n",
			 CPU_NUM, &imsg->monitor);

		//ttd_ring_channel_inc_cons(cons_channel);

#if defined(DEBUG_MWAIT_RETRY)
		retry_count = 0;
#endif

		local_cons++;
		count++;
	}
	end = ktime_get();
	printk(KERN_DEBUG "Time taken for function() execution: %llu on cpu %d\n",
	       ktime_to_ns(ktime_sub(end, start)), CPU_NUM);
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

	/* FROM THIS POINT FORWARD, ATLEAST ONE OF THE THREADS
	 * IS SITTING IN THE COMM CODE
	 */

	pr_debug("waking up process on CPU %d\n", CPU_NUM);
	kthread_unpark(container->thread);
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

	if (kland == NULL)
		return -EFAULT;


	/* todo talk about this bootstrap issue while we're beta testing */
	/* perhaps, we can use extern and export syms? */
	container->channel_rx = (struct ttd_ring_channel *) kland;
	start = (unsigned long) container->channel_rx->recs;
	end = (unsigned long) container->channel_rx->recs + (CHAN_NUM_PAGES * PAGE_SIZE);
	return 0;
}

static unsigned long beta_alloc_mem(struct ipc_container *container)
{
	int ret;
	if (container->channel_tx == NULL)
		return -EINVAL;

	ret = ttd_ring_channel_alloc(container->channel_tx,
				     CHAN_NUM_PAGES,
				     sizeof(struct ipc_message));

	if (ret != 0) {
		pr_err("Failed to alloc/Init ring channel\n");
		return -ENOMEM;
	}

	return 0;
}

static int beta_open(struct inode *nodp, struct file *filep)
{

	struct ipc_container *container;

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


	container->thread = kthread_create_on_cpu(&ipc_thread_func,
						  (void *)filep, CPU_NUM,
						  "betaIPC.%u");

	if (IS_ERR(container->thread)) {
		pr_err("Error while creating kernel thread\n");
		return PTR_ERR(container->thread);
	}

	filep->private_data = container;
	return 0;
}

static int beta_close(struct inode *nodp, struct file *filep)
{

	struct ipc_container *container;

	container = filep->private_data;
	kfree(container);


	/*	if (container->channel_tx)
		ttd_ring_channel_free(container->channel_tx);*/


	return 0;
}

static long beta_return_mem(struct ipc_container *container,
			    unsigned long __arg)
{
	unsigned long __user  *save = (unsigned long *) __arg;

	return put_user((unsigned long)container->channel_tx, save);
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
	"betaIPC2",
	&betaIPC_fops,
};



static int __init bIPC_init(void)
{
	int ret = 0;

	CPU_NUM = 3;
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
