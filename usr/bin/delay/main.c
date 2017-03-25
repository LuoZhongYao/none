#include <stdio.h>
#include <none/if.h>
#include <stdlib.h>
#include <sys/inter.h>
int main(int argc,char **argv)
{
    pid_t pid = atoi(argv[1]);
    time_t time = run0(pid,IF_READ);
    printf("start : %d\n",time);
    while(1) {
        run1(pid,IF_CREATE,1000);
        time++;
        printf("time : %d S\n",time);
    }
    return 0;
}
