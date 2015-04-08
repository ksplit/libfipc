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

volatile unsigned long int * volatile var; 
volatile int  * volatile signal;
volatile int  * volatile signal1;

//#define SNOOP_DEPTH 32
#define SNOOP_DEPTH 256
#define SHIFT (64 * 1)
int main(void) {
	volatile unsigned long long local = 0;
        unsigned long int tl0;
        int ret;


 	memset(calldata,0,DEPTH);
	memset(returndata,0,DEPTH);
	ret = initialize_shm();
	if (ret != 0) {
		printf("unable to alloc shmem\n");
		return -1;
	};

	nice(-20);
	printf("s_area starts at %p\n", s_area);
	//	memset(s_area, 2 ,4096 * 100);
	signal = (volatile int *)((char*)s_area + 124);
	//signal = (volatile int *)((char *)(s_area) + S_SIZE - sizeof(int));
	memset(s_area, 0x2, 4096*100);
	s_area = (char *)s_area + (4096 * 50);
	signal1 = (volatile int *)((char*)s_area + 124);
	for ( volatile int i = 0; i < DEPTH-1; i ++ ) {
		do {
			asm volatile ("nop");
		} while ( *signal != 0 );
		//local += *(volatile unsigned long long *)signal - 68;
		memset(s_area, 0XAA, 64);
		//asm volatile("sfence");
		*signal1 = 1;
		//asm volatile("lfence");
		signal = (volatile int*)((char *)signal + 64);
		signal1 = (volatile int*)((char *)signal1 + 64);
		s_area = ((char *) s_area + 64);
		//		printf("%llu ", local);
		local = 0;
	};

        return 0;
};

