/*
 */
#include <stdarg.h>
#include <string.h>
/* 数值转换成以base为基数的字符串,保存在str中 */
typedef enum _HexBase{
    OCTAL   = 8,
    DECIMAL =   10,
    HEX     =   16
}HexBase;

typedef enum{
    _false = 0,
    _true = 1,
}_bool;
//输出样式 
// <space>|<sign>|<special>|<zero>|num|<space>
#define STYLE_LEFT      1                       //左对齐
#define STYLE_SIGN      ((STYLE_LEFT)<<1)       //负数时显示'-'号,正数时显示'+'
#define STYLE_SPECIAL   ((STYLE_LEFT)<<2)       //显示进制前缀
#define STYLE_ZEROPAD   ((STYLE_LEFT)<<3)       //用零填充不足的位宽
#define STYLE_LARGE     ((STYLE_LEFT)<<4)       //使用大写字母

//数字字符表
static const char * const _lowerDigits = "0123456789abcdefx";
static const char * const _upperDigits = "0123456789ABCDEFX";

#define isDigit(c) ((c) >= '0' && (c) <= '9')
//
static inline int _atoi(const char **s){
    int i = 0;
    while(isDigit(**s)) i = i * 10 + *((*s)++) - '0';
    return i;
}
//进制转换
static inline int _octal(int *str,unsigned long value){
    int i = 0;
    do{
        *str++ = value & 7;
        value >>= 3;
        i++;
    }while(value);
    return i;
}
static inline int _decimal(int *str,unsigned long value){
    int i = 0;
    do{
        *str ++ = value % 10;
        value /= 10;
        i++;
    }while(value);
    return i;
}
static inline int _hex(int *str,unsigned long value){
    int i = 0;
    do{
        *str++ = value & 0xf;
        value >>= 4;
        i++;
    }while(value);
    return i;
}

static inline char *_toNumber(char *str,long long value,_bool sign,
        HexBase base,int size,int style,int mask){
    char signString = '+';
    const char *dig = _lowerDigits;
    int tmp[64];       //缓存转换后的值.还不是ASCII 
    int length = 0;

    if(style & STYLE_LARGE) dig = _upperDigits;
    if(style & STYLE_LEFT) style &= (~STYLE_ZEROPAD); //左对齐就不能使用0填充
    //如果不是10进制,则都按无符号处理 ,并且无正负符号显示
    //如果是10进制,则去除前缀属性
    if(DECIMAL != base){
        sign = _false;
        style &= ~STYLE_SIGN;
    } else {
        style &= ~STYLE_SPECIAL;
    }

    if(style & STYLE_SPECIAL){
        if(HEX == base) size -= 2;
        else if(OCTAL == base) size -= 1;
    }

    //如果是有符号数,却小于0,则取其补码,并设置符号标志,非16进制在前面已经去除
    //符号,所以不会受影响
    if((_true == sign) && ((long long)value) < 0){
            style |= STYLE_SIGN;
            value = ~value + 1;
            signString = '-';
    }

    value &= mask;
    if(style & STYLE_SIGN) size --;

    switch(base){
        case OCTAL:length = _octal(tmp,value);break;
        case DECIMAL:length = _decimal(tmp,value);break;
        case HEX:length = _hex(tmp,value);break;
    } 
    size -= length;

    if(!(style & (STYLE_ZEROPAD|STYLE_LEFT))) while(size-- > 0) *str++ = ' ';
    if(style & STYLE_SIGN) *str++ = signString;
    if(style & STYLE_SPECIAL){
        *str++ = '0';
        if(HEX == base) *str++ = dig[0x10];
    }
    if(style & STYLE_ZEROPAD) while(size-- > 0) *str++ = '0';
    while(length-- > 0) *str++ = dig[tmp[length]];
    if(style & STYLE_LEFT) while(size-- > 0) *str++ = ' ';

    return str;
}

static char *_toString(char *t,char *f,int size,int style){
    int sl = strlen(f);
    char *p = t,*q = t;
    if(style & STYLE_LEFT)
        p = t + sl;
    else 
        q = t + sl;

    memcpy(p,f,sl);
    for(int i = 0;i < size - sl;i++) *q++ = ' ';
    return t + (sl > size ? sl : size);
}

extern int vsprintf(char *buf,const char *fmt,va_list args){
    int style = 0;
    int width = 0;
    long num = 0;
    _bool   sign = _true;
    HexBase base = DECIMAL;
    char *str,*s;
    
    enum type{ _long = 'L', _int = 'l' , _short = 'h' , _char = 'H',_string = 's'}_type = _int;  //数据类型,long long,int,short

    for( str = buf; *fmt; fmt++){
        if(*fmt == '%'){
            style = 0;
            sign = _false;
            width = 0;
            _type = _int;
repeat:
            fmt++;
            switch(* fmt){
            case '-':   style |= STYLE_LEFT;goto repeat;
            case '+':   style |= STYLE_SIGN;goto repeat;
            case ' ':   style |= STYLE_SIGN;goto repeat;
            case '0':   style |= STYLE_ZEROPAD;goto repeat;
            case '#':   style |= STYLE_SPECIAL;goto repeat;
            }
            if(isDigit(*fmt))
                width = _atoi(&fmt);
            else if(* fmt == '*'){
                fmt++;
                width = va_arg(args,int);
            }

            if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'H'){
                _type = (enum type)(*fmt);
                fmt++;
            }


            switch(*fmt){
            case 'c': *str++ = (unsigned char)va_arg(args,int);continue;
            case 's': _type = _string;break;
            case 'p': style |= STYLE_SPECIAL; base = HEX; break;
            case 'X': style |= STYLE_LARGE;
            case 'x': base = HEX;break;
            case 'd': base = DECIMAL;
            case 'i': sign = _true;
            case 'u': break;
            case 'o': base = OCTAL;break;
            default: if(*fmt) *str++ = *fmt; continue;
            }
            int mask = -1;
            switch(_type){
            case _char:  mask = 0xff;
            case _short: mask &= 0xffff;
            case _int:   mask &= 0xffffffff;
                num = va_arg(args,int);
                str = _toNumber(str,num, sign,base,width,style,mask); 
                break;
            case _long:   
                num = va_arg(args,long long);
                str = _toNumber(str,num, sign,base,width,style,mask); 
                break;
            case _string: 
                s = va_arg(args,char *);
                if(!s) s = "<NULL>";
                str = _toString(str,s,width,style);
                break;
            }
        }
        else
            *str++ = *fmt;
    }
    *str = 0;

    return (str - buf);
}

extern int sprintf(char *buf,const char * fmt,...){
    va_list args;
    int n;
    va_start(args,fmt);
    n = vsprintf(buf,fmt,args);
    va_end(args);
    return n;
}
