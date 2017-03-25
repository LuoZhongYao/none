#include <elf.h>
#include <sys/inter.h>
#include <none/scntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <z.h>

int exec(const char *path,int argc,char **argv){
    int _v = -1;
    char cmd[125] = "/bin/";
    if(path[0] == '.' || path[0] == '/')
        strcpy(cmd,path);
    else
        strcat(cmd,path);
    object_t o = try(0 > ,open(cmd,0),throw e_fail);
    cnt_t count = argc > 31 ? 31 : argc;
    char *ptr;
    struct {
        char *argv[32];
        char env[0];
    } *buff = _push(NULL,0);
    ptr = buff->env;
    foreach(i,0,count){
        buff->argv[i] = ptr;
        strcpy(ptr,argv[i]);
        ptr += strlen(ptr) + 1;
    }
    _v = run(MM_PID,MIF_EXECVP,o,buff,argc);
    _pop(buff);
    catch(e_fail) {
        return _v;
    }
}

int execvp(const char *cmd,char **argv){
    int v = -1;
    foreach(i,0,32){
        if(argv[i] == NULL){
           v =  exec(cmd,i,argv);
           break;
        }
    }
    return v;
}
