#include <none/list.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <none/types.h>
#include <none/scntl.h>
#include <sys/inter.h>
#include <x86/io.h>
#include <posix.h>

#define TIMER_IRQ 8

struct timer {
    struct list_head list;
    long   tick;
    pid_t  pid;
};

#define WHEEL_TICK    (64)
#define WHEEL0_TICK   WHEEL_TICK
#define WHEEL1_TICK   (WHEEL0_TICK * WHEEL0_TICK)
#define WHEEL2_TICK   (WHEEL1_TICK * WHEEL0_TICK)
#define WHEEL3_TICK   (WHEEL2_TICK * WHEEL0_TICK)
#define WHEEL4_TICK   (WHEEL3_TICK * WHEEL0_TICK)
#define WHEEL5_TICK   (WHEEL4_TICK * WHEEL0_TICK)

struct timing_wheel
{
    int     port;
    struct list_head fns[WHEEL0_TICK];
};

static struct timing_wheel wheels[5];
static volatile time_t tick = 0;

static void wheel_insert(struct timer *timer,struct timing_wheel *wheel,int offset)
{
    offset = (wheel->port + offset) % WHEEL_TICK;
    list_add(&timer->list,wheel->fns + offset);
}

static bool timer_insert(struct timer *timer)
{
    int offset = 0;
    if(timer->tick < WHEEL0_TICK) {
        offset = timer->tick;
        timer->tick = 0;
        wheel_insert(timer,wheels,offset);
    } else if(timer->tick < WHEEL1_TICK) {
        offset = timer->tick / WHEEL0_TICK;
        timer->tick -= (offset * WHEEL0_TICK);
        wheel_insert(timer,wheels + 1,offset);
    } else if(timer->tick < WHEEL2_TICK) {
        offset = timer->tick / WHEEL1_TICK;
        timer->tick -= (offset * WHEEL1_TICK);
        wheel_insert(timer,wheels + 2,offset);
    } else if(timer->tick < WHEEL3_TICK) {
        offset = timer->tick / WHEEL2_TICK;
        timer->tick -= (offset * WHEEL2_TICK);
        wheel_insert(timer,wheels + 3,offset);
    } else if(timer->tick < WHEEL4_TICK) {
        offset = timer->tick / WHEEL3_TICK;
        timer->tick -= (offset * WHEEL3_TICK);
        wheel_insert(timer,wheels + 4,offset);
    } else {
        return false;
    }
    return true;
}

static bool timer_add(struct timer *timer)
{
    if(timer->tick < WHEEL0_TICK) {
    } else if(timer->tick < WHEEL1_TICK) {
        timer->tick += wheels[0].port;
    } else if(timer->tick < WHEEL2_TICK) {
        timer->tick += (wheels[0].port + (wheels[1].port * WHEEL0_TICK));
    } else if(timer->tick < WHEEL3_TICK) {
        timer->tick += (wheels[0].port + (wheels[1].port * WHEEL0_TICK) + (wheels[2].port * WHEEL1_TICK));
    } else if(timer->tick < WHEEL4_TICK){
        timer->tick += (wheels[0].port + (wheels[1].port * WHEEL0_TICK) + (wheels[2].port * WHEEL1_TICK) + (wheels[3].port * WHEEL2_TICK));
    } else {
        return false;
    }
    return timer_insert(timer);
}


static void timer_init(void)
{
    for(int i = 0;i < 5;i++) {
        wheels[i].port = 0;
        for(int j = 0;j < WHEEL0_TICK;j++) {
            INIT_LIST_HEAD(wheels[i].fns + j);
        }
    }
}

static void timer_tick(void)
{
    struct timing_wheel *wheel = NULL;
    for(int i = 0;i < 5;i++) {
        wheel = wheels + i;
        wheel->port = (wheel->port + 1) % WHEEL_TICK;
        list_for_each_safe(pos,next,wheel->fns + wheel->port) {
            list_del(pos);
            struct timer *timer = container_of(pos,struct timer,list);
            if(timer->tick) {
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
    t->tick = (delay * 1000) / 122;
    if(!timer_add(t)) {
        ret(caller,ERROR);
    }
}

static void rtc_intr(object_t o,int irq)
{
    (void)o;(void)irq;
    outb_p(0x0c,0x70);
    inb(0x71);
    tick++;
    timer_tick();
}

static void read_tick(object_t o)
{
    ret(o,(tick * 122 / 1000));
}

static void rtc_init(void)
{
    outb_p(0x8b,0x70);
    unsigned char v = inb(0x71);
    outb_p(0x8b,0x70);
    outb_p(v | 0x40,0x71);

    outb_p(0x8a,0x70);
    v = inb(0x71);
    outb_p(0x8a,0x70);
    outb_p((v & 0xF0) | 3,0x71);
}

int main(void)
{
    timer_init();
    rtc_init();
    hook(IF_CREATE,mktimer);
    hook(IF_INTR,rtc_intr);
    hook(IF_READ,read_tick);
    regirq(TIMER_IRQ);
    printf("tiemr : %d\n",getpid());
    workloop();
    return 0;
}

