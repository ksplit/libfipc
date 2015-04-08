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


inline volatile unsigned long int RDTSCL() {
     unsigned long int x;
     __asm__ __volatile__("rdtsc" : "=a" (x) : : "edx");
     return x;
}

inline volatile long long RDTSC1() {
   register long long TSC asm("eax");
   //asm volatile ("rdtsc" : : : "eax", "edx");
   //asm volatile (".byte 15, 49" : : : "eax", "edx");
   return TSC;
}


static unsigned long int calldata [DEPTH + 1];
static unsigned long int returndata [DEPTH + 1];

int shmid;
void *s_area = NULL;


int initialize_shm() {

	key_t key = 1112;
	printf("Slave about to shmget\n");
	shmid = shmget(key, S_SIZE, 0666);
	printf("got shmget with kjey %d\n",shmid);
	if (-1 == shmid) {

		if (ENOENT != errno)
			return -2;

		shmid = shmget(key, S_SIZE, IPC_CREAT | 0666);
		if (-1 == shmid) 
               		return -3;
	}

	s_area = shmat(shmid, NULL, 0);
	if (-1 == (long)s_area) {
		return -2;
	}

	return 0;
};

volatile unsigned long int * volatile var;
int  * volatile signal;


int main(void) {

        unsigned long int tl0;
        int ret;

        for ( int i = 0; i < DEPTH; i++) {
                calldata[i] = 0;
                returndata[i] = 0;
        };

	ret = initialize_shm();
	if (ret != 0) {
		std::cerr << "Unable to allocate shared memory region\n";
		return -1;
	};

	ret = nice(-20);

	var = ((unsigned long int *)s_area); 
	signal = (int *)((char*)s_area + S_SIZE - sizeof(int));	
	
	for ( int i = 0; i < DEPTH; i ++ ) {
		
		/*
		 * Snoop variable 
		 */
		//	printf("About to start writing in slave\n");
		var = ((volatile unsigned long int *)s_area); 
		for( int j = 0; j < SNOOP_DEPTH; j ++) {  
			//*var = i + j;
			//var = (volatile unsigned long int  *)((char *)var + SHIFT);
		};
		//		printf("done writing\n");
		asm volatile ("mfence"); /* to ensure that snooping completes before I fire up signal */

		*signal = SLAVE_READY; 

		asm volatile ("mfence");	

		std::cout << "Waiting for master\n";
	
		do {
			asm volatile ("nop");

		} while ( *signal != MASTER_READY );


	};
	
	/*
	 * Dump the data to console
	 */
//	for ( int i = 0; i < DEPTH; i ++ ) {
//                std::cout << calldata[i] << "\n";
//        };


        return 0;
};

