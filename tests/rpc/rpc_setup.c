#include <../../IPC.h>
#include <linux/spinlock.h>.


static volatile int migrate;
static spinlock_t lock = SPIN_LOCK_UNLOCKED;


int dispatch(void *data)
{


	/*
	 * Normally one person doesn't create both threads
	 * This means they both wont call the same dispatch function
	 * but since I've created two threads we need to seperate them
	 * into calle and callers.
	 * that is what this code below is doing
	 */
	spin_lock(&lock);
	if(migrate == 0) {
		migrate = 1;
		spin_unlock(&lock);
		calle((struct ttd_ring_channel *)data);
		return 1;
	}
	spin_unlock(&lock);
	caller((struct ttd_ring_channel *)data);
	return 1;
}

