#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define SHIFT (64*1)
#define DEPTH 1000

static unsigned long int calldata [DEPTH + 1];
static unsigned long int returndata [DEPTH + 1];

int shmid;
void *s_area = NULL;


#define S_SIZE 100*4096


static unsigned long RDTSCL(void)
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


static inline unsigned long _RDTSCL(void)
{
	unsigned int low, high;
	asm volatile("rdtsc" : "=a" (low), "=d" (high));
	return low | ((unsigned long)high) << 32;
}




/*
static inline uint64_t RDTSCL(void)
{
	uint64_t rax,rdx;
	asm volatile ( "rdtscp" : "=a" (rax), "=d" (rdx) : : );
	return (rdx << 32) + rax;
	}*/

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

};

static inline void clflush(volatile void *__p)
{
        asm volatile("clflush %0" : "+m" (*(volatile char *)__p));
}


static long long volatile local = 0;
volatile int * signal;
volatile int * signal1;
long long tmp;

int main(void) {
	//struct timespec start, end, result;
        unsigned long int tl0, tl1;
        int ret;
	volatile int i;
	volatile unsigned int touchme;
	//volatile unsigned long count = 10000;
	tl0 = tl1 = 0;
	memset(calldata,0,DEPTH);
	memset(returndata,0,DEPTH);

	ret = initialize_shm();
	if (ret != 0) {
		return -1;
	};

	if(nice(-20) != -20)
		perror("Couldn't nice!");

	signal = (volatile int *) ((char *)s_area + 60);
	signal1 = (volatile int *) ((char *)s_area + ((4096 * 50) + 60));

	printf("Master s_area starts at %p\n", s_area);
	if(*(volatile unsigned char *)s_area != 0x2 && *((volatile unsigned char *)s_area) != 0xAA)
		memset(s_area, 2, 4096 * 100);

	//__builtin_prefetch(s_area, 1,3);
	for (i = 0; i < DEPTH; i ++ ) {
		tl0 = RDTSC_START();
		while(*signal == 0)
			asm volatile("pause" ::: "memory");

		memset(s_area, 0xAA, 60);
		*signal = 0;
		//		clflush(s_area);

	       //asm volatile("nop");
		while (*signal1  != 1)
		       asm volatile("pause" ::: "memory");

		tl1 = RDTSCL();
		calldata[i] = tl1 - tl0;
		returndata[i] = local;
       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);
	};


	for (i = 0; i < DEPTH; i ++ ) {
		printf("returndata:%lu calldata%lu\n", returndata[i], calldata[i]);

        };

	//f.close();

        return 0;
};

