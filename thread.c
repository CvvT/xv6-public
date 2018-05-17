//thread.c

#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

int thread_create(void *(*start_routine)(void*), void *arg) {
	void *nstack;
	int pid;

	nstack = (void*)sbrk(PGSIZE*2);
	if ((pid = clone(nstack, PGSIZE*2))) {
		return pid;
	} else { // deal with calling convention
		start_routine(arg);
	}
	exit();
}

int
main(int argc, char *argv[]) {
	return 0;
}