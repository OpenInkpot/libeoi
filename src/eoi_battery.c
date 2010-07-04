/*
 * Copyright © 2010 Alexander V. Nikolaev <avn@daemon.hole.ru>
 * Copyright © 2010 Mikhail Gusarov <dottedmag@dottedmag.net>
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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <Edje.h>
#include <Ecore.h>

#include "libeoi_battery.h"

typedef struct {
    char *now;
    char *min;
    char *max;
    char *status;
} battery_loc_t;

static battery_loc_t batteries[] = {
    {
     "/sys/class/power_supply/n516-battery/charge_now",
     "/sys/class/power_supply/n516-battery/charge_empty_design",
     "/sys/class/power_supply/n516-battery/charge_full_design",
     "/sys/class/power_supply/n516-battery/status",
     },
    {
     "/sys/class/power_supply/lbookv3_battery/charge_now",
     "/sys/class/power_supply/lbookv3_battery/charge_empty_design",
     "/sys/class/power_supply/lbookv3_battery/charge_full_design",
     "/sys/class/power_supply/lbookv3_battery/status",
     },
};

static int
_find_battery()
{
    unsigned int i;
    for (i = 0; i < sizeof(batteries) / sizeof(batteries[0]); ++i)
        if (!access(batteries[i].now, R_OK))
            return i;
    return -1;
}

static battery_status_t
_read_status_file(const char *filename)
{
    char buf[256];
    FILE *f = fopen(filename, "r");
    if (!f)
        return UNKNOWN;
    if (!fgets(buf, 255, f))
        *buf = '\0';
    fclose(f);

    if (!strcmp(buf, "Charging\n"))
        return CHARGING;
    if (!strcmp(buf, "Discharging\n"))
        return DISCHARGING;
    if (!strcmp(buf, "Not charging\n"))
        return NOT_CHARGING;
    if (!strcmp(buf, "Full\n"))
        return FULL_CHARGE;
    if (!strcmp(buf, "Low charge\n"))
        return LOW_CHARGE;
    return UNKNOWN;
}

static int
_read_int_file(const char *filename)
{
    int res;
    FILE *f = fopen(filename, "r");
    if (1 != fscanf(f, "%d", &res))
        res = 0;
    fclose(f);
    return res;
}

void
eoi_get_battery_info(battery_info_t * info)
{
    int batt = _find_battery();

    if (batt == -1) {
        info->status = UNKNOWN;
        info->charge = -1;
    } else {
        int now = _read_int_file(batteries[batt].now);
        int min = _read_int_file(batteries[batt].min);
        int max = _read_int_file(batteries[batt].max);
        info->status = _read_status_file(batteries[batt].status);
        if (max > min && now >= min && now <= max)
            info->charge = 100 * (now - min) / (max - min);
        else
            info->charge = -1;
    }
}

void
eoi_draw_given_battery_info(battery_info_t * info, Evas_Object * edje)
{
    if (info->status == UNKNOWN) {
        edje_object_signal_emit(edje, "battery-unknown", "");
    } else if (info->status == CHARGING) {
        edje_object_signal_emit(edje, "battery-charging", "");
    } else if (info->status == FULL_CHARGE || info->status == NOT_CHARGING) {
        edje_object_signal_emit(edje, "battery-full-charge", "");
    } else {
        char signal[256];
        snprintf(signal, 256, "battery-level,%d", info->charge);
        edje_object_signal_emit(edje, signal, "");
    }
}

void
eoi_draw_battery_info(Evas_Object * edje)
{
    battery_info_t info;
    eoi_get_battery_info(&info);
    eoi_draw_given_battery_info(&info, edje);
}

/* Edje bound to signal handler */
static Evas_Object *_bound = NULL;

static void
_detach_battery_timer(void *data, Evas * e, Evas_Object * obj,
                      void *event_info)
{
    Ecore_Timer *timer = (Ecore_Timer *) data;
    ecore_timer_del(timer);

    /* also unbound self from signal handler */
    if(obj == _bound)
        _bound = NULL;
}

static Eina_Bool
_update_batt_cb(void *param)
{
    eoi_draw_battery_info((Evas_Object *) param);
    return 1;
}

static void
_sigusr(int signo)
{
    if(_bound)
        eoi_draw_battery_info(_bound);
    signal(signo, _sigusr);
}

void
eoi_run_battery(Evas_Object * top)
{
    eoi_draw_battery_info(top);
    Ecore_Timer *timer = ecore_timer_add(5 * 60, &_update_batt_cb, top);
    evas_object_event_callback_add(top, EVAS_CALLBACK_DEL,
                                   &_detach_battery_timer, timer);

    _bound = top;
    signal(SIGUSR1, _sigusr);
    signal(SIGUSR2, _sigusr);
}
