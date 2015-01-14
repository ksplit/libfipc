#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/irqflags.h>
//#include <linux/timekeeping.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>
#include <asm/mwait.h>
#include <asm/page_types.h>
#include <asm/cpufeature.h>

#include "betaModule.h"

MODULE_LICENSE("GPL");


/** 
 *   Useful things to look at:
 *   wait_event_interruptible /wait.h
 *   alloc_page / gfp.h
 * http://lxr.free-electrons.com/source/arch/x86/include/asm/mwait.h#L26
 *   
*/

/** TODO  and THOUGHTS
 *
 *  TODO: Move kthread creation into open so we can install 
 *        In filp->private_data? -- Think about context of 
 *        who calls this code... It's coming from kland?
 *
 *  TODO: Mutex on current_avail_token and the array of mem_tokens
 *  TODO: will atomic inc work for cur_avail_token?
 */




static const int CPU_NUM = 0;


/* 124 byte message */
static char* msg = "The quick brown fox jumped over the lazy dog."\
	"Sally sells sea shells down by the sea shore. abcdefghijkl"\
	"mnopqrstuvwxyz12345";


static inline int keep_waiting(struct ipc_message *i_msg, unsigned int notify_key)
{
	return !(i_msg->monitor == notify_key);
}


static void send_and_notify(struct ipc_message *i_msg, char* _msg, size_t len,
			    unsigned int notify_key)
{
		memcpy(i_msg->message,_msg,len);
		/*		pr_debug("BETA %d WRITING TO VOLTAILE VAR AT %p\n", CPU_NUM, 
				&i_msg->monitor);*/
		i_msg->monitor = notify_key;

} 


static inline void* get_current_slot(size_t offset, void* buf)
{
	return (void*) ((char*)buf + offset);
}

/* All the casts are in here to stick within the standard. 
 * 6.2.5-19: The void type comprises an empty set of values; 
 * it is an incomplete type that cannot be completed.
 */
static inline void* get_next_slot(size_t *offset, void* buf)
{
	*offset += sizeof(struct ipc_message);
	*offset %= (PAGE_SIZE * 2);
	return (void*) ((char*)buf) + *offset;
}




/* Stolen and slightly modified from http://rosettacode.org/wiki/Rot-13 */
static char *rot13(char *s, int amount)
{
        char *p=s;
        int upper;
	int count = 0;
        while(*p && count < amount) {
                upper = *p;
                if((upper>='a' && upper<='m') ||  (upper>='A' && upper<='M')) *p+=13;
                else if((upper>='n' && upper<='z') || (upper>='A' && upper<='Z')) *p-=13;
                ++p;
		count++;
        }
        return s;
}

static void assert_expect_and_zero(struct ipc_message *i_msg, int need_rot)
{
	if(need_rot){
		rot13(i_msg->message,123);
	}
	if(strncmp(i_msg->message,msg,124) != 0){
		pr_debug("STRINGS DIFFERED IN CPU %d\n", CPU_NUM);
	}
	i_msg->monitor = 0;
}	
	
 static void ___monitor(const void *eax, unsigned long ecx,
                               unsigned long edx)
 {
         /* "monitor %eax, %ecx, %edx;" */
	 //asm volatile("mov $0, %rbx;");
	 asm volatile(".byte 0x0f, 0x01, 0xc8;"
		      :: "a" (eax), "c" (ecx), "d"(edx));
 }

static void ___mwait(unsigned long eax, unsigned long ecx)
{
	/* "mwait %eax, %ecx;" */
	//asm volatile("mov $0, %rbx;");
	asm volatile(".byte 0x0f, 0x01, 0xc9;"
		     :: "a" (eax), "c" (ecx));
}


static inline void monitor_mwait(unsigned long rcx, volatile void *rax,
				 unsigned long wait_type)
{

	unsigned long flags;

	/* TODO Figure out wtf the "extensions" and "hints" do for monitor */
	if (this_cpu_has(X86_BUG_CLFLUSH_MONITOR))
		clflush(rax);
	local_irq_save(flags);
	___monitor((void*)rax,0, 0);
	smp_mb();
	___mwait(wait_type,rcx);
	local_irq_restore(flags);
}       



static int ipc_thread_func(void *input)
{ 
	/* This will be a while true loop with timing code and mwaits 
	   CPU 1 WILL be the first to send info. It will ROT 13 some
	   message and inc on some pointer at 128 bytes.

	   CPU 0 will come in here and will monitor/mwait on the 128
	   byte boundary until it gets out of its OPT loop
	*/
	
	struct file *filep = input;
	struct ipc_container *container = NULL;
	unsigned long ecx = 1; /*break of interrupt flag */
	unsigned long cstate_wait = 2;
	//	struct timespec64 start;
	//struct timespec64 end;
	size_t offset = 0;
	void* buf;
	struct ipc_message *overlay;
	int count = 0;
	int retry_count = 0;
	
	if(filep == NULL) {
		pr_debug("Thread was sent a null filepointer!\n");
		return -EINVAL;
	}

	container = filep->private_data;

	if(container == NULL && container->mem_size == 0){
		pr_debug("container was null in thread!\n");
		return -EINVAL;
	}
	buf = container->mem_start;

	/* setup hi-res timers */
	//	memset(&start,0,sizeof(struct timespec64));
	//	memset(&end,0,sizeof(struct timespec64));


	pr_debug("Hello from thread in CPU %d\n", CPU_NUM);
	if(CPU_NUM == 0) {
		while(count < 150) {
			/* we send first */
			overlay = get_current_slot(offset, buf);
			send_and_notify(overlay, msg, 124, 0xdeadbeef);
			overlay = get_next_slot(&offset, buf);

			/* mwait on response location */
			//pr_debug("BETA %d WAITING ON MONITOR, at %p\n", CPU_NUM, &overlay->monitor);
			/* TIME ON */
		retry:
			monitor_mwait(ecx,&overlay->monitor, cstate_wait);
			/*TIME OFF!*/
		   
			if(keep_waiting(overlay,0xbadc0de) && retry_count < 150) {
				retry_count++;
				pr_debug("Retrying with count %d on CPU %d\n", 
					 retry_count, CPU_NUM);
				goto retry;
			}
			else if(retry_count > 150){
				printk(KERN_DEBUG "TERMINATING EARLY ON CPU %d\n", CPU_NUM);
				break;
			}
			assert_expect_and_zero(overlay,1);
			get_next_slot(&offset,buf);
			retry_count = 0;
			count++;			
		}
	}
	return 0;
}

static inline unsigned long beta_ret_cpu(unsigned long __arg)
{

	unsigned long __user *arg = (void*) __arg;
	return put_user(CPU_NUM, arg);
}


static unsigned long beta_unpark_thread(struct ipc_container *container)
{
	if(container->thread == NULL || container->mem_size == 0) {
		return -EINVAL;
	}

	/* FROM THIS POINT FORWARD, ATLEAST ONE OF THE THREADS
	 * IS SITTING IN THE COMM CODE
	 */
	pr_debug("waking up process on CPU %d\n", CPU_NUM);
	kthread_unpark(container->thread);
	if(wake_up_process(container->thread) == 1){
		pr_debug("Woke up process on cpu %d\n", CPU_NUM);
	}
	//kthread_stop(container->thread);
	
	return 0;
}


static unsigned long beta_connect_mem(struct ipc_container *container,
				      unsigned long __arg)
{

	unsigned long  __user *ubuf = (void*) __arg;
	unsigned long kland_real; 
	unsigned long *kland;
	
	if(get_user(kland_real,ubuf)) {
		pr_debug("get_user failed connect_mem with ptr %p\n", ubuf);
		return -EFAULT;
	
	}
	
	kland = (unsigned long*) kland_real;
	
	if(kland != NULL && *kland != 0xdeadbeef) {
		return -EFAULT;
	}
	
	container->mem_start = kland;
	container->mem_size = PAGE_SIZE * 2;
	return 0;
}

static unsigned long beta_alloc_mem(struct ipc_container *container)
{
	if(container->mem_size > 0) {
		return 0;
	}
	container->mem_start = kzalloc((PAGE_SIZE * 2), GFP_KERNEL);
	if(!container->mem_start) {
		return -1;
	}
	
	container->mem_size = PAGE_SIZE * 2;
	
	/* Yes I know this is outright disgusting */
	*((unsigned long*)container->mem_start) = 0xdeadbeef;
	
	return 0;
}

static int beta_open(struct inode *nodp, struct file *filep)
{
	
	/* setup kernel thread, bound to some CPU, but do not run */
	struct ipc_container *container;
	
	container = kzalloc(sizeof(*container), GFP_KERNEL);
	
	if(!container) {
		pr_debug("Could not alloc space for container\n");
		return -1;
	}
	
	container->thread = kthread_create_on_cpu(&ipc_thread_func, (void*)filep,CPU_NUM,"betaIPC.%u");
	
	if(IS_ERR(container->thread)) {
		pr_debug("Error while creating kernel thread\n");
		return PTR_ERR(container->thread);
	}


	filep->private_data = container;
	
	return 0;

}

static int beta_close(struct inode *nodp, struct file *filep)
{
	/* TODO STOP LEAKING 2 PAGES OF MEMORY FROM THE CONTAINER!\n */
	struct ipc_container *container;
	container = filep->private_data;
	//kthread_stop(container->thread);
	kfree(container);
	return 0;
}

static long beta_return_mem(struct ipc_container *container,
			    unsigned long __arg)
{
	unsigned long __user  *save = (unsigned long *) __arg;
	return put_user((unsigned long)container->mem_start, save);
}

static long beta_ioctl(struct file *filep, unsigned int cmd,
			  unsigned long __arg)
{
	
	struct ipc_container *container = filep->private_data;
	long ret = 0;
	
	switch(cmd){
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
	return 0;
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
	

	if(this_cpu_has(X86_FEATURE_MWAIT)){
		printk(KERN_DEBUG "HAS MWAIT\n");
	}

	/* reading through the source of misc.c it looks like register
	   will init everything else for us */
	pr_debug("hello from bIPC with pr_debug\n");
	printk(KERN_DEBUG "Hello from bIPC with printk\n");
	ret = misc_register(&dev);
	if(ret) {
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
