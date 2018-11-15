/**
 * @File     : spinlock.h
 * @Author   : Minjun Cha
 */

#include <stdio.h>
#include <stdint.h>

#ifndef LIBFIPC_TEST
#include "../libfipc_test.h"
#endif

typedef unsigned int uint;

struct thread_spinlock {
  unsigned int locked;       // Is the lock held?
};

void thread_spin_init(struct thread_spinlock *lk);

void thread_spin_lock(struct thread_spinlock *lk);

void thread_spin_unlock(struct thread_spinlock *lk);

