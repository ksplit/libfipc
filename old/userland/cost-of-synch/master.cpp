#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <sys/shm.h>
#include <unistd.h>
#include "config.h"

inline volatile long long RDTSC() {
     unsigned long long int x;
     //__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     __asm__ __volatile__ ("rdtsc" : "=A" (x));
     return x;
}
inline volatile void NOP() {

        //asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
        asm volatile ("nop;");
        return;
}

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

static inline unsigned long _RDTSCL(void)
{
  unsigned int low, high;
  asm volatile("rdtscp" : "=a" (low), "=d" (high));
  return low | ((unsigned long)high) << 32;
}


/*inline volatile unsigned long int RDTSCL() {
     unsigned long int x;
     __asm__ __volatile__("rdtsc" : "=a" (x) : : "edx");
     return x;
     }*/

inline volatile long long RDTSC1() {
   register long long TSC asm("eax");
   //asm volatile ("rdtsc" : : : "eax", "edx");
   //asm volatile (".byte 15, 49" : : : "eax", "edx");
   return TSC;
}

	
//#define OP(x) 	asm volatile ("movl %0, %%eax\n" : :"r"( *(unsigned long long *)( ((char*)var) + x*SHIFT*8) ): "eax")
	
//#define OP(x) 	asm volatile ("movl (%0), %%eax\n" : :"r"((char*)var + (x)*SHIFT): "eax")
//#define OP(x) 		asm volatile ("movl (%0), %%eax\n" :"=a"(tmp) :"r"(((char*)var) + (x)*SHIFT): ); \
			local +=tmp;

#define OP(x) local +=  *(unsigned long int *)(((char*)var) + (x)*SHIFT);
		    



//#define OP(x)   __asm__ __volatile__ ("movl (%2), %%eax\n" : :"r"(((char*)var) + (x)*SHIFT): "eax")

//#define OP(x) __asm__ __volatile__ ("movl %0, %%eax\n" : :"r"( *(__u32 *)( ((char*)p) + (x)*SHIFT) ): "eax")

//#define OP(x)   __asm__ __volatile__ ("movl %0, %%eax\n" : "=a"(tmp) : "r"( *(unsigned long int *) ( (char*)p) + (x)*SHIFT) ); \
                               local +=tmp;

#define OP10(_x) OP( (_x *10 + 0) ); OP( (_x *10 + 1) ); OP( (_x *10 + 2) ); OP( (_x *10 + 3) ); \
                 OP( (_x *10 + 4) ); OP( (_x *10 + 5) ); OP( (_x *10 + 6) ); OP( (_x *10 + 7) ); \
                 OP( (_x *10 + 8) ); OP( (_x *10 + 9) );

#define OP100(_x) OP10( (_x *10 + 0) ); OP10( (_x *10 + 1) ); OP10( (_x *10 + 2) ); OP10( (_x *10 + 3) ); \
                  OP10( (_x *10 + 4) ); OP10( (_x *10 + 5) ); OP10( (_x *10 + 6) ); OP10( (_x *10 + 7) ); \
                  OP10( (_x *10 + 8) ); OP10( (_x *10 + 9) );

#define OP1000(_x) OP100( (_x *10 + 0) ); OP100( (_x *10 + 1) ); OP100( (_x *10 + 2) ); OP100( (_x *10 + 3) ); \
                   OP100( (_x *10 + 4) ); OP100( (_x *10 + 5) ); OP100( (_x *10 + 6) ); OP100( (_x *10 + 7) ); \
                   OP100( (_x *10 + 8) ); OP100( (_x *10 + 9) );

/*
 * Pseudo random order of memory access
 */
#define PR_OP10(_x) OP( (_x *10 + 8) ); OP( (_x *10 + 4) ); OP( (_x *10 + 9) ); OP( (_x *10 + 5) ); \
                    OP( (_x *10 + 2) ); OP( (_x *10 + 6) ); OP( (_x *10 + 3) ); OP( (_x *10 + 0) ); \
                    OP( (_x *10 + 7) ); OP( (_x *10 + 1) );

#define PR_OP100(_x) PR_OP10( (_x *10 + 5) ); PR_OP10( (_x *10 + 4) ); PR_OP10( (_x *10 + 8) ); PR_OP10( (_x *10 + 2) ); \
                     PR_OP10( (_x *10 + 0) ); PR_OP10( (_x *10 + 3) ); PR_OP10( (_x *10 + 9) ); PR_OP10( (_x *10 + 6) ); \
                     PR_OP10( (_x *10 + 1) ); PR_OP10( (_x *10 + 7) );

#define PR_OP1000(_x) PR_OP100( (_x *10 + 1) ); PR_OP100( (_x *10 + 6) ); PR_OP100( (_x *10 + 3) ); PR_OP100( (_x *10 + 4) ); \
                      PR_OP100( (_x *10 + 2) ); PR_OP100( (_x *10 + 7) ); PR_OP100( (_x *10 + 8) ); PR_OP100( (_x *10 + 5) ); \
                      PR_OP100( (_x *10 + 9) ); PR_OP100( (_x *10 + 0) );

static unsigned long int calldata [DEPTH + 1];
static unsigned long int returndata [DEPTH + 1];

int shmid;
void *s_area = NULL;



int initialize_shm() {

	key_t key = 1112;

	shmid = shmget(key, S_SIZE, 0666);

	if (-1 == shmid) {


           if (ENOENT != errno) {
               return -2;
           }

           shmid = shmget(key, S_SIZE, IPC_CREAT | 0666);

           if (-1 == shmid) {

               return -3;

           }
       }
	s_area = shmat(shmid, NULL, 0);
	if (-1 == (long)s_area) {
		
		return -4;
	}

	return 0;	

};

static long long volatile local = 0;
int  * volatile signal;
long long * volatile var; 
long long tmp;

int main(void) {

        unsigned long int tl0, tl1;
        int ret;

        for ( int i = 0; i < DEPTH; i++) {
                calldata[i] = 0;
                returndata[i] = 0;
        };

	ret = initialize_shm();
	if (ret != 0) {
		std::cerr << "Unable to allocate shared memory region" << ret <<"\n";
		return -1;
	};

	ret = nice(-20);

	var = ((long long *)s_area);

	printf("var:%p\n", var);

	signal = (int *)((char *)(s_area) + S_SIZE - sizeof(int));	

	
	for ( int i = 0; i < DEPTH; i ++ ) {
	
		tmp = 0; 
		local = 0;

		std::cout << "Waiting for slave\n"; 
		
		do {

			asm volatile ("nop");

		} while ( *signal != SLAVE_READY );

		//		asm volatile("lfence");
		tl0 = RDTSCL();

#if 0  /* pseudorandom access 16 sets */

                PR_OP10(0);
                OP(10);
                OP(16);
		OP(13);
		OP(15);
		OP(14);
		OP(11);

#endif /* end: pseudorandom access */

		
#if 0 /* pseudorandom access 32 sets */
                PR_OP10(0);
                PR_OP10(1);
                PR_OP10(2);

                OP(31);
                OP(30);

#endif /* end: pseudorandom access */

#if 0 /* sequential access, 32 sets */

                OP10(0);
                OP10(1);
                OP10(2);

                OP(30);
                OP(31);

#endif /* end: sequential access */

#if 0 /* sequential access in a loop,  32 sets */
		for(int k = 0; k < 32; k++ ) {
			OP(k);
		};
		
#endif 		
		
#if 0  /* pseudorandom access 128 lines */

                PR_OP100(0);
                PR_OP10(10);

		OP(22);
		OP(27);
		OP(23);
		OP(26);
		OP(24);
                OP(21);
		OP(25);
                OP(20);
	

#endif /* end: pseudorandom access */

#if 0 /* sequential access 256 lines */

                OP100(0);
		OP100(1);
                OP10(20);
		OP10(21);
		OP10(22);
		OP10(23);
		OP10(24);

		OP(250);
		OP(251);
		OP(252);
		OP(253);
                OP(254);
		OP(255);		

#endif /* end: pseudorandom access */


		
#if 0  /* pseudorandom access 256 lines */

                PR_OP100(0);
		PR_OP100(1);
                PR_OP10(20);
		PR_OP10(21);
		PR_OP10(22);
		PR_OP10(23);
		PR_OP10(24);

		OP(253);
		OP(251);
		OP(254);
		OP(252);
                OP(255);
		OP(250);		

#endif /* end: pseudorandom access */

#if 0 /* sequential access in a loop,  256 sets */
		for(int k = 0; k < 256; k++ ) {
			OP(k);
		};
		
#endif 		

		
#if 0  /* pseudorandom access 512 lines */

                PR_OP100(0);
		PR_OP100(1);
		PR_OP100(2);
		PR_OP100(3);
		PR_OP100(4);

                PR_OP10(50);
		
		OP(510);
		OP(511);
		

#endif /* end: pseudorandom access */
		//		asm volatile ("lfence");		
		tl1 = RDTSCL();



		calldata[i] = tl1 - tl0;
		returndata[i] = local;

		asm volatile ("mfence");

		*signal = MASTER_READY;

		asm volatile  ("mfence"); 


	};

        *signal = MASTER_READY;

	/*
	 * Dump the data to console
	 */

	//std::ofstream f("results.out"); 

	for ( int i = 0; i < DEPTH; i ++ ) {
                //f << "returndata:" << returndata[i] << " calldata" << calldata[i] << "\n";
		std::cout << "returndata:" << returndata[i] << " calldata" << calldata[i] << "\n";

        };

	//f.close();

        return 0;
};

