#include    "kernel.h"

#ifdef  PRINT_SCHED
    #define LOGD(fmt,...)   DBG("task",fmt,##__VA_ARGS__)
#endif

static Task    *rdy_head[NR_PRI];  /*! 调度队列头,参考MINIX设计 !*/
static Task    *rdy_tail[NR_PRI];
       Task    *leading = NULL;    /*! 主角,在你的世界里,你就是主角 ~*/
static Tss     *tss;               /*! 保存任务的内核态堆栈,以及IO操作允许 !*/

#define isTaskp(p)  ((p)->pri == PRI_TASK)  /*! 是任务,具有高优先级 !*/
#define isUserp(p)  ((p)->pri == PRI_USER)  /*! 普通任务,时间片调度 !*/

#define working(x)  ((x)->work)
#define isSleep(x)  (working(x) == SLEEPING)
#define isWait(x)  (working(x) == WAITING)
#define setSleep(x) (working(x) = SLEEPING)
#define setWait(x) (working(x) = WAITING)
#define setActive(x)    (working(x) = ACTIVE)
#define isWaitMe(x) ((x->wait) == self())

#define TASK_GOD()  TASK(toObject(GOD))   /*! 和上帝对话,我想你大概不会很喜欢他 !*/

static void pick_task(void)
{
    /*! 只有基础服务得于优先运作,我们的工作才有意义 !*/

#ifdef  PRINT_SCHED
    Object *oo = self();
#endif

    if(rdy_head[PRI_TASK] != NULL)
        leading = rdy_head[PRI_TASK];
    else if(rdy_head[PRI_USER] != NULL)
        leading = rdy_head[PRI_USER];
    else
        leading = rdy_head[PRI_GOD];       /*! 上帝只有在我们都不干了的时候才过来擦屁股,很明显,他做得很好 !*/
    tss->esp0 = (unsigned long)(STACK(leading)->stackp);
#ifdef  PRINT_SCHED
    if(oo != self() && oo->id && self()->id){
        LOGD("\eb[%02d]%14s\er-> \eb[%02d]%14s<%08x>\ew\n",oo->id,oo->name,self()->id,self()->name,leading->core);
    }
#endif
}

void sched(void)
{
    if(rdy_head[PRI_USER] && !(rdy_head[PRI_USER]->ucount)){
        rdy_tail[PRI_USER]->next = rdy_head[PRI_USER];
        rdy_tail[PRI_USER] = rdy_head[PRI_USER];
        rdy_head[PRI_USER] = rdy_head[PRI_USER]->next;
        rdy_tail[PRI_USER]->next = NULL;
        rdy_head[PRI_USER]->ucount = rdy_head[PRI_USER]->count;
    }
    pick_task();
}

/*! 接受党和人民考验的时候到了,是时候派我上场啦?你是说我还要排队,shit !*/
static void ready(Task *rt)
{
    if(!rt) panic("\erReady \eb[rTask is <null>\eb]");
    if(!rdy_head[rt->pri])
        rdy_head[rt->pri] = rt;
    else
        rdy_tail[rt->pri]->next = rt;
    rt->next = NULL;
    rdy_tail[rt->pri] = rt;
}

static void unready(Task *rt)
{
    Task *xt;
    if(!rt) panic("\erUnready   \eb[\erTask is <null>\eb]");
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
    do_switch();
}

static void _sleep(Object *obj)
{
    setSleep(obj);
    unready(TASK(obj));
}

static void _wait(Object *obj,Object *wait)
{
    setWait(obj);
    obj->wait = wait;
    unready(TASK(obj));
}


static void _wakeup(Object *obj)
{
    setActive(obj);
    obj->wait = NULL;
    ready(TASK(obj));
}

int dohook(unsigned long fn,Methon hook)
{
    if(fn < NR_METHON){
        self()->fns[fn] = hook;
        return OK;
    }
    return -EINVAL;
}

int dofn(object_t o,unsigned long fn,unsigned long r1,unsigned long r2,unsigned long r3)
{
    Object *obj = toObject(o);
    if(!obj) 
        return -ENOBJ;
    /*! 目标有事务在处理,则,将本次请求挂入队列 !*/
    //printk("%s call %s (%d)\n",self()->name,obj->name,fn);
    self()->fn = fn; self()->r1 = r1;
    self()->r2 = r2; self()->r3 = r3;
    if(!obj->wlink) 
        obj->wlink = self();
    else 
        obj->wtail->wnext = self();
    self()->wnext = NULL;
    obj->wtail = self();
    /* 如果对方在休眠,唤醒之 */
    if(isSleep(obj))
        _wakeup(obj);
    _wait(self(),obj);          /*! 任务提交了,等待对方为你服务.吹箫? !*/
    return self()->talk;        /*! 再次运行到这里已经是很久很久以后的事了 !*/
}

/*! 中断具有高优先级,必须优先处理 !*/
static iLink *first_iLink = NULL;
static cnt_t count_iLink = 0;
static iLink *alloc_iLink(void) 
{
    iLink *tmp;
    if(!first_iLink)
        return kalloc(sizeof(*first_iLink));
    tmp = first_iLink;
    first_iLink = first_iLink->inext;
    count_iLink--;
    return tmp;
}

static void free_iLink(iLink *ptr) 
{
    if(count_iLink < 32) {
        ptr->inext = first_iLink;
        first_iLink = ptr;
        count_iLink++;
    } else {
        kfree(ptr);
    }
}

int doint(object_t o,unsigned long fn,unsigned long r1,unsigned long r2,unsigned long r3)
{
    Object *obj = toObject(o);
    if(!obj) 
        panic("\er doint   \eb[\rnull\eb]\n");
    iLink   *in = alloc_iLink();
    if(!in) 
        panic("\er doint \eb[memory full\eb]\n");;
    in->fn = fn; in->r1 = r1;
    in->r2 = r2; in->r3 = r3;
    in->inext = NULL;
    if(obj->ilink) 
        in->inext = obj->ilink;
    obj->ilink = in;
    if(isSleep(obj)){
        _wakeup(obj);
    }
    return OK;
}

int doret(object_t o,long talk)
{
    Object *obj = toObject(o);
    if(obj && isWaitMe(obj)){
        obj->talk = talk;
        _wakeup(obj);
    }
    return OK;
}

void doloop(void) 
{
    long fn,r1,r2,r3,id;
    long ptr;
_again:
    if(self()->ilink) {
        iLink *in = self()->ilink;
        id = 0;
        fn = in->fn; r1 = in->r1;
        r2 = in->r2; r3 = in->r3;
        self()->ilink = in->inext;
        free_iLink(in);
    } else if(self()->wlink) {
        Object *in = self()->wlink;
        id = in->id;
        fn = in->fn; r1 = in->r1;
        r2 = in->r2; r3 = in->r3;
        self()->wlink = in->wnext;
    } else {
        _sleep(self());
        goto _again;
    }

    if(NULL == self()->fns[fn]) {
        doret(id,-ENOSYS);
        goto _again;
    } else {
        ptr = (long)self()->fns[fn];
    }

    __asm__("leave\n\t"
            "ret\n\t"
            ::"a"(ptr),"b"(id),"c"(r1),"d"(r2),"D"(r3)
            );
}

static inline void gam(Object *this)
{
    this->friend[0]         = 6;
    this->friend[1]         = 6;
    this->friend[2]         = 6;
}

static Task* make_task(id_t id,String name,uintptr_t data,uintptr_t code,int pri,int (*entry)())
{
    Task *task;
    task = (Task *)get_free_object();
    OBJECT(task)->id = id;
    task->pri = pri;
    task->count = 20;
    task->ucount = 20;
    task->core = getcr3();
    task->registers = (void*)(STACK(task)->stackp) - sizeof(Registers);
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
    gam(OBJECT(task));
    ready(task);
    return task;
}

void god_init(void)
{
    uintptr_t tr = TR_DESC;

    for(int i = 0;i < NR_PRI;i++) rdy_head[i] = rdy_tail[i] = NULL;
    leading = make_task(GOD,"God",KERNEL_DATA,KERNEL_CODE,PRI_GOD,NULL);
    leading->next =leading;
    extern int mm_main();
    make_task(MM_PID,"MM",KERNEL_DATA,KERNEL_CODE,PRI_TASK,mm_main);

    tss = (Tss*)(TSS_TABLE);
    tss->ss0 = KERNEL_DATA;
    tss->esp0 = (uintptr_t)(STACK(leading)->stackp);
    //tss->gs = tss->fs = tss->es = tss->ds = USER_DATE;
    tss->ldt = 0;
    tss->io = 0xffff0000;
    tss->eflags = 0x200;
    __asm__("ltr %0"::"m"(tr));
    do_switch();
}
