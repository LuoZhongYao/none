#include "pci.h"
#include <stdio.h>
#include <x86/io.h>
#include <sys/inter.h>
#include <none/scntl.h>
#include <string.h>
static void bochas_scan_pci(u32 device,u16 v,u16 d,void *extra)
{
    printf("vendor : %x device : %x\n",v,d);
    if((v == 0x1234 && d == 0x1111) || 
       (v == 0x80EE && d == 0xBEEF)) {
        uintptr_t t = pci_read_field(device, PCI_BAR0,4);
        if(t > 0)
            *((u8 **)extra) = (u8 *)(t & 0xFFFFFFF0);
    }
}

int main(void)
{
    u8 *vid = (u8 *) 0x00000000;
    run(MM_PID,MIF_DMAMAP,0xE0000000,0xE0000000,0x4096);
    outw(0x00,0x1ce);
    u16 i = inw(0x1cf);
    if(i < 0xb0c0 || i > 0xb0c6)
        printf("FLASE\n");
    outw(0xb0c4,0x1cf);
    i = inw(0x1cf);
    outw(0x04,0x1ce);
    outw(0x00,0x1cf);

    outw(0x01,0x1ce);
    outw(1024,0x1cf);

    outw(0x02,0x1ce);
    outw(768,0x1cf);

    outw(0x03,0x1ce);
    outw(32,0x1cf);

    outw(0x07,0x1ce);
    outw(4096,0x1cf);

    outw(0x04,0x1ce);
    outw(0x41,0x1cf);

    memset((void*)0xe0000000,0xff,1 << 24);
    pci_scan(bochas_scan_pci,-1,&vid);
    printf("MEMORY : %p\n",vid);
    while(1);
    return 0;
}
