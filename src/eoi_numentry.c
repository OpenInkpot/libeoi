/*
 * Copyright © 2010 Alexander Kerner <lunohod@openinkpot.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>

#include <Evas.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include "libeoi.h"
#include "libkeys.h"
#include "libeoi_themes.h"

#include "libeoi_numentry.h"

#define UNUSED __attribute__ ((unused))

#define MAXDIGITS	6

typedef struct numentry_info_t numentry_info_t;
struct numentry_info_t {
    Evas_Object *o;
    keys_t *keys;

    unsigned long value;
    const char *text;
    unsigned int cnt;

    numentry_handler_t handler;
};

static void
numentry_close(Evas_Object *obj, void *param)
{
    numentry_info_t *info = (numentry_info_t *) param;

    evas_object_hide(obj);

    keys_free(info->keys);

    Evas_Object *focus = evas_object_data_get(obj, "prev-focus");
    if (focus)
        evas_object_focus_set(focus, 1);

    free(info);

    evas_object_del(obj);
    evas_object_raise(focus);
}

static void
key_handler(void *data, Evas *evas UNUSED, Evas_Object *obj UNUSED,
            void *event_info)
{
    numentry_info_t *l_data = data;

    if (!l_data->keys)
        l_data->keys = keys_alloc("eoi_numentry");

    const char *action = keys_lookup_by_event(l_data->keys, "default", event_info);
    if(!action)
        return;

    if (!strcmp(action, "Return")) {
        void *custom_data = evas_object_data_get(l_data->o, "custom-data");
        l_data->handler(l_data->o, l_data->value, custom_data);
        numentry_close(l_data->o, data);
        return;
    } else if (!strcmp(action, "Escape")) {
        numentry_close(l_data->o, data);
        return;
    } else if (!strcmp(action, "Backspace")) {
        if (l_data->cnt <= 0)
            return;

        l_data->cnt--;
        l_data->value /= 10;
    } else if (isdigit(action[0]) && l_data->cnt < MAXDIGITS) {
        if (l_data->value > 0)
            l_data->value *= 10;
        else
            l_data->value = 0;

        l_data->value += action[0] - '0';

        if (l_data->value > 0)
            l_data->cnt++;
    }

	char *t;
    if(l_data->value > 0)
        asprintf(&t, "%s: %-*ld", l_data->text, MAXDIGITS, l_data->value);
    else
        asprintf(&t, "%s: %-*c", l_data->text, MAXDIGITS, 0);
	edje_object_part_text_set(obj, "entrytext", t);
	free(t);
}

Evas_Object *
numentry_new(Evas *canvas, numentry_handler_t handler,
          const char *name, const char *text, void *data)
{
    numentry_info_t *l_data = malloc(sizeof(numentry_info_t));
    l_data->value = 0;
    l_data->cnt = 0;
    l_data->text = text;
    l_data->keys = NULL;
    l_data->handler = handler;

    Evas_Object *obj =
        eoi_create_themed_edje(canvas, "eoi-entry", "entrybox");
    evas_object_name_set(obj, name);
    evas_object_move(obj, 0, 0);
    evas_object_resize(obj, 600, 800);

    evas_object_data_set(obj, "private-data", l_data);
    evas_object_data_set(obj, "custom-data", data);
    evas_object_data_set(obj, "prev-focus", evas_focus_get(canvas));

    Evas_Coord cw, ch;
    evas_output_size_get(canvas, &cw, &ch);

	char *t;
	asprintf(&t, "%s: %-*d", text, MAXDIGITS, 999999);
	edje_object_part_text_set(obj, "entrytext", t);
	free(t);

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(
			edje_object_part_object_get(obj, "entrytext"),
			&x, &y, &w, &h);

	asprintf(&t, "%s: %-*c", text, MAXDIGITS, 0);
	edje_object_part_text_set(obj, "entrytext", t);
	free(t);

    w += 40;
    h += 20;
    evas_object_resize(obj, cw / 2, h);
    evas_object_move(obj, cw / 4, (ch - h) / 2);

	evas_object_event_callback_add(obj,
			EVAS_CALLBACK_KEY_UP,
			&key_handler,
			(void*)l_data);

    evas_object_show(obj);

    evas_object_focus_set(obj, 1);

    l_data->o = obj;

    return obj;
}
