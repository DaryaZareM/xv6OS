#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "childrenArray.h"
#include "timeElem.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;

  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->priority = 3;

  p->retime = 0;
  p->rutime = 0;
  p->stime = 0;
  p->etime = 0;
  p->ctime = ticks;
  
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
 // p->ctime = ticks; //TODO unuseable
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S
//  p->priority = 3; //TODO UNUSEable
  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
  // set number of called sysCalls to zero
  for(int i=0; i < 50; i++){
    p->syscallCount[i] = 0; 
  }
  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}


//TODO
//test 
int
printTimeStatus(struct proc *p){
  cprintf("\nprocess %d terminated.\n creation time: %d termination time: %d\nready time: %d running time: %d sleeping time: %d\n",p->pid ,p->ctime,p->etime,p->retime,p->rutime,p->stime);
  return 0;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  curproc->etime = ticks;
  //printTimeStatus();


  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}
// Exit the current process.  Does not return. and print time
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exitT(struct timeElem *te)
{
  struct proc *curproc = myproc();


  curproc->etime = ticks;

  te->creationTime = curproc->ctime;
  te->ExitTime = curproc->etime;
  te->readyTime = curproc->retime;
  te->runningTime = curproc->rutime;
  te->waitTime = curproc->stime;


  //printTimeStatus(curproc);


}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->ctime = 0;
        p->etime = 0;
        p->rutime = 0;
        p->retime = 0;
        p->stime = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  int winner = 0;
  //int winner = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      struct proc *highP = 0;
      struct proc *p1 = 0;
      
      if(p->state != RUNNABLE)
        continue;

      // Multi Layered Scheduling
      int count_1 = 0;
      int count_2 = 0;
      int count_3 = 0;
      int count_4 = 0;
      struct proc *layer_1_p = p;
      //struct proc *layer_2_p = p;
      //struct proc *layer_3_p;
      struct proc *layer_4_p = p;
      struct proc *highP_layer_2 = p;
      struct proc *highP_layer_3 = p;

      if (mycpu()->schedulePolicy == MULTI_LAYERED_POLICY){
        
        for(p1 = ptable.proc; p1 < &ptable.proc[NPROC]; p1++){
          if((p1->state == RUNNABLE) && p1->queueLayer == 1){
            p = p1;
            count_1 += 1;
            break;
          }else if((p1->state == RUNNABLE) && p1->queueLayer == 2){
            if(count_2 == 0){
              highP_layer_2 = p1;
            }else{
              if(p1->priority > highP_layer_2->priority){
                highP_layer_2 = p1;
              }
            }
            count_2 += 1;
          }else if((p1->state == RUNNABLE) && p1->queueLayer == 3){
            if(count_3 == 0){
              highP_layer_3 = p1;
            }else{
              if(p1->priority < highP_layer_3->priority){
                highP_layer_3 = p1;
              }
            }
            count_3 += 1;
          }else if((p1->state == RUNNABLE) && p1->queueLayer == 4){
            layer_4_p = p1;
            count_4 += 1;
          }
         }

        // 1st queue == default
        if(count_1 != 0){
          p = layer_1_p;
          winner = 1;
        // 2st queue == priority
        }else if(count_2 != 0){
          p = highP_layer_2;
          winner = 2;
        // 3st queue == reverse priority
        }else if(count_3 != 0){
          p = highP_layer_3;
          winner = 3;
        // 4st queue == Round Robin
        }else if(count_4 != 0){
          p = layer_4_p;
          winner = 4;
        }
       
       }else if(mycpu()->schedulePolicy == PRIORITY_SCHEDULE_POLICY){
        // Choose the process with highest priority (among RUNNABLEs)
        highP = p;
        for(p1 = ptable.proc; p1 < &ptable.proc[NPROC]; p1++){
          if((p1->state == RUNNABLE) && (highP->priority < p1->priority))
            highP = p1;
        }
        if(highP != 0)
          p = highP;
      }
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->multiLayeredWinner = winner;
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}
void
schedulerpriority(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      struct proc *highP = 0;
      struct proc *p1 = 0;

      if(p->state != RUNNABLE)
        continue;

      // Choose the process with highest priority (among RUNNABLEs)
      highP = p;
      for(p1 = ptable.proc; p1 < &ptable.proc[NPROC]; p1++){
        if((p1->state == RUNNABLE) && (highP->priority < p1->priority))
          highP = p1;
      }
      if(highP != 0)
        p = highP;
  
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}


// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

/*
  This method will run every clock tick and update the statistic fields for each proc
*/
void updatestatistics() {
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    switch(p->state) {
      case SLEEPING:
        p->stime++;
        break;
      case RUNNABLE:
        p->retime++;
        break;
      case RUNNING:
        p->rutime++;
        break;
      default:
        ;
    }
  }
  release(&ptable.lock);
}

int
getParentID(){
  int  parentPid;
  struct proc *curproc = myproc();
  parentPid = curproc->parent->pid;
  return parentPid;

}
int
getChildren(int* childs){
    
    struct proc *curproc = myproc();
    struct proc *p;
    int count = 0;
    int currentPID;
    currentPID = curproc->pid;

    acquire(&ptable.lock);

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent->pid == currentPID){
        childs[count] = p->pid;
        count++;
      }
    }

    release(&ptable.lock);

    
    return 1;
}


int 
getSyscallCounter(int sysID){
  int sysCount;
  struct proc *curproc = myproc();
  sysCount = curproc->syscallCount[sysID];
  return sysCount;
}

// Change Process priority
int
setPriority(int priority)
{
  struct proc *curproc = myproc();

  if (priority>0 && priority<7)
    curproc->priority = priority;
  else
  {
    curproc->priority = 5;
  }
  return priority;
}

int
changePolicy(int policy){
  //struct cpu *c = mycpu();
  pushcli();
  mycpu()->schedulePolicy = policy;
  int pol = mycpu()->schedulePolicy;
  popcli();
  return pol;
}

int
setQueueLayer(int layer){
  struct proc *curproc = myproc();
  curproc->queueLayer = layer;
  return 0;
}

//find if higher priority layer proc is available in multi layered scheduling
int
findHigherPriorityProc(void){
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->queueLayer < myproc()->queueLayer){
      release(&ptable.lock);
      return 1;
    }
  }
  release(&ptable.lock);
  return 0;
}

int
totalTime(int parentpid){
  struct proc *p;
  int count[7]={0};
  int cbt[7]={0};
  int tt[7]={0};
  int wt[7]={0};
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent->pid == parentpid){
      cbt[p->priority]+= p->rutime;
      cbt[0] += p->rutime;
      tt[p->priority]+= p->etime-p->ctime;
      tt[0] += p->etime-p->ctime;
      wt[p->priority]+= p->stime;
      wt[0] += p->stime;
      count[p->priority]+=1;
      count[0]+=1;
    }
  }
  release(&ptable.lock);

  for(int i=0; i<7;i++){
    if(i==0){
      cprintf("total average: cbt=%d  TT=%d WT=%d \n",cbt[i]/count[i],tt[i]/count[i],wt[i]/count[i]);
    }
    else{
      cprintf("Priority Queue [%d] average: cbt=%d  TT=%d  WT=%d\n",i,cbt[i]/count[i],tt[i]/count[i],wt[i]/count[i]);
    }
  }
  return 0;
}
int
leyerAve(int parentpid){
  struct proc *p;
  int count[5]={0};
  int cbt[5]={0};
  int tt[5]={0};
  int wt[5]={0};
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent->pid == parentpid){
      cbt[p->queueLayer]+= p->rutime;
      cbt[0] += p->rutime;
      tt[p->queueLayer]+= p->etime-p->ctime;
      tt[0] += p->etime-p->ctime;
      wt[p->queueLayer]+= p->stime;
      wt[0] += p->stime;
      count[p->queueLayer]+=1;
      count[0]+=1;
    }
  }
  release(&ptable.lock);

  for(int i=0; i<5;i++){
    if(i==0){
      cprintf("total average: cbt=%d  TT=%d WT=%d \n",cbt[i]/count[i],tt[i]/count[i],wt[i]/count[i]);
    }
    else{
      cprintf("Leyer Queue [%d] average: cbt=%d  TT=%d  WT=%d\n",i,cbt[i]/count[i],tt[i]/count[i],wt[i]/count[i]);
    }
  }
  return 0;
}

int
rrAve(int parentpid){
  struct proc *p;
  int count=0;
  int cbt=0;
  int tt=0;
  int wt=0;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent->pid == parentpid){
      cbt += p->rutime;
      tt += p->etime-p->ctime;
      wt += p->stime;
      count +=1;
    }
  }
  release(&ptable.lock);
  
      cprintf("total average: cbt=%d  TT=%d WT=%d \n",cbt/count,tt/count,wt/count);
  return 0;
}