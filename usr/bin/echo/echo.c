#include <stdio.h>
#include <z.h>
int main(int argc,char **argv){
    foreach(i,1,argc){
        printf("%s ",argv[i]);
    }
    printf("\n");
    return 0;
}
