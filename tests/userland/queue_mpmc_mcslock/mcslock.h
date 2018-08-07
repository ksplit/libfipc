/**
 * @File     : mcslock.h
 * @Author   : Jiwon Jeon
 */

#include <stdint.h>

#ifndef LIBFIPC_TEST
#include "../libfipc_test.h"
#endif

#ifndef __KERNEL__ 
#define pr_err printf 
#endif 

#define MAX_MCS_LOCKS        2

// Types
typedef uint64_t data_t;

typedef struct qnode {
    volatile void* CACHE_ALIGNED next; 
    volatile char CACHE_ALIGNED waiting; 
} qnode; 

typedef qnode mcslock;

/*
static inline uint64_t
cmp_and_swap_atomic(mcslock* L, uint64_t cmpval, uint64_t newval)
{
    uint64_t out;
    __asm__ volatile(
                "lock; cmpxchgq %2, %1"
                //: "=a" (out), "+m" ((L->v)->locked)
                : "=a" (out), "+m" (L->v)
                : "q" (newval), "0"(cmpval)
                : "cc");
    return out == cmpval;
}
*/

static inline mcslock*
fetch_and_store ( mcslock** L, qnode** val )
{
    mcslock* ret = *L;
    __asm__ volatile(
                "lock; xchgq %0, %1\n\t"
                : "=r" (L)
                :  "m" (*L), "0" (*val)
                : "memory", "cc");
    return ret;
}
 
static inline uint64_t
cmp_and_swap ( mcslock *L, uint64_t cmpval, uint64_t newval )
{
    uint64_t out;
    __asm__ volatile(
                "lock; cmpxchgq %2, %1"
                : "=a" (out), "+m" (L)
                : "q" (newval), "0"(cmpval)
                : "cc");
    return out == cmpval;
}

void mcs_init_global( mcslock* L );
void mcs_init_local	( qnode* I );
//void mcs_init	( mcslock *L );
void mcs_lock 	( mcslock *L, qnode* I );
void mcs_unlock ( mcslock *L, qnode* I );
