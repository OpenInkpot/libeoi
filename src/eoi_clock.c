/*
 * Copyright © 2010 Alexander V. Nikolaev <avn@daemon.hole.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdio.h>
#include <time.h>

#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>

#include "libeoi_clock.h"

static int
_update_clock(void *param)
{
    Evas_Object *edje = (Evas_Object *) param;
    char buf[256];
    time_t curtime;
    struct tm *loctime;
    curtime = time(NULL);
    loctime = localtime(&curtime);
    if (loctime->tm_year < 108) /* 2008 */
        edje_object_part_text_set(edje, "clock", "--:--");
    else {
        strftime(buf, 256, "%H:%M", loctime);
        edje_object_part_text_set(edje, "clock", buf);
    }
    return 1;
}

static void
_detach_clock_timer(void *data, Evas * e, Evas_Object * obj,
                    void *event_info)
{
    Ecore_Timer *timer = (Ecore_Timer *) data;
    ecore_timer_del(timer);
}

void
eoi_run_clock(Evas_Object * edje)
{
    Ecore_Timer *timer = ecore_timer_add(60, &_update_clock, edje);
    evas_object_event_callback_add(edje, EVAS_CALLBACK_DEL,
                                   &_detach_clock_timer, timer);
    _update_clock(edje);
}
