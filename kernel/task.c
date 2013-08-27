#include    "task.h"
#include    "kernel.h"

Task    *rdy_head[NR_PRI];  /*! 调度队列头,参考MINIX设计 !*/
Task    *rdy_tail[NR_PRI];
Task    *leading = NULL;    /*! 主角,在你的世界里,你就是主角 ~*/
Tss     *tss;               /*! 保存任务的内核态堆栈,以及IO操作允许 !*/

#define isTaskp(p)  ((p)->pri == PRI_TASK)  /*! 是任务,具有高优先级 !*/
#define isUserp(p)  ((p)->pri == PRI_USER)  /*! 普通任务,时间片调度 !*/

#define working(x)  ((x)->work)
#define isAdmitp(x)  (working(x) == ADMINTING)
#define isSleep(x)  (working(x) == SLEEPING)
#define isWait(x)  (working(x) == WAITING)
#define isAcctive(x)  (working(x) == ACTIVE)
#define setAdmit(x) (working(x) = ADMINTING)
#define setSleep(x) (working(x) = SLEEPING)
#define setWait(x) (working(x) = WAITING)
#define setActive(x)    (working(x) = ACTIVE)
#define isWaitMe(x) ((x->wait) == self())

#define TASK_GOD()  TASK(toObject(GOD))   /*! 和上帝对话,我想你大概不会很喜欢他 !*/


static void pick_task(void){
    /*! 只有基础服务得于优先运作,我们的工作才有意义 !*/
    if(rdy_head[PRI_TASK] != NULL) leading = rdy_head[PRI_TASK];    
    else if(rdy_head[PRI_USER] != NULL) leading = rdy_head[PRI_USER];
    else leading = rdy_head[PRI_GOD];       /*! 上帝只有在我们都不干了的时候才过来擦屁股,很明显,他做得很好 !*/
}

void sched(void){
    if(!isNullp(rdy_head[PRI_USER])){
        rdy_tail[PRI_USER]->next = rdy_head[PRI_USER];
        rdy_tail[PRI_USER] = rdy_head[PRI_USER];
        rdy_head[PRI_USER] = rdy_head[PRI_USER]->next;
        rdy_tail[PRI_USER]->next = NULL;
    }
    pick_task();
}

/*! 接受党和人民考验的时候到了,是时候派我上场啦?你是说我还要排队,shit !*/
/*static */void ready(Task *rt){
    if(isNullp(rt)) panic("\erReady    \eb[rTask is <null>\eb]");
    if(isNullp(rdy_head[rt->pri]))
        rdy_head[rt->pri] = rt;
    else
        rdy_tail[rt->pri]->next = rt;
    rt->next = NULL;
    rdy_tail[rt->pri] = rt;
}

static void unready(Task *rt){
    Task *xt;
    if(isNullp(rt)) panic("\erUnready    \eb[\erTask is <null>\eb]");
    if(NULL != (xt = rdy_head[rt->pri])){ 
        if(xt == rt){
            rdy_head[rt->pri] = rt->next;
        }else{
            while(xt->next != rt) 
                if(NULL == (xt = xt->next)) panic("\erUnready    \eb[\erTask is Bad\eb]");
            xt->next = xt->next->next;
            if(rdy_tail[rt->pri] == rt) rdy_tail[rt->pri] = xt;
        }
    }
    /* if(rt == leading) pick_task(); */
    sti();      /*! 开中断,你懂的 !*/
    hlt();      /*! 等待一个时钟进行任务切换,假如来的是其他中断呢? !*/
    cli();      /*! 关中断,保证之后的操作不会被打断 !*/
}

static void _admit(Object *obj,unsigned long fn,unsigned long r1,unsigned long r2,unsigned long r3){
    setAdmit(obj);
    obj->fn = fn;
    obj->r1 = r1;
    obj->r2 = r2;
    obj->r3 = r3;
    ready(TASK(obj));
}

static void _sleep(Object *obj){
    setSleep(obj);
    unready(TASK(obj));
}

static void _wait(Object *obj,Object *wait){
    setWait(obj);
    obj->wait = wait;
    unready(TASK(obj));
}


static void _wakeup(Object *obj){
    setActive(obj);
    obj->wait = NULL;
    ready(TASK(obj));
}

int dofn(ObjectDesc o,unsigned long fn,unsigned long r1,unsigned long r2,unsigned long r3){
    Object*obj = toObject(o);
    if(isNullp(obj)) return ERROR;
    /*! 目标有事务在处理,则,将本次请求挂入队列 !*/
    if(!isSleep(obj)){
        if(isNullp(obj->wlink)) obj->wlink = self();
        else obj->wtail->wnext = self();
        self()->wnext = NULL;
        obj->wtail = self();
        _wait(self(),obj);   /*! 等待对方接客结束,然后唤醒self()将任务提交 !*/
    }else{
        obj->admit = self();
    }
    _admit(obj,fn,r1,r2,r3);
    _wait(self(),obj);          /*! 任务提交了,等待对方为你服务.吹箫? !*/
    return self()->talk;   /*! 再次运行到这里已经是很久很久以前的事了 !*/
}

/*! 中断具有高优先级,必须优先处理 !*/
/*! 我很讨厌定义多个接口,接口太多太不美.但是,能有什么更好的办法呢? !*/
int doint(ObjectDesc o,unsigned long fn,unsigned long r1,unsigned long r2,unsigned long r3){
    Object *obj = toObject(o);
    if(isNullp(obj)) panic("\er doint   \eb[\rnull\eb]\n");
    if(!isSleep(obj)){
        iLink   *in = malloc(sizeof(iLink));
        if(isNullp(in)) panic("\er doint \eb[\ermemory full\eb]\n");
        in->fn = fn; in->r1 = r1;
        in->r2 = r2; in->r3 = r3;
        in->admit = self(); in->inext = NULL;
        if(!isNullp(obj->ilink)) in->inext = obj->ilink;
        obj->ilink = in;
    }else{
        _admit(obj,fn,r1,r2,r3);
    }
    return OK;
}

int doret(Object *obj,unsigned long talk){
    /*! Object *obj = toObject(o); !*/
    if(!isNullp(obj) && isWaitMe(obj)){
        obj->talk = talk;
        _wakeup(obj);
    }
    return OK;
}


int doget(void){
    if(!isNullp(self()->ilink)){
        iLink   *in = self()->ilink;
#define eval(v,x)   ((v)->x = (v)->ilink->x)
#define getInt(this)    {\
        eval(this,admit); eval(this,fn);\
        eval(this,r1); eval(this,r2);\
        eval(this,r3);\
        }
        getInt(self());
#undef  eval
#undef  getInt
        self()->ilink = in->inext;
        free(in);
        return OK;
    }else if(!isNullp(self()->wlink)){
        self()->admit = self()->wlink;
        self()->wlink = self()->wlink->wnext;
    }else{      /*! 如果当前没有请求,则进入睡眠,等待请求将其唤醒 !*/
        _sleep(self());     /*! 从这里醒来,说明请求已经装填,可以处理了 !*/
        return OK;
    }
    _wakeup(self()->admit); /*! 唤醒等待处理的对象来装填请求 !*/
    _sleep(self()); /*! 等待对象的装填完成来唤醒 !*/
    return OK;
}

static Task* make_task(id_t id,String name,Pointer data,Pointer code,int pri,int (*entry)()){
    Task *task;
    task = (Task *)get_free_page();
    OBJECT(task)->id = id;
    task->pri = pri;
    task->core = getcr3();
    task->registers = (void *)(STACK(task)->stackp - sizeof(Registers));
    strcpy(OBJECT(task)->name,name);
    memcpy(task->registers,&(Registers){
            .gs = data,
            .fs = data,
            .ds = data,
            .es = data,
            .edi = 0,
            .esi = 0,
            .ebp = 0,
            .ebx = 0,
            .edx = 0,
            .ecx = 0,
            .eax = 0,
            .eip = (long)(entry),
            .cs = code,
            .eflags = 0x200,
            },sizeof(Registers));
    object_table[id] = OBJECT(task);
    ready(task);
    return task;
}

void god_init(void){
    Pointer tr = TR_DESC;

    for(int i = 0;i < NR_PRI;i++) rdy_head[i] = rdy_tail[i] = NULL;
    leading = make_task(GOD,"God",KERNEL_DATA,KERNEL_CODE,PRI_GOD,NULL);
    leading->next =leading;

    extern int mm_main();
    make_task(MM_PID,"MM",KERNEL_DATA,KERNEL_CODE,PRI_TASK,mm_main);
#if 0
    make_task(CLOCK_PID,"Clock",KERNEL_DATA,KERNEL_CODE,PRI_TASK,clock_main);
    extern int at_main();
    make_task(AT_PID,"Hardware",KERNEL_DATA,KERNEL_CODE,PRI_TASK,at_main);
    extern int fs_main();
    make_task(FS_PID,"FS",KERNEL_DATA,KERNEL_CODE,PRI_TASK,fs_main);
    extern int cons_main();
    make_task(CONS_PID,"Conslo",KERNEL_DATA,KERNEL_CODE,PRI_TASK,cons_main);
    extern int system_main();
    make_task(SYSTEM_PID,"System",KERNEL_DATA,KERNEL_CODE,PRI_USER,system_main);
#endif

    tss = (Tss*)(TSS_TABLE);
    tss->ss0 = KERNEL_DATA;
    tss->esp0 = (Pointer)(STACK(leading)->stackp);
    //tss->gs = tss->fs = tss->es = tss->ds = USER_DATE;
    tss->ldt = 0;
    tss->io = 0xffff0000;
    tss->eflags = 0x200;
    asm("ltr %0"::"m"(tr));
    /*! 时钟中断的特殊之处,因为时钟是一切调度的开始,如果让时钟自己来初始化,kernel就无法运作了 !*/
    void clock_init(void);
    clock_init();
    /* pick_task(); */
}
