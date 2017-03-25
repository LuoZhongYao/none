#include <sys/inter.h>
#include <none/scntl.h>
pid_t fork(void){
    return run(MM_PID,MIF_CLONE,0,0,0);
}
