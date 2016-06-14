/*
 *    (C) 2011-10
 *    Email:LuoZhongYao@gmail.com
 *    每天都对自己说:"Good luck!"
 *    该程序负责初始化内核工作环境,是内核执行的第一个程序.
 *    初始化内容包括,重新安排GDT(由multiboot规范知道,现在已经有一个GDT了),并且
 *    对8259进行初始化,本质上来说,该程序只是原gmL的setup去除进入保护模式和添加
 *    multiboot支持而已.
 *        使用multiboot的好处是可以使用现存的文件系统,而不用去研究它,可以被其
 *    引导器引导.内核大小没有限制.反正很多啦,所以我个人建议,有写内核的同学,必
 *    要参考multiboot,并遵守它.
 */
/*  汇编代码    */
#define ASM
/*  multiboot规范头文件,该文件直接拷贝于grub_0.98/docs/multiboot.h */
#include    <boot/multiboot.h>
#include    "const.h"

#define CreateSection(base,limit,attr) \
    .short  limit&0xffff;\
    .short  base&0xffff; \
    .byte   (base>>16)&0xff;\
    .short  (attr)|((limit>>8)&0x0f00);\
    .byte   (base>>24)&0xff;
//代码段相关属性宏定义
#define CODE_READ   0x009a      //代码段可读
#define CODE_C      0x009c      //一致代码段
#define CODE_G      0x8098      //使用4GB的界限
#define CODE_D      0x4098      //默认使用32位地址
//数据段属性
#define DATA_WRITE  0x0092      //数据段可写
#define DATA_E      0x0094      //向下扩展
#define DATA_G      0x8090      //使用32界限
#define DATA_B      0x4090      //堆栈段使用ESP等32位指针,向下扩展,指明栈上限4GB
//TSS描述符
#define TSS_G       0x8089      //TSS使用4GB界限,通常这个是用不到的
#define TSS         0x0089      //说明这是一个TSS段
//权限
#define R0          0x0000      //ring 0
#define R1          0x0020      //
#define R2          0x0040      //
#define R3          0x0060      //ring 3

GDT_LEN = gdt_end-gdt_start
.text
.globl  _start,envp,main
_start:
    jmp start
    /*  multiboot   要求4字节对齐   */
.align  4
    multiboot_header:
    /*  multiboot header magic  详情请查阅multiboot */
    .long   MULTIBOOT_HEADER_MAGIC
    /*  multiboot header flags multiboot为你做什么服务,色情服务不在这个范围内*/
    /*  同时也告诉了multiboot你是什么款式的人,oh,程序   */
    .long   MULTIBOOT_HEADER_FLAGS
    /*  multiboot   header checksum ,详情请看multiboot  */
    /* 我猜测引导器就是靠checksum和magic来识别multiboot header的. */
    .long   -(MULTIBOOT_HEADER_MAGIC+MULTIBOOT_HEADER_FLAGS)
    /*  如果不是ELF文件格式,下面的信息就是必须的,这个__ELF__不知道是在哪里定义的 */
#undef __ELF__
#ifndef __ELF__
    /* multiboot header addr,hi哥们,你在哪? */
    .long   multiboot_header
    /* Load addr .text在OSimage中的偏移 */
    .long   _start 
    /* Load end addr 引导到OSimage中那个位置,如果为零,引导整个OS Image */
    .long   _edata
    /*  bss段的结束地址.Boot loader将其清零,并且保证不被覆盖 */
    .long   _end
    /*  Boot loader 引导完后将跳转到这里执行 */
    .long   _start
#endif
start:
    cli
    /* 保存由Boot loader提供的信息地址 */
    mov     %ebx,envp

    mov     %cr0,%eax
    or      $0x22,%eax
    mov     %eax,%cr0
    fninit

    /* 重新设置GDT,GDT被放置到GDT_TABLE处 */
    mov     $GDT_TABLE,%edi
    mov     $gdt_start,%esi
    mov     $GDT_LEN,%ecx
    shr     $2,%ecx
    cld
    rep     movsl
    lgdt    gdt
    lidt    idt
    /* 刷新环境 */
    ljmp    $0x8,$new_env
new_env:
    mov     $0x10,%eax
    mov     %eax,%ds
    mov     %eax,%es
    mov     %eax,%ss
    mov     %eax,%fs
    mov     %eax,%gs
    mov     $KERNEL_STACK,%esp  
    call    main   
    sti
0:
#hlt
    jmp     0b

.data
envp:
    .long   0x0
gdt:
    .word   0xFFFF
    .long   GDT_TABLE
idt:
    .word   0x7FF
    .long   IDT_TABLE
gdt_start:
    CreateSection(0x0,0xFFFFF,DATA_WRITE|R0|DATA_G);
    CreateSection(0x0,0xFFFFF,CODE_READ |R0|CODE_G|CODE_D);
    CreateSection(0x0,0xFFFFF,DATA_WRITE|R0|DATA_G|DATA_B);
    CreateSection(0x0,0xFFFFF,CODE_READ |R3|CODE_G|CODE_D);
    CreateSection(0x0,0xFFFFF,DATA_WRITE|R3|DATA_G|DATA_B);
    CreateSection(0x0,0xFFFFF,CODE_READ |R0|CODE_G);
    CreateSection(0x0,0xFFFFF,DATA_WRITE|R0|DATA_G);
    CreateSection(TSS_TABLE,0x100,TSS|R3);
gdt_end:
