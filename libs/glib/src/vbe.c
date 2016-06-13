#include <stdio.h>
#include <x86/io.h>
#include <sys/inter.h>
#include <none/scntl.h>
#include "vbe.h"
#include <string.h>

void graphics_uninstall_bochs(void)
{
    outw(0x00,0x1ce);
    outw(0xb0c5,0x1cf);

    /* Disable VBE */
    outw(0x04,0x1ce);
    outw(0x00,0x1cf);
    /* Set X resolution */
    outw(0x01,0x1ce);
    outw(0x280,0x1cf);
    /* Set Y resolution */
    outw(0x02,0x1ce);
    outw(0x1e0,0x1cf);
    /* Set bpp to 8 */
    outw(0x03,0x1ce);
    outw(8,0x1cf);
    /* Set virtual height to stuff */
    outw(0x07,0x1ce);
    outw(1e0,0x1cf);
}

void graphics_install_bochs(u16 x,u16 y)
{
    u16 i;
    outw(0x0000,0x1ce);
    i = inw(0x1cf);
    if(i < 0xb0c0 || i > 0xb0c6)
        return;
    outw(0xb0c4,0x1cf);
    i = inw(0x1cf);

    /* Disable VBE */
    outw(0x04,0x1ce);
    outw(0x00,0x1cf);
    /* Set X resolution */
    outw(0x01,0x1ce);
    outw(x,0x1cf);
    /* Set Y resolution */
    outw(0x02,0x1ce);
    outw(y,0x1cf);
    /* Set bpp to 24 */
    outw(0x03,0x1ce);
    outw(24,0x1cf);
    /* Set virtual height to stuff */
    outw(0x07,0x1ce);
    outw(4096,0x1cf);
    /* Re-enable VBE */
    outw(0x04,0x1ce);
    outw(0x41,0x1cf);
    run(MM_PID,MIF_DMAMAP,0xE0000000,0xE0000000,4096);
}
