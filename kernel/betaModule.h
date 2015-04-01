#include <linux/types.h>


int CHAN_NUM_PAGES = 256;
int BUF_SIZE = 60;
int NUM_LOOPS = 10000;
int FLOOD_SIZE = 64;

#if defined(USE_MWAIT)
	unsigned long ecx = 1; /*break of interrupt flag */
	unsigned long cstate_wait = 0x0; /* 4 states, 0x0, 0x1 0x10 0x20 */
#endif



struct ipc_container{
	struct task_struct *thread;
	struct ttd_ring_channel *channel_tx;
	struct ttd_ring_channel *channel_rx;
};

/*Don't let gcc do anything cute, we need this to be 128 bytes */
struct ipc_message{
	char message[60];
	volatile uint32_t monitor;
}__attribute__((packed));






#define BETA_GET_CPU_AFFINITY 1<<1
#define BETA_CONNECT_SHARED_MEM 1<<2
#define BETA_UNPARK_THREAD 1<<3
#define BETA_ALLOC_MEM 1<<4
#define BETA_GET_MEM 1<<5
#define BETA_DUMP_TIME 1<<6
