//thread.c

#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

#define NULL 0

int thread_create(void *(*start_routine)(void*), void *arg) {
	void *nstack;
	int pid;

	printf(1, "%p\n", start_routine);
	nstack = (void*)sbrk(PGSIZE*2);
	printf(1, "%p %d\n", nstack, PGSIZE*2);
	if ((pid = clone(nstack, PGSIZE*2))) {
		printf(1, "hello from parent\n");
		return pid;
	} else { // deal with calling convention
		printf(1, "hello from child\n");
		// start_routine(arg);
	}
	exit();
}

void *routine(void *arg) {
	printf(1, "hello from routine\n");
	return NULL;
}

int
main(int argc, char *argv[]) {
	thread_create(routine, NULL);
	while (1)
		sleep(1);
	exit();
}