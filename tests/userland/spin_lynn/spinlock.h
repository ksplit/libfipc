/**
 * @File     : spinlock.h
 * @Author   : Minjun Cha
 */

#include <stdio.h>
#include <inttypes.h>

typedef unsigned int uint;

struct thread_spinlock {
  unsigned int locked;       // Is the lock held?
};

static inline uint xchg(volatile uint *addr, uint newval);

void thread_spin_init(struct thread_spinlock *lk);

void thread_spin_lock(struct thread_spinlock *lk);

void thread_spin_unlock(struct thread_spinlock *lk);
