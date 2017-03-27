

#
#   
#   X86硬件相关处理包括x86中断,时钟中断,硬件中断等入口,系统调用入口也在该文件处理
#   
#
#include "const.h"
#include "i8259.h"
#include "asm-offsets.h"

.globl	divide_error               # 除法出错
.globl	single_step_exception      # 单步异常
.globl	nmi                        # nmi
.globl	breakpoint_exception       # 断点
.globl	overflow                   # 溢出
.globl	bounds_check               # 边界检测
.globl	inval_opcode               # 无效操作码
.globl	copr_not_available         # 协处理器无效
.globl	double_fault               # 双重错误
.globl	copr_seg_overrun           # 协处理器段越界
.globl	inval_tss                  # 无效的任务段
.globl	segment_not_present        # 段不存在
.globl	stack_exception            # 栈异常
.globl	general_protection         # 常规错误
.globl	page_fault                 # 页异常
.globl	copr_error                 # 协处理器错误
.globl  none
.globl  sys_call
.globl  door                          #供内核调方法使用,上层软件也应该自己由一个这样的函数

.globl  _null
.globl  do_keybord
.globl  do_call
.globl  sched
.globl  doloop
.globl  switch_task
.globl  irq_handler
.globl  clock_handler

.globl hwint00 , hwint01 , hwint02 , hwint03 ,  hwint04 ,  hwint05 ,  hwint06 ,  hwint07 
.globl hwint08 , hwint09 , hwint10 , hwint11 ,  hwint12 ,  hwint13 ,  hwint14 ,  hwint15

.globl  jiffies
.globl  leading

.globl  NR_syscalls,sys_call_table
.globl  irq_table,irq_object_table


.macro sched_task
    movl leading,%ebx
    movl %esp,registers(%ebx)
    call sched
    movl leading,%ebx
    movl registers(%ebx),%esp
    movl core(%ebx),%eax
    movl %eax,%cr3
.endm

.macro save
    pushal   
    pushl   %es
    pushl   %ds
    pushl   %fs
    pushl   %gs
    movl    $0x10,%eax
    movl    %eax,%ds
    movl    %eax,%es
    movl    %eax,%fs
    movl    %eax,%gs
.endm

.macro recov
    popl    %gs
    popl    %fs
    popl    %ds
    popl    %es
    popal
.endm

.text
# 除法出错处理
divide_error:
    push    $0                  #人工设置错误号
    pushl   $do_divide_error
error_code:
    xchgl   %eax,4(%esp)        #保存%eax,并将错误号放入%eax
    xchgl   %ecx,(%esp)         #保存%ecx,并将处理函数指针放入%ecx
    pushl   %edx
    pushl   %ebx
    pushl   %ebp
    pushl   %esi
    pushl   %edi
    pushl   %es
    pushl   %ds
    pushl   %fs
    pushl   %gs
    pushl   %esp
    pushl   %eax
    movl    $0x10,%eax
    movw    %ax,%ds
    movw    %ax,%es
    movw    %ax,%fs
    call    *%ecx
    addl    $8,%esp
    popl    %gs
    popl    %fs
    popl    %ds
    popl    %es
    popl    %edi
    popl    %esi
    popl    %ebp
    popl    %ebx
    popl    %edx
    popl    %ecx
    popl    %eax
    iret

# 调试中断
single_step_exception:
    pushl   $0
    pushl   $do_debug
    jmp     error_code

#_nmi
nmi:
    pushl   $0
    pushl   $do_nmi
    jmp     error_code

breakpoint_exception:
    pushl   $0
    pushl   $do_breakpoint
    jmp     error_code

overflow:
    pushl   $0
    pushl   $do_overflow
    jmp     error_code

bounds_check:
    pushl   $0
    push    $do_bounds_check
    jmp     error_code

inval_opcode:
    pushl   $0
    pushl   $do_inval_opcode
    jmp     error_code

copr_not_available:
    pushl   $0
    pushl   $do_copr_not_available
    jmp     error_code

double_fault:
    pushl   $do_double_fault
    jmp     error_code

copr_seg_overrun:
    pushl   $0
    pushl   $do_copr_seg_overrun
    jmp     error_code

inval_tss:
    pushl   $do_inval_tss
    jmp     error_code

segment_not_present:
    pushl   $do_segment_not_present
    jmp     error_code

stack_exception:
    pushl   $do_stack_exception
    jmp     error_code

general_protection:
    pushl   $do_general_protection
    jmp     error_code

page_fault:
    pushl   $do_page_fault
    jmp     error_code

copr_error:
    pushl   $0
    pushl   $do_copr_error
    jmp     error_code

none:
    pushl   $0
    pushl   $do_none
    jmp     error_code

# hwint01-07
# 
#define  hwint_master(irq) \
    save;                        \
    inb     $INT_CTLMASK,%al;   \
    orb     $(1<<(irq)),%al;       \
    outb    %al,$INT_CTLMASK;    \
    movb    $0x20,%al;   \
    outb    %al,$INT_CTL; \
    push    $irq;   \
    call    irq_handler;    \
    addl    $4,%esp;    \
    inb     $INT_CTLMASK,%al;    \
    andb    $(~(1<<(irq))),%al; \
    outb    %al,$INT_CTLMASK;    \
    recov;   \
    iret;    

switch_task:
    save;
    sched_task;
    recov;   
    iret
hwint00:
    save;                        
    inb     $INT_CTLMASK,%al;   
    orb     $1,%al;       
    outb    %al,$INT_CTLMASK;    
    movb    $0x20,%al;   
    outb    %al,$INT_CTL; 

    incl    jiffies

## if(leading->count) leading->ucount--;
    movl    leading,%ebx
    movl    ucount(%ebx),%eax
    testl   %eax,%eax
    je      0f
    decl    ucount(%ebx)
0:
##  if(!(jiffies & 0x3ff))  call irq_handler
    movl    jiffies,%eax
    andl    $0x3ff,%eax
    je      0f

    pushl   $0
    call    irq_handler
    addl    $4,%esp
0:
    sched_task;
    inb     $INT_CTLMASK,%al;    
    andb    $(~1),%al; 
    outb    %al,$INT_CTLMASK;    
    recov;   
    iret;    

hwint01:
    hwint_master(1);
hwint02:
    hwint_master(2);
hwint03:
    hwint_master(3);
hwint04:
    hwint_master(4);
hwint05:
    hwint_master(5);
hwint06:
    hwint_master(6);
hwint07:
    hwint_master(7);
# hwinnt09-15
#define  hwint_slave( irq) \
    save;    \
    inb     $INT2_CTLMASK,%al;   \
    orb     $(1<<(irq-8)),%al; \
    outb    %al,$INT2_CTLMASK;  \
    movb    $0x20,%al;   \
    outb    %al,$INT_CTL; \
    jmp     .+2;     \
    outb    %al,$INT2_CTL;    \
    pushl   $irq;   \
    call    irq_handler;  \
    addl    $4,%esp;\
    inb     $INT2_CTLMASK,%al;   \
    andb    $(~(1<<(irq-8))),%al;   \
    outb    %al,$INT2_CTLMASK;   \
    recov;   \
    iret;    
hwint08:
    hwint_slave(8);
hwint09:
    hwint_slave(9);
hwint10:
    hwint_slave(10);
hwint11:
    hwint_slave(11);
hwint12:
    hwint_slave(12);
hwint13:
    hwint_slave(13);
hwint14:
    hwint_slave(14);
hwint15:
    hwint_slave(15);

bad_sys_call:
    movl    $-1,%eax
    sti
    iret

method:
    pushl   %esi
    pushl   %edi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    call    *%eax

    addl    $20,%esp
    movl    $0x0,%eax
    int     $0x80

_workloop:
    pushl   %ds
    pushl   %es
    movl    $KERNEL_DATA,%eax
    movl    %eax,%ds
    movl    %eax,%es

    call    doloop

    popl    %es
    popl    %ds
    addl    $4,%esp
    pushl   $method
    sti
    iret

# eax = function args = {ebx,ecx,edx,edi,esi}
sys_call:
    cli
    cmpl    NR_syscalls,%eax
    jae     bad_sys_call
    cmpl    $0,%eax
    je      _workloop

    pushl   %ds
    pushl   %es
    pushl   %fs
    pushl   %esi
    pushl   %edi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    pushl   %eax
    movl    $KERNEL_DATA,%eax
    movl    %eax,%ds
    movl    %eax,%es
    movl    %eax,%fs
    popl    %eax

    call    *sys_call_table(,%eax,4)

ret_from_sys_call:
    popl    %ebx
    popl    %ecx
    popl    %edx
    popl    %edi
    popl    %esi
    popl    %fs
    popl    %es
    popl    %ds
    sti
    iret
