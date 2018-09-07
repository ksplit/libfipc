/**
 * @File     : mcs.h
 */

#include <stdio.h>
#include <stdint.h>

#ifndef __KERNEL__ 
#define pr_err printf 
#endif 

#define MAX_MCS_LOCKS        2

// Types
typedef uint64_t data_t;

typedef struct qnode {
    struct qnode* next; 
    char waiting; 
} qnode; 

typedef qnode mcslock;

// L = val, return former L value
static inline qnode*
fetch_and_store ( mcslock** L, qnode* val )
{
    qnode* ret = *L;
    __asm__ volatile(
                "lock; xchgq %0, %1\n\t"
                : "=r" (L)
                :  "m" (*L), "0" (val)
                : "memory", "cc");
    return ret;
}
 
// if L == cmpval, L = newval
static inline uint64_t
cmp_and_swap ( mcslock **L, uint64_t cmpval, uint64_t newval )
{
    uint64_t out;
    __asm__ volatile(
                "lock; cmpxchgq %2, %1"
                : "=a" (out), "+m" (*L)
                : "q" (newval), "0"(cmpval)
                : "cc");
    return out == cmpval;
}

void mcs_init_global( mcslock** L );
void mcs_init_local	( qnode* I );
void mcs_lock 	( mcslock **L, qnode* I );
void mcs_unlock ( mcslock **L, qnode* I );

