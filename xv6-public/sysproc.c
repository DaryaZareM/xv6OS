#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "childrenArray.h"
#include "timeElem.h"

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

int sys_getParentID(void)
{
  return getParentID();
}

int
sys_getChildren(void){
  int* chs;
  if(argptr(0, (void *)&chs,sizeof(*chs)) < 0)
    return -1;
  return getChildren(chs);
}

int 
sys_getSyscallCounter(void)
{
  int sysCallID;
  argint(0, &sysCallID);
  return getSyscallCounter(sysCallID);
}

int
sys_setPriority(void){
  int priority;
  argint(0, &priority);
  return setPriority(priority);

}

int 
sys_changePolicy(void)
{
  int policy;
  argint(0, &policy);
  return changePolicy(policy);
}

int
sys_exitT(void)
{
  struct timeElem *t;
  if(argptr(0, (char **)&t, sizeof(*t)) < 0)
    return -1;
  
  exitT(t);
  return 0;
}

int 
sys_setQueueLayer(void)
{
  int layer;
  argint(0, &layer);
  return setQueueLayer(layer);
}