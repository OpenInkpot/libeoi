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

#include "libeoi_vk.h"
#include "libeoi_entry.h"

#define UNUSED __attribute__ ((unused))

#define ENTRY_HEIGHT 76

typedef struct entry_info_t entry_info_t;
struct entry_info_t {
    Evas_Object *o;
    Evas_Object *vk;

    const char *title;
    char *text;
    unsigned int cnt;
    size_t text_buf_size;

    entry_handler_t handler;
};

static void
entry_close(Evas_Object *obj, void *param)
{
    entry_info_t *info = (entry_info_t *) param;

    if (info->vk)
        evas_object_del((Evas_Object *) info->vk);

    evas_object_hide(obj);

    Evas_Object *focus = evas_object_data_get(obj, "prev-focus");
    if (focus)
        evas_object_focus_set(focus, 1);

    free(info->text);
    free(info);

    evas_object_del(obj);
    evas_object_raise(focus);
}

static void
input_handler(const char *input, size_t size, void *data)
{
    entry_info_t *l_data = data;

    if (!strcmp(input, "Return")) {
        void *custom_data = evas_object_data_get(l_data->o, "custom-data");
        l_data->handler(l_data->o, l_data->text, custom_data);
        entry_close(l_data->o, data);
        return;
    } else if (!strcmp(input, "Escape")) {
        entry_close(l_data->o, data);
        return;
    } else if (!strcmp(input, "BackSpace")) {
        if (l_data->cnt <= 0)
            return;
        char *c = l_data->text + l_data->cnt - 1;
        while (--l_data->cnt > 0 && (*c & 0xc0) == 0x80)
            c--;
        *c = '\0';
    } else {
        while (l_data->text_buf_size - l_data->cnt < size) {
            l_data->text_buf_size += 256;
            l_data->text = realloc(l_data->text, l_data->text_buf_size);
        }

        memcpy(&l_data->text[l_data->cnt], input, size);
        l_data->cnt += size;
    }

    char *t;
    asprintf(&t, "<ellipsize=left>%s</ellipsize>", l_data->text);
    edje_object_part_text_set(l_data->o, "entrytext", t);
    free(t);
}

/*
static void
_entry_resized(Ecore_Evas *win UNUSED, Evas_Object *object, int w, int h,
               void *param UNUSED)
{
    evas_object_move(object, w / 3, 49);
    evas_object_resize(object, w * 2 / 3, ENTRY_HEIGHT);
    evas_object_raise(object);
}
*/

Evas_Object *
entry_new(Evas *canvas, entry_handler_t handler,
          const char *name, const char *text, void *data)
{
    entry_info_t *l_data = malloc(sizeof(entry_info_t));
    l_data->title = text;
    l_data->text = calloc(1, 256);
    l_data->text_buf_size = 256;
    l_data->cnt = 0;
    l_data->handler = handler;

    Evas_Object *obj =
        eoi_create_themed_edje(canvas, "eoi-entry", "entrybox");
    evas_object_name_set(obj, name);
    /*
    evas_object_move(obj, 0, 0);
    evas_object_resize(obj, 600, 800);
    */

    evas_object_data_set(obj, "private-data", l_data);
    evas_object_data_set(obj, "custom-data", data);
    evas_object_data_set(obj, "prev-focus", evas_focus_get(canvas));

    Evas_Coord w, h;
    evas_output_size_get(canvas, &w, &h);

    /*
    evas_object_resize(obj, w * 2 / 3, ENTRY_HEIGHT);
    evas_object_move(obj, w / 3, 49);
    */
    evas_object_show(obj);

    evas_object_focus_set(obj, 1);

    Ecore_Evas *window = ecore_evas_ecore_evas_get(canvas);
    //eoi_resize_object_register(window, obj, _entry_resized, NULL);

    Evas_Object *mw = eoi_create_themed_edje(canvas, "eoi-entry", "mw");
    evas_object_move(mw, 0, 0);
    evas_object_resize(mw, w, h);
    evas_object_show(mw);
    eoi_fullwindow_object_register(window, mw);

    Evas_Object *bg = eoi_main_window_create(canvas);

    l_data->o = obj;
    l_data->vk = open_vk(canvas, input_handler, l_data->title, l_data);

    edje_object_part_swallow(mw, "bg-overlay", bg);
    edje_object_part_swallow(mw, "evk-overlay", l_data->vk);
    edje_object_part_swallow(mw, "entry-overlay", obj);

    setlocale(LC_ALL, "");
    const char *help_text = dgettext("libeoi",
        "M - Change layout<br>"
        "OK - Accept text<br>"
        "Left - Delete last symbol<br>"
        "Right - Add space after last symbol<br>"
        "<br>"
        "Press number key multiple times to enter the corresponding symbol");

    edje_object_part_text_set(mw, "help-text", help_text);

    return obj;
}
