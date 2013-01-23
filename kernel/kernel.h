#include    <const.h>
#include    <types.h>
#include    <x86/io.h>
#include    <sys/inter.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

extern void trap_init(void);
extern void mm_init(void);

/* */
extern void disable_irq(int irq);
extern void enable_irq(int irq);
extern void put_irq_handler(int irq,IrqHandler handler);

extern int interrupt(pid_t dest,Message *m_ptr);

extern void sched(void);

extern Proc *act_proc;
extern Proc *proc_ptr;
extern IrqHandler irq_table[NR_IRQ_VECTORS];

#define proc_number(rp) ((rp)->pid)
#define proc_addr(pid)  (task[pid])

extern Tss *tss;

/* */
#define getcr3()    ({  \
        Pointer _cr3;   \
        asm("movl %%cr3,%0\n\t\t":"=a"(_cr3)); \
        _cr3;   \
        })

#define ldcr3(cr3)  ({\
        asm("movl %0,%%cr3\n\t\t"::"a"(cr3));\
        })

/* Task List */
int clock_main(void);


