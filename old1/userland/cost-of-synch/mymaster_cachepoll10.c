#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#define SHIFT (64*1)
#define DEPTH 1000

static unsigned long int calldata [DEPTH + 1];
static unsigned long int returndata [DEPTH + 1];

int shmid;
void *s_area = NULL;


#define S_SIZE 100*4096


static inline unsigned long RDTSCL(void)
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

static inline unsigned long _RDTSCL(void)
{
	unsigned int low, high;
	asm volatile("rdtscp" : "=a" (low), "=d" (high));
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

static long long volatile local = 0;
volatile int * volatile signal;
volatile int * volatile signal1;
long long tmp;

int main(void) {
        unsigned long int tl0, tl1;
        int ret;
	volatile int i;
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

	//signal = (volatile int *)((char *)(s_area) + S_SIZE - sizeof(int));
	signal = (volatile int *) ((char *)s_area + 60);
	signal1 = (volatile int *) ((char *)s_area + ((4096 * 50) + 60));
	printf("Master s_area starts at %p\n", s_area);
	if(*(char *)s_area != 0x2)
		memset(s_area, 2, 4096 * 100);
	asm volatile("mfence");
	for (i = 0; i < DEPTH; i+= 10 ) {
		printf("%d\n",i);

		tl0 = RDTSCL();

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//	asm volatile("sfence");
		*signal = 0;
		do {
			//asm volatile("nop");
			asm volatile("pause" ::: "memory");
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);


		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64, 1, 0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);

		__builtin_prefetch((char*)s_area + 64,1,0);
		memset(s_area, 0xAA, 60);
		//		asm volatile("sfence");
		*signal = 0;
		do {
			asm volatile("pause" ::: "memory");;
		} while (*signal1 != 1);

       		signal = (volatile int*)((char *)signal + SHIFT);
		signal1 = (volatile int*)((char *)signal1 + SHIFT);
		s_area = ((char*) s_area + SHIFT);


		tl1 = RDTSCL();
		calldata[i] = tl1 - tl0;
		returndata[i] = local;
		
	};


	for (i = 0; i < DEPTH; i ++ ) {
		printf("returndata:%lu calldata%lu\n", returndata[i], calldata[i]);

        };

	//f.close();

        return 0;
};

