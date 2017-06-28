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


static int compare(const void *_a, const void *_b){

	uint64_t a = *((uint64_t *)_a);
	uint64_t b = *((uint64_t *)_b);

	if(a < b)
		return -1;
	if(a > b)
		return 1;
	return 0;
}



static void dump_time(unsigned long*  timekeeper)
{
	int i;
	unsigned long long counter = 0;
        uint64_t min;
	uint64_t max;
	if (timekeeper == NULL) {
		printf("Time keeper was null, ret");
		return;
	}
	for (i = 1; i < NUM_LOOPS; i++) {
		printf("%lu\n", timekeeper[i]);
		counter+= timekeeper[i];
	}

	qsort(timekeeper, NUM_LOOPS, sizeof(uint64_t), compare);

	min = timekeeper[0];
	max = timekeeper[NUM_LOOPS-1];
	counter = min;
	for (i = 1; i < NUM_LOOPS; i++) {
		//printf("%lu\n", timekeeper[i]);
		counter+= timekeeper[i];
	}
	printf("MIN\tMAX\tAVG\tMEDIAN\n");
	printf("%lu & %lu & %llu & %lu\n", min, max, counter/NUM_LOOPS, timekeeper[4999]);

}




int main(void)
{

	int ret;

	static	unsigned int pTok = 0xC1346BAD;
	static	unsigned int cTok = 0xBADBEEF;
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


	prod_msg = s_area;
	cons_msg = (struct ipc_message*) ((char *)s_area + (4096 * 50));

	if(nice(-20) != -20)
		perror("Couldn't nice!");


	while (count < NUM_LOOPS) {
		start64 = RDTSC_START();
		while(prod_msg->monitor != pTok)
			asm volatile("pause" ::: "memory");

		prod_msg->monitor = cTok;

		while(cons_msg->monitor != cTok)
			asm volatile("pause" ::: "memory");

		prod_msg++;
		cons_msg++;
		end64 = RDTSCP();
		timekeeper[count] = end64-start64;
		count++;
	}

	dump_time(timekeeper);
	return EXIT_SUCCESS;
}
