#include <linux/types.h>

struct ipc_container{
	struct task_struct *thread;
	size_t mem_size;
	void * mem_start;
};

/*Don't let gcc do anything cute, we need this to be 128 bytes */
struct ipc_message{ 
	char message[124];
	volatile uint32_t monitor;
}__attribute__((packed));










#define BETA_GET_CPU_AFFINITY 1<<1
#define BETA_CONNECT_SHARED_MEM 1<<2
#define BETA_UNPARK_THREAD 1<<3
#define BETA_ALLOC_MEM 1<<4

