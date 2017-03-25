#include    <stdio.h>

int main(void){
    for(unsigned short i = 0;i < 16;i++){
        for(int j = 0;j < 16;j++){
            switch(i + (j << 4)){
            case 0 ... 31 : printf("    ");break;
            default  :      printf("%c  ",i + (j << 4)); break;
            }
        }
        printf("\n");
    }
    return 0;
}
