/*
 * Copyright © 2009 Mikhail Gusarov <dottedmag@dottedmag.net>
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

#define _GNU_SOURCE

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Evas.h>
#include <Edje.h>

#include "libeoi.h"
#include "libeoi_battery.h"

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static Eina_Bool
exit_handler(void *param, int ev_type, void *event)
{
    ecore_main_loop_quit();
    return 1;
}

static void
main_win_close_handler(Ecore_Evas * main_win)
{
    ecore_main_loop_quit();
}

static void
exit_app(void *param)
{
    ecore_main_loop_quit();
}

static void
main_win_resize_handler(Ecore_Evas * main_win)
{
    Evas *canvas = ecore_evas_get(main_win);
    int w, h;
    evas_output_size_get(canvas, &w, &h);

    Evas_Object *main_edje = evas_object_name_find(canvas, "main-window");
    evas_object_resize(main_edje, w, h);
}

const char *sorts[] = {
    "sort-mode,none",
    "sort-mode,name",
    "sort-mode,namerev",
    "sort-mode,date"
};

const int sorts_size = sizeof(sorts) / sizeof(sorts[0]);

int cur_sort;

int show_clock;

#define L_OPEN 1
#define R_OPEN 2

int overlays_open;

static void
key_down(void *param, Evas * e, Evas_Object * o, void *event_info)
{
    Evas_Event_Key_Down *ev = (Evas_Event_Key_Down *) event_info;
    Evas *canvas = evas_object_evas_get(o);
    Evas_Object *rr = evas_object_name_find(canvas, "main-window");

    if (!strcmp(ev->keyname, "Escape"))
        ecore_main_loop_quit();

    if (!strcmp(ev->keyname, "KP_0")) {
        battery_info_t info = {.status = FULL_CHARGE };
        eoi_draw_given_battery_info(&info, rr);
    } else if (!strcmp(ev->keyname, "KP_1")) {
        battery_info_t info = {.status = UNKNOWN };
        eoi_draw_given_battery_info(&info, rr);
    } else if (!strcmp(ev->keyname, "KP_2")) {
        battery_info_t info = {.status = CHARGING };
        eoi_draw_given_battery_info(&info, rr);
    } else if (!strcmp(ev->keyname, "KP_9")) {
        battery_info_t info = {.status = NOT_CHARGING };
        eoi_draw_given_battery_info(&info, rr);
    } else if (strlen(ev->keyname) >= 4 && isdigit(ev->keyname[3])) {
        battery_info_t info = {.status = DISCHARGING,.charge =
                (ev->keyname[3] - '0' - 3) * 20 };
        eoi_draw_given_battery_info(&info, rr);
    }

    if (!strcmp(ev->keyname, "space")) {
        if (++cur_sort == sorts_size)
            cur_sort = 0;
        edje_object_signal_emit(rr, sorts[cur_sort], "");
    }

    if (!strcmp(ev->keyname, "Return")) {
        if (++show_clock == 2)
            show_clock = 0;

        edje_object_part_text_set(rr, "clock",
                                  show_clock ? "57:95" : "--:--");
    }

    if (!strcmp(ev->keyname, "Up")) {
        if (++overlays_open == 4)
            overlays_open = 0;
    }
}

static void
run()
{
    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

    Ecore_Evas *main_win =
        ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
    ecore_evas_title_set(main_win, "eoi-test");
    ecore_evas_name_class_set(main_win, "eoi-test", "eoi-test");

    Evas *main_canvas = ecore_evas_get(main_win);
    ecore_evas_callback_delete_request_set(main_win,
                                           main_win_close_handler);

    Evas_Object *rr = eoi_main_window_create(main_canvas);

    edje_object_part_text_set(rr, "title", "Some title");
    edje_object_part_text_set(rr, "footer", "0/0");

    evas_object_name_set(rr, "main-window");
    evas_object_move(rr, 0, 0);
    evas_object_resize(rr, 600, 800);
    evas_object_show(rr);

    evas_object_focus_set(rr, true);
    evas_object_event_callback_add(rr, EVAS_CALLBACK_KEY_DOWN, &key_down,
                                   NULL);

    Evas_Object *r = evas_object_rectangle_add(main_canvas);
    evas_object_color_set(r, 0, 0, 255, 255);
    evas_object_show(r);
    edje_object_part_swallow(rr, "contents", r);

    Evas_Object *set = eoi_settings_left_create(main_canvas);
    evas_object_show(set);
    edje_object_part_swallow(rr, "left-overlay", set);

    ecore_evas_callback_resize_set(main_win, main_win_resize_handler);
    ecore_evas_show(main_win);

    ecore_x_io_error_handler_set(exit_app, NULL);

    ecore_main_loop_begin();
}

int
main(int argc, char **argv)
{
    if (!evas_init())
        die("Unable to initialize Evas\n");
    if (!ecore_init())
        die("Unable to initialize Ecore\n");
    if (!ecore_evas_init())
        die("Unable to initialize Ecore_Evas\n");
    if (!edje_init())
        die("Unable to initialize Edje\n");

    run();

    edje_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
    return 0;
}
