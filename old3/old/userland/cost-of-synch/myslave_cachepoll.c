#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>



inline volatile unsigned long int RDTSCL() {
     unsigned long int x;
     __asm__ __volatile__("rdtscp" : "=a" (x) : : "edx");
     return x;
}

inline volatile long long RDTSC1() {
   register long long TSC asm("eax");
   //asm volatile ("rdtsc" : : : "eax", "edx");
   //asm volatile (".byte 15, 49" : : : "eax", "edx");
   return TSC;
}

#define DEPTH 1000

static unsigned long int calldata [DEPTH + 1];
static unsigned long int returndata [DEPTH + 1];

int shmid;
void *s_area = NULL;

#define S_SIZE 100*4096

int initialize_shm() {

	key_t key = 12345;

	shmid = shmget(key, S_SIZE, 0666);
	printf("shim id in slave is %d\n", shmid);
	if (-1 == shmid) {
		printf("In slave tehre was no shm by that key, creating\n");
		if (ENOENT != errno)
			return -2;

		shmid = shmget(key, S_SIZE, IPC_CREAT | 0666);
		if (-1 == shmid) 
               		return -3;
	}

	s_area = shmat(shmid, NULL, 0);

	printf("s_area after shmat(slave) is %p\n", s_area);
	if (-1 == (long)s_area) {
		return -2;
	}

	return 0;
};


static inline void clflush(volatile void *__p)
{
        asm volatile("clflush %0" : "+m" (*(volatile char *)__p));
}



volatile unsigned long int * volatile var;
volatile int * signal;
volatile int * signal1;

//#define SNOOP_DEPTH 32
#define SNOOP_DEPTH 256
#define SHIFT (64 * 1)
int main(void) {
	volatile unsigned long long local = 0;
	register unsigned long count = 0;
        unsigned long int tl0;
        int ret;
	int i;
	volatile unsigned int touchme;
	memset(calldata, 0, DEPTH);
	memset(returndata, 0, DEPTH);

	ret = initialize_shm();
	if (ret != 0) {
		printf("unable to alloc shmem\n");
		return -1;
	};

	nice(-20);
	printf("s_area starts at %p\n", s_area);
	signal = (volatile int *)((char*)s_area + 60);
	signal1 = (volatile int *)((char*)s_area + ((4096 * 50) + 60));
	if(*(volatile unsigned char *)s_area != 0x2 && *((volatile unsigned char *)s_area) != 0xAA){
		memset(s_area, 0x2, 4096*100);
	}
	//asm volatile("mfence");
	s_area = (char *)s_area + (4096 * 50);
	for (i = 0; i < DEPTH; i ++ ) {

		while ( *signal  != 0 )
			asm volatile("pause" ::: "memory");

		signal = (volatile int*)((char *)signal + 64);

		while(*signal1 == 1)
			asm volatile("pause" ::: "memory");

		memset(s_area, 0XAA, 60);
		*signal1 = 1;
		//clflush(s_area);
		signal1 = (volatile int*)((char *)signal1 + 64);
		s_area = (char *)s_area + 64;
	};
        return 0;
};

