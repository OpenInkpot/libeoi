/*
 * Copyright © 2009,2010 Mikhail Gusarov <dottedmag@dottedmag.net>
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
#include <stdbool.h>
#include <string.h>

#include <Edje.h>
#include <Eina.h>
#include <Efreet.h>
#include <Ecore_X.h>

#include <libchoicebox.h>

#include "libeoi.h"
#include "libeoi_themes.h"

#define THEME_EDJ "eoi-main-window"

Evas_Object *
eoi_main_window_create(Evas * canvas)
{
    return eoi_create_themed_edje(canvas, THEME_EDJ, "main-window");
}

Evas_Object *
eoi_settings_left_create(Evas * canvas)
{
    return eoi_create_themed_edje(canvas, THEME_EDJ, "settings-left");
}

Evas_Object *
eoi_settings_right_create(Evas * canvas)
{
    return eoi_create_themed_edje(canvas, THEME_EDJ, "settings-right");
}

void
eoi_main_window_footer_show(Evas_Object *window)
{
    edje_object_signal_emit(window, "footer,show", "");
}

void
eoi_main_window_footer_hide(Evas_Object *window)
{
    edje_object_signal_emit(window, "footer,hide", "");
}

/* Choicebox numbering */

#define CONFIG_NAME SYSCONFDIR "/eoi/choicebox-numbering.ini"

static bool numbering_config_read;
static bool numbering_enabled;
static bool numbering_always;
static int width;
static int height;
static Ecore_X_Randr_Rotation rotation;

static void
_eoi_numbering_read_config()
{
    if (numbering_config_read)
        return;

    numbering_config_read = true;

    Efreet_Ini *config = efreet_ini_new(CONFIG_NAME);
    if (!config)
        return;

    if (!efreet_ini_section_set(config, "default")) {
        efreet_ini_free(config);
        return;
    }

    numbering_enabled = true;
    numbering_always = efreet_ini_boolean_get(config, "always");
    width = efreet_ini_int_get(config, "w");
    height = efreet_ini_int_get(config, "h");
    rotation = efreet_ini_int_get(config, "rotation");
}

static bool
_is_currently_hinted()
{
    if (!numbering_enabled)
        return false;

    if (numbering_always)
        return true;

    Ecore_X_Window root = ecore_x_window_root_first_get();
    ecore_x_randr_get_screen_info_prefetch(root);
    ecore_x_randr_get_screen_info_fetch();
    Ecore_X_Randr_Rotation r = ecore_x_randr_screen_rotation_get(root);
    Ecore_X_Screen_Size s = ecore_x_randr_current_screen_size_get(root);

    return !(width == s.width && height == s.height && rotation == r);
}

static void
_fullscreen_choicebox_cb(Ecore_Evas *window, int w, int h, void *param)
{
    choicebox_set_hinted((Evas_Object*)param, _is_currently_hinted());
}

static void
_fullscreen_choicebox_del(void *data, Evas *canvas,
                   Evas_Object *choicebox, void *event_info)
{
    eoi_resize_callback_token_t token = (eoi_resize_callback_token_t)data;
    Ecore_Evas *window = ecore_evas_ecore_evas_get(canvas);

    eoi_resize_callback_del(window, token);
}


void
eoi_register_fullscreen_choicebox(Evas_Object *choicebox)
{
    _eoi_numbering_read_config();
    Evas *canvas = evas_object_evas_get(choicebox);
    Ecore_Evas *window = ecore_evas_ecore_evas_get(canvas);

    eoi_resize_callback_token_t token
        = eoi_resize_callback_add(window, &_fullscreen_choicebox_cb, choicebox);
    evas_object_event_callback_add(choicebox, EVAS_CALLBACK_DEL,
                                   &_fullscreen_choicebox_del, (void*)token);

    if (_is_currently_hinted())
        choicebox_set_hinted(choicebox, true);
}

/* Trimming */

void
eoi_edje_text_trim_left(Evas_Object *edje, char *part, const char *prefix,
                        const char *str, const char* suffix)
{
    const Evas_Object *part_obj = edje_object_part_object_get(edje, part);

    int width;
    bool trim = false;

    edje_object_part_text_set(edje, part, "");
    edje_object_calc_force(edje);

    edje_object_part_geometry_get(edje, part, NULL, NULL, &width, NULL);
    for (;;) {
        char *s = xasprintf("%s%s%s%s", prefix, trim ? "..." : "", str, suffix);
        edje_object_part_text_set(edje, part, s);
        free(s);

        edje_object_calc_force(edje);
        int advance = evas_object_text_horiz_advance_get(part_obj);

        /* FIXME? */
        if (width > advance + width / 20)
            break;
        trim = true;

        int d;
        int p = evas_string_char_next_get(str, 0, &d);
        if (d == 0) {
            char *s = xasprintf("%s...%s", prefix, suffix);
            edje_object_part_text_set(edje, part, s);
            free(s);
            break;
        }
        str += p;
    }
}

void
eoi_edje_text_trim_right(Evas_Object *edje, char *part, const char *prefix,
                         const char *str, const char *suffix)
{
    const Evas_Object *part_obj = edje_object_part_object_get(edje, part);

    /* For modifications */
    char *d = strdup(str);
    int pos = strlen(d);

    int width;
    bool trim = false;

    edje_object_part_text_set(edje, part, "");
    edje_object_calc_force(edje);

    edje_object_part_geometry_get(edje, part, NULL, NULL, &width, NULL);
    for (;;) {
        char *s = xasprintf("%s%s%s%s", prefix, d, trim ? "..." : "", suffix);
        edje_object_part_text_set(edje, part, s);
        free(s);

        edje_object_calc_force(edje);
        int advance = evas_object_text_horiz_advance_get(part_obj);

        /* FIXME? */
        if (width > advance + width / 20)
            break;
        trim = true;

        /* Step one character back */
        int c;
        pos = evas_string_char_prev_get(d, pos, &c);
        d[pos] = '\0';

        /* Whole string was cut */
        if (pos == 0) {
            char *s = xasprintf("%s...%s", prefix, suffix);
            edje_object_part_text_set(edje, part, s);
            break;
        }
    }

    free(d);
}
