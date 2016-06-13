#include "kernel.h"
#include "i8259.h"
#include "sys.h"
#include <none/scntl.h>

#define     INT_GATE    0x8e00
#define     TRA_GATE    0x8f00
#define     TR_GATE     0x8500
#define     IDT_R0      0x0000
#define     IDT_R1      0x2000
#define     IDT_R2      0x4000
#define     IDT_R3      0x6000


extern void 	divide_error(long,long *);
extern void 	single_step_exception(long,long *);
extern void 	nmi(long,long *);
extern void 	breakpoint_exception(long,long *);
extern void 	overflow(long,long *);
extern void 	bounds_check(long,long *);
extern void 	inval_opcode(long,long *);
extern void 	copr_not_available(long,long *);
extern void 	double_fault(long,long *);
extern void 	copr_seg_overrun(long,long *);
extern void 	inval_tss(long,long *);
extern void 	segment_not_present(long,long *);
extern void 	stack_exception(long,long *);
extern void 	general_protection(long,long *);
extern void 	page_fault(long,long *);
extern void 	copr_error(long,long *);
extern void 	none(long,long *);

extern void 	hwint00(void);
extern void 	hwint01(void);
extern void 	hwint02(void);
extern void 	hwint03(void);
extern void 	hwint04(void);
extern void 	hwint05(void);
extern void 	hwint06(void);
extern void 	hwint07(void);
extern void 	hwint08(void);
extern void 	hwint09(void);
extern void 	hwint10(void);
extern void 	hwint11(void);
extern void 	hwint12(void);
extern void 	hwint13(void);
extern void 	hwint14(void);
extern void 	hwint15(void);
extern void     switch_task(void);      
extern int      sys_call(int EAX,int EBX,int ECX,int EDX); 

#define idt  ((unsigned long (*)[2])IDT_TABLE)

object_t    irq_table[NR_IRQ_VECTORS] = {-1};

#define set_int(nr,func,section,attr) {\
    idt[nr][0] = ((((unsigned int)func)&0xffff)|((unsigned short )(section)<<16));\
    idt[nr][1] = ((((unsigned int)func)&0xffff0000)|(unsigned short)(attr));\
}


#define exit(n) panic("oop:-_-|\n");

void print_cpu_info(Registers *reg)
{
    printk("CS  = %010p EIP = %010p    ",reg->cs,reg->eip);
    printk("SS  = %010p ESP = %010p\n",reg->ss,reg->_esp);
    printk("DS  = %010p EDI = %010p    ",reg->ds,reg->edi);
    printk("ES  = %010p ESI = %010p\n",reg->es,reg->esi);
    printk("EAX = %010p ECX = %010p    ",reg->eax,reg->ecx);
    printk("EBX = %010p EDX = %010p\n",reg->ebx,reg->edx);
}

static void print_task_info(String str,long nr)
{
    printk("\er******* %s<%p:%ld> %s(%x) *******\ew\n",self()->name,self(),self()->id,str,nr);
}

static inline void die(String str,long *reg,long nr)
{
    print_task_info(str,nr);
    print_cpu_info((void*)(reg - 1));
    run(MM_PID,MIF_CLOSE,0,0,0);
    /* panic(":-("); */
}

extern void do_divide_error(long code,long *reg)
{
    die("divede error",reg,code);
}

extern void do_debug(long code,long *reg)
{
    die("debug traps",reg,code);
    printk("This debug!,But the code no ready![TRAP]\n");
}

extern void do_nmi(long code,long *reg)
{
    die("nmi error!",reg,code);
}

extern void do_breakpoint(long code,long *reg)
{
    die("breakpoint traps.",reg,code);
    printk("code no ready![TRAP]\n");
}

extern void do_overflow(long code,long *reg)
{
    die("overflow",reg,code);
}

extern void do_bounds_check(long code,long *reg)
{
    die("bounds",reg,code);
}

extern void do_inval_opcode(long code,long *reg)
{
    die("invalid operand",reg,code);
}

extern void do_copr_not_available(long code,long *reg)
{
    die("copr not available.",reg,code);
}

extern void do_double_fault(long code,long *reg)
{
    die("double fault",reg,code);
}

extern void do_copr_seg_overrun(long code,long *reg)
{
    die("copr segment overrun",reg,code);
}

extern void do_inval_tss(long code,long *reg)
{
    die("inval tss",reg,code);
}

extern void do_segment_not_present(long code,long *reg)
{
    die("segment not present",reg,code);
}

extern void do_stack_exception(long code,long *reg)
{
    die("stack segment",reg,code);
}

extern void do_general_protection(long code,long *reg)
{
    die("general protection",reg,code);
}

extern void do_page_fault(long code,long *reg)
{
    int ret;
    void* cr2 = (void*)getcr2();
    if(!(code & 0x1)){
        ret = run(MM_PID,MIF_NOPAGE,cr2,0,0);
    } else if(code & 0x2){
        ret = run(MM_PID,MIF_WPPAGE,cr2,0,0);
    }

    if(ret < 0)
        die("page fault",reg,code);
}

extern void do_copr_error(long code,long *reg)
{
    die("copr error",reg,code);
}

extern void do_none(long code,long *reg)
{
    die("what the fuck!",reg,code);
}

extern void _null(void)
{
    // die("-_-!",1);
    ;
}

extern void disable_irq(int irq)
{
    unsigned char mask;
    unsigned char ctl;
    if(NR_IRQ_VECTORS < irq || 0 > irq)
        panic("invalid irq disable_irq");

    if(irq > 7){
        mask = 1 << (irq - 8);
        ctl = INT2_CTLMASK;
    }else{
        mask = 1 << irq;
        ctl = INT_CTLMASK;
    }
    cli();
    mask |= inb(ctl);
    outb(mask,ctl);
    sti();
}

extern void enable_irq(int irq)
{
    unsigned char mask;
    unsigned char ctl;
    if(NR_IRQ_VECTORS < irq || 0 > irq)
        panic("invalid irq disable_irq");
    if(irq > 7){
        mask = ~(1 << (irq - 8));
        ctl = INT2_CTLMASK;
    }else{
        mask = ~(1 << irq);
        ctl = INT_CTLMASK;
    }
    cli();
    mask &= inb_p(ctl);
    outb_p(mask,ctl);
    sti();
}

extern void irq_handler(int irq)
{
    if(irq < 0 || irq >= NR_IRQ_VECTORS)
        panic("invalid call to spurious_irq");

    if(irq_table[irq] < 0)
        printk("Unhandle irq %d %x\n",irq);
    else
        doint(irq_table[irq],IF_INTR,irq,0,0);
}

extern int put_irq_handler(object_t o,int irq)
{
    if(irq < 0 || irq >= NR_IRQ_VECTORS)
        return -ERANGE;
    if(irq_table[irq] != SYSTEM_PID)
        return -EEXIST;
    disable_irq(irq);
    irq_table[irq] = o;
    enable_irq(irq);
    return OK;
}

extern void trap_init(void)
{
    /* install defualt int */
    for(int i = 0;i<256;i++) set_int(i,none,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0,divide_error,KERNEL_CODE,((TRA_GATE|IDT_R0)));
    set_int(1,single_step_exception,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(2,nmi,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(3,breakpoint_exception,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(4,overflow,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(5,bounds_check,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(6,inval_opcode,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(7,copr_not_available,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(8,double_fault,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(9,copr_seg_overrun,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(10,inval_tss,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(11,segment_not_present,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(12,stack_exception,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(13,general_protection,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(14,page_fault,KERNEL_CODE,(TRA_GATE|IDT_R0));
    set_int(16,copr_error,KERNEL_CODE,(TRA_GATE|IDT_R0));


    set_int(0x20,hwint00,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x21,hwint01,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x22,hwint02,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x23,hwint03,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x24,hwint04,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x25,hwint05,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x26,hwint06,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x27,hwint07,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x28,hwint08,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x29,hwint09,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x2a,hwint10,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x2b,hwint11,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x2c,hwint12,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x2d,hwint13,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x2e,hwint14,KERNEL_CODE,(INT_GATE|IDT_R0));
    set_int(0x2f,hwint15,KERNEL_CODE,(INT_GATE|IDT_R0));

    set_int(0x80,sys_call,KERNEL_CODE,(INT_GATE|IDT_R3));
    set_int(0x81,switch_task,KERNEL_CODE,(INT_GATE|IDT_R3));

    for(int i = 0;i < NR_IRQ_VECTORS;i++) 
        irq_table[i] = SYSTEM_PID;

    /* 8259a */
    outb_p(0x11,INT_CTL);
    outb_p(IRQ0_VECTOR,INT_CTLMASK);
    outb_p((1 << CASCADE_IRQ),INT_CTLMASK);
    outb_p(1,INT_CTLMASK);
    outb_p(~(1 << CASCADE_IRQ),INT_CTLMASK);

    outb_p(0x11,INT2_CTL);
    outb_p(IRQ8_VECTOR,INT2_CTLMASK);
    outb_p(CASCADE_IRQ,INT2_CTLMASK);
    outb_p(1,INT2_CTLMASK);
    outb_p(0xff,INT2_CTLMASK);
}
