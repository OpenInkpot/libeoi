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
#include <string.h>
#include <stdbool.h>

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include "libchoicebox.h"
#include "libeoi.h"
#include "libkeys.h"
#include "libeoi_themes.h"

#include "libeoi_vk.h"

#define UNUSED __attribute__ ((unused))

#define EVK_WINDOW_ID "libeoi-evk-window"
#define DEFAULT_CHOICEBOX_THEME_FILE "choicebox"

typedef struct vk_layout_t {
    const char *name;
    const char *sname;
    const char *file;
} vk_layout_t;

typedef struct vk_info_t {
    Evas_Object *main_window;
    Evas_Object *choicebox;
    Ecore_Timer *timer;
    keys_t *keys;
    keys_t *layout_keys;
    char *last_key;
    bool shift;
    int i;

    Eina_List *layouts;
    Eina_List *curr_layout;

    input_cb_t input_callback;
    void *data;
} vk_info_t;

static Eina_List *
load_layouts_from_path(Eina_List *layout_list, const char *path1,
                       const char *path2)
{
    char *f, *p;

    char *file = malloc(1024);

    Eina_List *l, *ls;

    asprintf(&p, "%s/%s", path1, path2);

    ls = ecore_file_ls(p);
    EINA_LIST_FOREACH(ls, l, f) {
        char *s = strrchr(f, '.');

        if (!s || strcmp(s, ".ini"))
            continue;

        *s = '\0';

        snprintf(file, 1024, "%s/%s", path2, f);

        free(f);

        keys_t *keys = keys_alloc(file);

        if (keys) {
            vk_layout_t *layout = malloc(sizeof(vk_layout_t));

            const char *str;

            str = keys_lookup(keys, "default", "name");
            layout->name = strdup(str);
            str = keys_lookup(keys, "default", "sname");
            layout->sname = strdup(str);

            layout->file = strdup(file);

            layout_list = eina_list_append(layout_list, layout);

            keys_free(keys);
        }
    }
    eina_list_free(ls);

    free(p);
    free(file);

    return layout_list;
}

static const char *
get_action(vk_info_t *info, const char *key)
{
    char *x;

    if (info->shift)
        asprintf(&x, "Shift+%d", info->i + 1);
    else
        asprintf(&x, "%d", info->i + 1);
    const char *action = keys_lookup(info->layout_keys, key, x);

    free(x);

    return action;
}

static void
send_key(vk_info_t *info, const char *key)
{
    const char *action = get_action(info, key);

    if (info->input_callback && action)
        (info->input_callback) (action, strlen(action), info->data);

    info->i = 0;
}

static int
timer_callback(void *param)
{
    vk_info_t *info = param;

    info->timer = NULL;

    if (!info->last_key)
        return 0;

    send_key(info, info->last_key);
    free(info->last_key);
    info->last_key = NULL;

    return 0;
}

static void
del_handler(void *data, Evas *evas UNUSED, Evas_Object *obj UNUSED,
            void *event_info UNUSED)
{
    vk_info_t *info = data;

    choicebox_request_close(info->choicebox);
}

static void
key_handler(void *data, Evas *evas UNUSED, Evas_Object *obj UNUSED,
            void *event_info)
{
    vk_info_t *info = data;

    if (info->timer) {
        ecore_timer_del(info->timer);
        info->timer = NULL;
    }

    if (!info->layout_keys) {
        vk_layout_t *layout = eina_list_data_get(info->curr_layout);

        info->layout_keys = keys_alloc(layout->file);
    }

    Evas_Event_Key_Up *event = (Evas_Event_Key_Up *) event_info;

    const char *action = keys_lookup_by_event(info->keys, "default", event_info);
    if(action) {
        if (!strcmp(action, "change_layout")) {
            keys_free(info->layout_keys);
            info->layout_keys = NULL;

            info->curr_layout = eina_list_next(info->curr_layout);
            if (!info->curr_layout)
                info->curr_layout = info->layouts;

            choicebox_invalidate_interval(info->choicebox, 0, 9);

            return;
        }

        if (!strcmp(action, "shift")) {
            info->shift = !info->shift;
            choicebox_invalidate_interval(info->choicebox, 0, 9);
            return;
        }

        if (!strcmp(action, "space")) {
            (info->input_callback) (" ", strlen(" "), info->data);
            return;
        }

        if (!strcmp(action, "BackSpace")
                || !strcmp(action, "Return")
                || !strcmp(action, "Escape")) {
            (info->input_callback) (action, strlen(action),
                    info->data);
            return;
        }
    }

    if (info->last_key) {
        if (strcmp(info->last_key, event->keyname))
            send_key(info, info->last_key);
        else
            info->i++;
    }

    action = get_action(info, event->keyname);

    if (action)
        info->timer = ecore_timer_add(0.7, &timer_callback, info);

    if (!info->last_key || strcmp(info->last_key, event->keyname)) {
        free(info->last_key);
        info->last_key = strdup(event->keyname);
    }
}

static void
_close_handler(Evas_Object *choicebox __attribute__ ((unused)),
               void *param)
{
    vk_info_t *info = param;

    Evas_Object *focus =
        (Evas_Object *) evas_object_data_get(info->main_window,
                                             "prev-focus");

    if (focus)
        evas_object_focus_set(focus, true);

    evas_object_hide(info->choicebox);
    evas_object_del(info->choicebox);

    if (info->layouts) {
        vk_layout_t *d;

        Eina_List *l;

        EINA_LIST_FOREACH(info->layouts, l, d)
            free(d);

        eina_list_free(info->layouts);
    }

    if(info->layout_keys)
        keys_free(info->layout_keys);

    if(info->keys)
        keys_free(info->keys);

    free(info);
}

static void
_page_handler(Evas_Object *self UNUSED,
              int a UNUSED, int b UNUSED, void *param UNUSED)
{
}

static void
_draw_handler(Evas_Object *choicebox __attribute__ ((unused)),
              Evas_Object *item,
              int item_num,
              int page_position __attribute__ ((unused)),
              void *param __attribute__ ((unused)))
{
    vk_info_t *info = param;

    if (!info->layout_keys) {
        vk_layout_t *layout = eina_list_data_get(info->curr_layout);

        info->layout_keys = keys_alloc(layout->file);
    }

    char *section = NULL, *label = NULL;

    asprintf(&section, "KP_%d", item_num + 1);
    for (int i = 1; i <= 4; i++) {
        char *x;

        if (info->shift)
            asprintf(&x, "Shift+%d", i);
        else
            asprintf(&x, "%d", i);

        const char *action = keys_lookup(info->layout_keys, section, x);

        if (action && strlen(action)) {
            if (label) {
                char *tmp = strdup(label);

                free(label);
                asprintf(&label, "%s %s", tmp, action);
                free(tmp);
            } else
                asprintf(&label, "%s", action);
        }

        free(x);
    }
    free(section);

    edje_object_part_text_set(item, "text", label ? label : "");
    free(label);

    vk_layout_t *layout = eina_list_data_get(info->curr_layout);

    edje_object_part_text_set(info->main_window, "footer", layout->sname);
}

static void
destroy_choicebox(vk_info_t *evk_info)
{
    edje_object_part_unswallow(evk_info->main_window, evk_info->choicebox);
    evas_object_hide(evk_info->choicebox);
    evas_object_del(evk_info->choicebox);
    evk_info->choicebox = NULL;
}

static void
create_choicebox(Evas *canvas, vk_info_t *evk_info, int height)
{
    choicebox_info_t info = {
        NULL,
        height >
            600 ? DEFAULT_CHOICEBOX_THEME_FILE :
            DEFAULT_CHOICEBOX_THEME_FILE "_vk",
        "settings-left",
        height >
            600 ? DEFAULT_CHOICEBOX_THEME_FILE :
            DEFAULT_CHOICEBOX_THEME_FILE "_vk",
        "item-default",
        NULL,
        _draw_handler,
        _page_handler,
        _close_handler,
    };

    Evas_Object *choicebox = choicebox_new(canvas, &info, evk_info);

    if (!choicebox) {
        printf("no choicebox\n");
        return;
    }

    eoi_register_fullscreen_choicebox(choicebox);
    choicebox_set_size(choicebox, 9);
    evas_object_name_set(choicebox, "evk");
    evas_object_focus_set(choicebox, 1);
    evas_object_show(choicebox);

    edje_object_part_swallow(evk_info->main_window, "contents", choicebox);

    evk_info->choicebox = choicebox;
}

static void
_evk_resized(Ecore_Evas *ee, Evas_Object *object, int w, int h,
             void *param)
{
    vk_info_t *info = param;

    destroy_choicebox(info);
    create_choicebox(ecore_evas_get(ee), info, h);

    evas_object_resize(object, w / 3, h);
    evas_object_raise(object);
}

Evas_Object *
open_vk(Evas *canvas, input_cb_t input_callback, const char *title,
        void *data)
{
    vk_info_t *evk_info = calloc(sizeof(vk_info_t), 1);

    evk_info->keys = keys_alloc("evk");

    evk_info->input_callback = input_callback;
    evk_info->data = data;

    evk_info->layouts =
        load_layouts_from_path(evk_info->layouts, DATADIR "/keys",
                               "evk");

    char *user_path;

    asprintf(&user_path, "%s" "/.keys", getenv("HOME")),
        evk_info->layouts =
        load_layouts_from_path(evk_info->layouts, user_path, "evk_custom");
    free(user_path);

    if (!evk_info->layouts)
        return NULL;

    evk_info->curr_layout = evk_info->layouts;

    Evas_Object *evkwin = eoi_settings_left_create(canvas);

    evas_object_name_set(evkwin, EVK_WINDOW_ID);
    edje_object_part_text_set(evkwin, "title", title);
    int w, h;

    evas_output_size_get(canvas, &w, &h);
    evas_object_resize(evkwin, w / 3, h);
    evas_object_show(evkwin);

    Evas_Object *focus = evas_focus_get(canvas);

    evas_object_data_set(evkwin, "prev-focus", focus);

    Ecore_Evas *window = ecore_evas_ecore_evas_get(canvas);

    eoi_resize_object_register(window, evkwin, _evk_resized, evk_info);

    evk_info->main_window = evkwin;

    evas_object_event_callback_add(evkwin, EVAS_CALLBACK_DEL,
                                   &del_handler, evk_info);
    evas_object_event_callback_add(evkwin, EVAS_CALLBACK_KEY_UP,
                                   &key_handler, evk_info);

    create_choicebox(canvas, evk_info, h);

    return evkwin;
}
