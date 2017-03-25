#include <posix.h>
#include <sys/inter.h>
#include <none/scntl.h>

object_t getpid(void)
{
    return run(SYSTEM_PID,SIF_GETPID,0,0,0);
}
