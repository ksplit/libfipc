/**
 * @File     : spinlock.c
 * @Author   : Minjun Cha
 */

#include "spinlock.h"

static inline uint
xchg(volatile uint *addr, uint newval)
{
	uint result;

	// The + in "+m" denotes a read-modify-write operand.
	asm volatile("lock; xchgl %0, %1" :
				"+m" (*addr), "=a" (result) :
				"1" (newval) :
				"cc");
	return result;
}

void
thread_spin_init(struct thread_spinlock *lk)
{
	lk->locked = 0;
}

void
thread_spin_lock(struct thread_spinlock *lk)
{
	// The xchg is atomic.
	while(xchg(&lk->locked, 1) != 0)
		fipc_test_pause();

	__sync_synchronize();
}

void
thread_spin_unlock(struct thread_spinlock *lk)
{
	__sync_synchronize();

	asm volatile("movl $0, %0" : "+m" (lk->locked) : );
}
