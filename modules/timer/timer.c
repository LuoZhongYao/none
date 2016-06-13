#include <none/list.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <none/types.h>
#include <none/scntl.h>
#include <sys/inter.h>
#include <x86/io.h>

#define TIMER_IRQ 8

struct timer {
    struct list_head list;
    long   delay;
    pid_t  pid;
};

#define WHEEL_TICK    (30)
#define WHEEL0_TICK   WHEEL_TICK
#define WHEEL1_TICK   (WHEEL0_TICK * WHEEL0_TICK)
#define WHEEL2_TICK   (WHEEL1_TICK * WHEEL0_TICK)
#define WHEEL3_TICK   (WHEEL2_TICK * WHEEL0_TICK)
#define WHEEL4_TICK   (WHEEL3_TICK * WHEEL0_TICK)

struct timing_wheel
{
    int     port;
    struct list_head fns[WHEEL0_TICK];
};

static struct timing_wheel wheels[4];

static void wheel_insert(struct timer *timer,struct timing_wheel *wheel,int offset)
{
    offset = (wheel->port + offset) % WHEEL_TICK;
    list_add(&timer->list,wheel->fns + offset);
}

static bool timer_insert(struct timer *timer)
{
    int offset = 0;
    if(timer->delay < WHEEL0_TICK) {
        offset = timer->delay;
        timer->delay = 0;
        wheel_insert(timer,wheels,offset);
    } else if(timer->delay < WHEEL1_TICK) {
        offset = timer->delay / WHEEL0_TICK;
        timer->delay -= (offset * WHEEL0_TICK);
        wheel_insert(timer,wheels + 1,offset);
    } else if(timer->delay < WHEEL2_TICK) {
        offset = timer->delay / WHEEL1_TICK;
        timer->delay -= (offset * WHEEL1_TICK);
        wheel_insert(timer,wheels + 2,offset);
    } else if(timer->delay < WHEEL3_TICK) {
        offset = timer->delay / WHEEL2_TICK;
        timer->delay -= (offset * WHEEL2_TICK);
        wheel_insert(timer,wheels + 3,offset);
    } else {
        return false;
    }
    return true;
}

static bool timer_add(struct timer *timer)
{
    if(timer->delay < WHEEL0_TICK) {
    } else if(timer->delay < WHEEL1_TICK) {
        timer->delay += wheels[0].port;
    } else if(timer->delay < WHEEL2_TICK) {
        timer->delay += (wheels[0].port + (wheels[1].port * WHEEL0_TICK));
    } else if(timer->delay < WHEEL3_TICK) {
        timer->delay += (wheels[0].port + (wheels[1].port * WHEEL0_TICK) + (wheels[2].port * WHEEL1_TICK));
    } else {
        return false;
    }
    return timer_insert(timer);
}


static void timer_init(void)
{
    for(int i = 0;i < 4;i++) {
        wheels[i].port = 0;
        for(int j = 0;j < WHEEL0_TICK;j++) {
            INIT_LIST_HEAD(wheels[i].fns + j);
        }
    }
}

static void timer_tick(void)
{
    struct timing_wheel *wheel = NULL;
    for(int i = 0;i < 4;i++) {
        wheel = wheels + i;
        wheel->port = (wheel->port + 1) % WHEEL_TICK;
        list_for_each_safe(pos,next,wheel->fns + wheel->port) {
            list_del(pos);
            struct timer *timer = container_of(pos,struct timer,list);
            if(timer->delay) {
                timer_insert(timer);
            } else {
                ret(timer->pid,OK);
                free(timer);
            }
        }
        if(wheel->port)
            break;
    }
}

static void mktimer(object_t caller,long delay)
{
    struct timer *t = malloc(sizeof(*t));
    INIT_LIST_HEAD(&t->list);
    t->pid = caller;
    t->delay = delay;
    if(!timer_add(t)) {
        ret(caller,ERROR);
    }
}

static void rts_intr(object_t o,int irq)
{
    outb_p(0x0c,0x70);
    printf("timer 0x0c = %02x\n",inb(0x71));
    (void)timer_tick;
}

static void rts_init(void)
{
    outb_p(0x8b,0x70);
    char v = inb(0x71);
    outb_p(0x8b,0x70);
    outb_p(v | 0x40,0x71);
}

int main(void)
{
    printf("timer\n");
    timer_init();
    rts_init();
    hook(IF_CREATE,mktimer);
    hook(IF_INTR,rts_intr);
    regirq(TIMER_IRQ);
    workloop();
    return 0;
}

