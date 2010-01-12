#include <stdio.h>
#include <time.h>

#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>

#include "libeoi_clock.h"

static int _update_clock(void *param)
{
    Evas_Object *edje = (Evas_Object*)param;
    char buf[256];
    time_t curtime;
    struct tm* loctime;
    curtime = time(NULL);
    loctime = localtime(&curtime);
    if(loctime->tm_year < 108) /* 2008 */
        edje_object_part_text_set(edje, "clock", "--:--");
    else
    {
        strftime(buf, 256, "%H:%M", loctime);
        edje_object_part_text_set(edje, "clock", buf);
    }
    return 1;
}

static void
_detach_clock_timer(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Ecore_Timer* timer = (Ecore_Timer*) data;
    ecore_timer_del(timer);
}

void
eoi_run_clock(Evas_Object *edje)
{
    Ecore_Timer* timer = ecore_timer_add(60, &_update_clock, edje);
    evas_object_event_callback_add(edje, EVAS_CALLBACK_DEL,
            &_detach_clock_timer, timer);
    _update_clock(edje);
}
