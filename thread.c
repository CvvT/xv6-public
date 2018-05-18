//thread.c

#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"
#include "x86.h"

#define NULL 0

struct spinlock {
  uint locked;       // Is the lock held?

  int pid;   // The thread holding the lock.
};

// Check whether this thread is holding the lock.
int
holding(struct spinlock *lock)
{
  return lock->locked && lock->pid == getpid();
}

void
initlock(struct spinlock *lk)
{
  lk->locked = 0;
  lk->pid = 0;
}

void
acquire(struct spinlock *lk)
{
  if(holding(lk)) {
    printf(2, "acquire");
    return;
  }

  // The xchg is atomic.
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();

  lk->pid = getpid();
}

void
release(struct spinlock *lk)
{
  if(!holding(lk)) {
    printf(2, "release");
    return;
  }

  lk->pid = 0;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code can't use a C assignment, since it might
  // not be atomic. A real OS would use C atomics here.
  asm volatile("movl $0, %0" : "+m" (lk->locked) : );

}

int thread_create(void *(*start_routine)(void*), void *arg) {
	void *nstack;
	void *sp;
	int pid;
	
	nstack = (void*)sbrk(PGSIZE*2);
	sp = (void*)((int)nstack +PGSIZE*2 - 3*4);
	*(int*)sp = (int)start_routine;
	*(int*)(sp+4) = (int)&exit;
	*(int*)(sp+8) = (int)arg;
	// printf(1, "addr1: %p %p\n", &arg, nstack);
	// printf(1, "%p %d\n", nstack, PGSIZE*2);
	if ((pid = clone(nstack, PGSIZE*2))) {
		// printf(1, "hello from parent\n");
		// printf(1, "id1: %d\n", arg);
		return pid;
	}

	printf(2, "never call this function\n");
	return -1;
}


// test program below

struct spinlock lock;
int volatile nround, nthread, location, passes;

void *routine(void *arg) {
	// printf(1, "addr2: %p\n", &arg);
	int token = (int)arg;
	// printf(1, "id3: %d\n", token);
	while(passes <= nround) {
		if (location == token) {
			acquire(&lock);
			// printf(1, "thread %d acquire\n", token);

			if (location != token) {
				// printf(1, "thread %d release\n", token);
				release(&lock);
				continue;
			}
			location = (location+1) % nthread;
			passes++;
			printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n", passes, token, location);
			if (passes == nround) {
				// printf(1, "thread %d release\n", token);
				release(&lock);
				break;
			}

			// printf(1, "thread %d release\n", token);
			release(&lock);
		}
	}
	return NULL;
}

int
main(int argc, char *argv[]) {
	int i;

	if (argc < 3) {
		printf(2, "too few arguments\n");
		exit();
	}

	nthread = atoi(argv[1]);
	nround = atoi(argv[2]);

	initlock(&lock);
	location = 0;
	passes = 0;

	for (i = 0; i < nthread; i++) {
		thread_create(&routine, (void*)i);
	}
	// thread_create(routine, NULL);
	// while (1)
	// 	sleep(1);
	exit();
}