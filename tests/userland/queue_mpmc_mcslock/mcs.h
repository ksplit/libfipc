/**
 * @File    : mcs.h
 *
 */

#include <stdio.h>
#include <stdint.h>

#ifndef LIBFIPC_TEST
#include "../libfipc_test.h"
#endif

/* Compile read-write barrier */
#define barrier() asm volatile("": : :"memory")

/* Atomic exchange (of various sizes) */
static inline void *xchg_64(void *ptr, void *x)
{
        __asm__ __volatile__("xchgq %0,%1"
                             :"=r" ((unsigned long long) x)
                             :"m" (*(volatile long long *)ptr), "0" ((unsigned long long) x)
                             :"memory");
        return x;
}

//typedef struct mcs_lock_t mcs_lock_t;
typedef struct qnode
{
        struct qnode *next;
        int spin;
} qnode;

typedef qnode mcslock;

//typedef struct mcs_lock_t *mcs_lock;
void mcs_init_global( mcslock** L );
void mcs_lock	( mcslock **L, qnode* I);
void mcs_unlock	( mcslock **L, qnode* I);

