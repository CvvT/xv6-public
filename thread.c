//thread.c

#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"
#include "x86.h"

#define NULL 0

// SPIN LOCK
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

// Array Lock
#define ARRAY_LEN 20
static int counter = 0;

static inline int fetch_and_inc(int* variable, int value) {
  __asm__ volatile("lock; xaddl %0, %1"
    : "+r" (value), "+m" (*variable) // input+output
    : // No input-only
    : "memory"
  );
  return value;
}

void initArrayLock(struct spinlock *arraylock) {
	int i;
	arraylock[0].locked = 0;
	arraylock[0].pid = 0;
	for (i = 1; i < ARRAY_LEN; i++) {
		arraylock[i].locked = 1;
		arraylock[i].pid = 0;
	}
}

int
holding_array(struct spinlock *lock)
{
  return !lock->locked && lock->pid == getpid();
}

int array_acquire(struct spinlock *arraylock) {
	int index;
	index = fetch_and_inc(&counter, 1);
	
	while (arraylock[index % ARRAY_LEN].locked == 1);

	__sync_synchronize();

	arraylock[index % ARRAY_LEN].pid = getpid();
	return index;
}

void array_release(struct spinlock *arraylock, int index) {
	struct spinlock *next;
	if (!holding_array(&arraylock[index % ARRAY_LEN])) {
		printf(2, "release\n");
    	return;
	}

	arraylock[index % ARRAY_LEN].locked = 1;
	arraylock[index % ARRAY_LEN].pid = 0;
	// arraylock[index+1].locked = 0;
	next = &arraylock[(index + 1) % ARRAY_LEN];

	 __sync_synchronize();
	 asm volatile("movl $0, %0" : "+m" (next->locked) : );
}

// Thread Creation
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
struct spinlock arraylock[ARRAY_LEN];
int nround, nthread;
int volatile location;
int volatile passes;

void *routine(void *arg) {
	// printf(1, "addr2: %p\n", &arg);
	int token = (int)arg;
	// printf(1, "id3: %d\n", token);
	while(passes <= nround) {
		if (location == token) {
			acquire(&lock);
			// printf(1, "thread %d acquire\n", token);

			if (location != token) { // double check it's my turn
				// printf(1, "[+]thread %d release\n", token);
				release(&lock);
				continue;
			}

			if (passes > nround) {
				// printf(1, "[-]thread %d release\n", token);
				release(&lock);
				break;
			}

			location = (location+1) % nthread;
			printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n", passes, token, location);
			// printf(1, "[*]thread %d release\n", token);
			if (passes == nround) {
				printf(1, "\nSimulation of Frisbee game has finished, %d rounds were played in total!\n", nround);
			}
			passes++;
			release(&lock);
		}
	}
	return NULL;
}

void *routine_arraylock(void *arg) {
	// printf(1, "addr2: %p\n", &arg);
	int token = (int)arg;
	int index;
	// printf(1, "id3: %d\n", token);
	while(passes <= nround) {
		if (location == token) {
			index = array_acquire(arraylock);
			// printf(1, "thread %d acquire\n", token);

			if (location != token) { // double check it's my turn
				// printf(1, "[+]thread %d release\n", token);
				array_release(arraylock, index);
				continue;
			}

			if (passes > nround) {
				// printf(1, "[-]thread %d release\n", token);
				array_release(arraylock, index);
				break;
			}

			location = (location+1) % nthread;
			printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n", passes, token, location);
			// printf(1, "[*]thread %d release\n", token);
			if (passes == nround) {
				printf(1, "\nSimulation of Frisbee game has finished, %d rounds were played in total!\n", nround);
			}
			passes++;
			array_release(arraylock, index);
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
	passes = 1;

	for (i = 0; i < nthread; i++) {
		// thread_create(&routine, (void*)i);
		thread_create(&routine_arraylock, (void*)i);
	}
	// thread_create(routine, NULL);
	// while (1)
	// 	sleep(1);
	exit();
}