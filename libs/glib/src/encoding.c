#include <stdint.h>
#define is10(x) ((x & 0xc0) == 0x80)
#define is110(x) ((x & 0xe0) == 0xc0)
#define is1110(x) ((x & 0xf0) == 0xe0)
#define is11110(x) ((x & 0xf8) == 0xf0)
uint16_t UTF8toUnicode(uint8_t *ch,int len){
    if(*ch < 0x80)
        return *ch;
    if(is110(ch[0]) && len >= 2 && is10(ch[1])){
        return (ch[0] & 0x1f) << 6 | (ch[1] & 0x3f);
    }
    if(is1110(ch[0]) && len >= 3 && is10(ch[1]) && is10(ch[2])){
        return (ch[0] & 0x0f) << 12 | (ch[1] & 0x3f) << 6 | (ch[2] & 0x3f);
    }
#if 0
    if(is11110(ch[0]) && len >= 4 && is10(ch[1]) && is10(ch[2]) && is10(ch[3]))
        return (ch[0] & 0x7) << 18 | (ch[1] & 0x3f) << 12 | (ch[2] & 0x3f) << 6 | (ch[3] & 0x3f);
#endif

    return 0xFFFF;
}
