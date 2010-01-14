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

#ifndef EOI_BATTERY_H
#define EOI_BATTERY_H

#include <Edje.h>

typedef enum {
    UNKNOWN,
    CHARGING,
    DISCHARGING,
    NOT_CHARGING,
    FULL_CHARGE,
    LOW_CHARGE,
} battery_status_t;

typedef struct {
    battery_status_t status;

    /* This field is reliable only in DISCHARGING and LOW_CHARGE states */
    int charge;
} battery_info_t;

void eoi_get_battery_info(battery_info_t * info);

void eoi_draw_given_battery_info(battery_info_t * info,
                                 Evas_Object * edje);

void eoi_draw_battery_info(Evas_Object * edje);

void eoi_run_battery(Evas_Object *);
#endif
