#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>


#include <asm/uaccess.h>
#include <asm/x86/mwait.h>
#include <asm/page_types.h>




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




static const int CPU_NUM 1;


/* 124 byte message */
static char* msg = "The quick brown fox jumped over the lazy dog. Sally sells sea shells down by the sea shore. abcdefghijklmnopqrstuvwxyz123456";


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
	
	
static inline void monitor_mwait(unsigned long ecx, unsigned long *eax)
{
	/* TODO Figure out wtf the "extensions" and "hints" do for monitor */
	__monitor((void*)eax,0, 0);


}       



static void ipc_thread_func(void *input)
{ 
	/* This will be a while true loop with timing code and mwaits 
	   CPU 1 WILL be the first to send info. It will ROT 13 some
	   message and inc on some pointer at 128 bytes.

	   CPU 0 will come in here and will monitor/mwait on the 128
	   byte boundary until it gets out of its OPT loop
	*/
	
	struct file *filep = input;
	ipc_container *container = NULL;
	unsigned long ecx = 1; /*break of interrupt flag */
	unsigned long edx = 0;
	struct timespec64 start;
	struct timespec64 end;
	size_t offset = 0;
	void* buf;
	ipc_message *overlay;
	
	
	if(filep == NULL) {
		pr_debug("Thread was sent a null filepointer!\n");
		return;
	}

	container = filep->private_data;

	if(container == NULL && container->mem_size == 0){
		pr_debug("container was null in thread!\n");
		return;
	}
	buf = container->mem_start;


	/* setup hi-res timers */
	memset(&start,0,sizeof(struct timespec64));
	memset(&end,0,sizeof(struct timespec64));

	if(CPU_NUM == 1) {
		while(1) {
			/* we send first */
			overlay = buf;
			memcpy(overlay->message,msg,124);
			/* AT THIS VOLATILE WRITE WE SHOULD HAVE JUST WOKEN UP THE OTHER THREAD */
			overlay->monitor = 0xdeadbeef;
			offset += sizeof(*overlay);
			offset %= (PAGE_SIZE * 2);
			overlay += offset;
			
			/* mwait on response location */
			/* TIME ON */
			monitor_mwait(ecx,&overlay->monitor);
			/*TIME OFF!*/
		}
	}
	if(CPU_NUM == 3) {
		
		while(1) {
			/*we recv first */
			
		}
	}
	

}

static inline unsigned long beta_ret_cpu(unsigned long __arg)
{

	void __user *arg = (void*) __arg;
	return put_user(CPU_NUM, arg);
}


static unsigned long beta_unpark_thread(struct ipc_container *container)
{

	int ret = 0;
	if(container->thread == NULL || container->mem_size == 0) {
		return -EINVAL;
	}

	/* FROM THIS POINT FORWARD, ATLEAST ONE OF THE THREADS
	 * IS SITTING IN THE COMM CODE
	 */

        kthread_unpark(container->thread);
       
	
	return 0;
}


static unsigned long beta_connect_mem(struct ipc_container *container,
				      unsigned long __arg)
{

	void __user *ubuf = (void*) __arg;
	void *kland; 
	struct ipc_container *container;
	
	if(get_user(kland,ubuf)) {
		pr_debug("get_user failed connect_mem\n");
		return -EFAULT;
	
	}
	
	
	if(kland != NULL && *((unsigned long*)kland) != 0xdeadbeef) {
		return -EFAULT;
	}
	
	container->mem_start = kland;
	container->mem_size = PAGE_SIZE * 2;
	return 0;
}

static unsigned long beta_alloc_mem(struct ipc_container *container)
{
	if(mem_size > 0) {
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

static ssize_t beta_open(struct inode *nodp, struct file *filep)
{
	
	/* setup kernel thread, bound to some CPU, but do not run */
	struct ipc_container *container;
	
	container = kzalloc(sizeof(*container), GFP_KERNEL);
	
	if(!container) {
		pr_debug("Could not alloc space for container\n");
		return -1;
	}
	
	container->thread = kthread_create_on_cpu(&ipc_thread_func, (void*)filep,
				       CPU_NUM,"betaIPC.%u");
	
	if(IS_ERR(container->thread)) {
		pr_debug("Error while creating kernel thread\n");
		return PTR_ERR(container->thread);
	}
	
	filep->private_data = container;
	
	return ret;

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
	default:
		pr_debug("No such ioctl %d\n", cmd);
		break;
		
	}


}


/* Dispatch messages to work queue */
/* Perhaps this thread will read messages too? */

static int ipc_thread_func(void* data)
{
	pr_debug("ipcThreadFunc not implemented\n");
	return 0;
}


static const struct file_operations betaIPC_fops = {
	.owner	 = THIS_MODULE,
	.open    = beta_open,
	.ioctl   = beta_ioctl,
};

static struct miscdevice dev = {
	MISC_DYNAMIC_MINOR,
	"betaIPC",
	&betaIPC_fops,
};



static int __init bIPC_init(void)
{
	int ret = 0;
	
	/* reading through the source of misc.c it looks like register
	   will init everything else for us */
	ret = misc_register(&dev);
	if(ret) {
		pr_debug("Failed to register dev for BetaIPC\n");
		return ret;
	}


}
