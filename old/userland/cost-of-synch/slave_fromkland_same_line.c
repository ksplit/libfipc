#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>


struct ipc_message{
	char message[60];
	volatile uint32_t monitor;
}__attribute__((packed));

int shmid;
void *s_area = NULL;
const unsigned int S_SIZE = 4096 * 100;
const unsigned long NUM_LOOPS = 3136;

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


static inline void clflush(volatile void *__p)
{
        asm volatile("clflush %0" : "+m" (*(volatile char *)__p));
}

int initialize_shm() {

	key_t key = 12345;

	shmid = shmget(key, S_SIZE, 0666);
	   printf("shmid in master is %d\n",shmid);
	if (-1 == shmid) {
		printf("In master, there was no shm by that key, creating\n");
           if (ENOENT != errno) {
               return -2;
           }

           shmid = shmget(key, S_SIZE, IPC_CREAT | 0666);
	   printf("shmid in master is %d\n",shmid);
           if (-1 == shmid) {
               return -3;
           }


       }

	s_area = shmat(shmid, NULL, 0);
	printf("s_area after shmat(master) is %p\n", s_area);
	if (-1 == (long)s_area) {
		return -4;
	}

	return 0;

}



int main(void)
{

	int ret,i;

	static	unsigned int pTok = 0xC1346BAD;
	static	unsigned int msg_available = 0xBADBEEF;
	static unsigned int msg_ready = 0x13370000;
	struct ipc_message *prod_msg, *cons_msg;
	unsigned int count = 0;
	unsigned long  start64, end64;

	unsigned long *timekeeper = malloc(NUM_LOOPS * sizeof(unsigned long));
	if(!timekeeper)
		return EXIT_FAILURE;

	memset(timekeeper,0,NUM_LOOPS);

	ret = initialize_shm();
	if (ret != 0)
		return EXIT_FAILURE;

	memset(s_area,0xFF,S_SIZE);
	//for(i = 0; i < (S_SIZE/sizeof(unsigned int)); i++)
	//	*(((volatile unsigned int *)s_area)+i) = pTok;
	//	for(; < NUM_LOOPS; i++)
	//	*((unsigned int *)s_area+i) = cTok;

	while(count != NUM_LOOPS) {
		clflush(s_area + (sizeof(struct ipc_message) * count));
		count++;
	}
	count = 0;
	cons_msg = s_area;
	prod_msg = s_area;//(struct ipc_message*) ((char *)s_area + (4096 * 50));

	if(nice(-20) != -20)
		perror("Couldn't nice!");


	while (count < NUM_LOOPS) {

		while(cons_msg->monitor != msg_available)
			asm volatile("pause" ::: "memory");

		//printf("Slave %d\n",count);
		prod_msg->monitor = msg_ready;
		count++;
	}

	return EXIT_SUCCESS;
}
