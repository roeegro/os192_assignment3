// Per-CPU state
#define MAX_TOTAL_PAGES 32
#define MAX_PSYC_PAGES 16
#define INIT_SHELL_MAX_PID 2

struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

struct pageInfo {
    uint vAddress; //Virtual address
    uint fileOffset; // offset in the swap file
    int inFile; //boolean. 1- in swap file. otherwise 0
    int allocated; //boolean. 1- allocated in the memory. otherwise 0 and we will need to send pageFault.
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

  //Swap file. must initiate with create swap file
  struct file *swapFile;      //page file

  //Our fields
  struct pageInfo pagesInfo[MAX_TOTAL_PAGES]; //Data structure which will hold the pages of the process.
  uint numberOfPagedOut; //Number of pages in swap file
  int swapFileOffset; //Next place we can write in fileOffset
  int numberOfAllocatedPages;                    // the total allocated pages
  /*
   *   uint fileOffset;                           // next place to write in the file


  int numberOfPageFaults;                        // the number of times a page fault has occurred
  int totalNumberOfPagedOut;                    // the number of times a page was moved to swap file
  int inRAMQueue[MAX_PSYC_PAGES];
  int availableOffsetQueue[MAX_PSYC_PAGES];
   */
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
