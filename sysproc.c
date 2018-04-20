#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//My implementation of SYSTEM CALL
#define SYSINFO_PROCESS 1
#define SYSINFO_SYSCALL 2
#define SYSINFO_PAGE    3

int
sys_info(void) {
  int n;
  int ret = -1;

  if(argint(0, &n) < 0)
    return -1;

  switch (n) {
    case SYSINFO_PROCESS:
      ret = countproc();
      break;
    case SYSINFO_SYSCALL:
      ret = countsyscall();
      break;
    case SYSINFO_PAGE:
      ret = nmpage();
      break;
    default:
      break;
  }

  return ret;
}

int
sys_priority(void) {
  struct proc *p = myproc();
  int ticket;

  if (argint(0, &ticket) < 0)
    return -1;

  set_priority(p, ticket);
  return 0;
}