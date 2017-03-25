#include <stdio.h>
#include <sys/inter.h>
#define VBE_DISP_INDEX  0x1ce
#define VBE_DISP_DATA   0x1cf

#define inv(a)  ({\
    outw(a,VBE_DISP_INDEX);\
    inw(VBE_DISP_DATA);})

#define outv(v,a) ({\
    outw(a,VBE_DISP_INDEX);\
    outw(v,VBE_DISP_DATA);})


int main(void)
{
    u16 value,port;
    port = 0x00;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x01;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x02;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x03;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x04;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x05;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x06;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x07;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x08;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x09;
    value = inv(port);
    printf("%x : %x\n",port,value);

    port = 0x0a;
    value = inv(port);
    printf("%x : %x\n",port,value);
    return 0;
}
