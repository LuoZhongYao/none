.globl _start,exit,main,errno
.text
_start:
    and   $0xfffffff0,%esp
    pushl %eax
    pushl %ecx
    call  main
    add   $0x08,%esp
    push  %eax
    call exit
.data
errno:
    .long 0
