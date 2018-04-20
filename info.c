#include "types.h"
#include "stat.h"
#include "user.h"

//My implementation of SYSTEM CALL
#define SYSINFO_PROCESS 1
#define SYSINFO_SYSCALL 2
#define SYSINFO_PAGE    3

int main(int argc, char *argv[]) {
	int nproc = info(SYSINFO_PROCESS);
	int nsyscall = info(SYSINFO_SYSCALL);
	int nmpage = info(SYSINFO_PAGE);
	printf(1, "Number of processes: %d\n", nproc);
	printf(1, "Number of system call: %d\n", nsyscall);
	printf(1, "Number of memory page: %d\n", nmpage);
	exit();
}