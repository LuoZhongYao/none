#include <sys/inter.h>
#include <none/scntl.h>

void exit(int status) {
    (void)run(MM_PID,MIF_CLOSE,status,0,0);
}
