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
#include <stdbool.h>

#include <Edje.h>
#include <Eina.h>
#include <Efreet.h>
#include <Ecore_X.h>

#include <libchoicebox.h>

#include "libeoi.h"

#define THEME_EDJ (THEME_DIR "/eoi.edj")

static Evas_Object* _eoi_create(Evas* canvas, const char* group)
{
    Evas_Object* o = edje_object_add(canvas);
    edje_object_file_set(o, THEME_EDJ, group);
    return o;
}

Evas_Object* eoi_main_window_create(Evas* canvas)
{
    return _eoi_create(canvas, "main-window");
}

Evas_Object* eoi_settings_left_create(Evas* canvas)
{
    return _eoi_create(canvas, "settings-left");
}

Evas_Object* eoi_settings_right_create(Evas* canvas)
{
    return _eoi_create(canvas, "settings-right");
}

/* Choicebox numbering */

#define CONFIG_NAME SYSCONFDIR "/eoi/choicebox-numbering.ini"

static Eina_List* choiceboxes;

static bool numbering_config_read;
static bool numbering_enabled;
static bool numbering_always;
static int width;
static int height;
static Ecore_X_Randr_Rotation rotation;

static void _eoi_numbering_read_config()
{
    if(numbering_config_read) return;

    numbering_config_read = true;

    Efreet_Ini* config = efreet_ini_new(CONFIG_NAME);
    if(!config)
        return;

    if(!efreet_ini_section_set(config, "default"))
    {
        efreet_ini_free(config);
        return;
    }

    numbering_enabled = true;
    numbering_always = efreet_ini_boolean_get(config, "always");
    width = efreet_ini_int_get(config, "w");
    height = efreet_ini_int_get(config, "h");
    rotation = efreet_ini_int_get(config, "rotation");
}

static void _eoi_unregister_fullscreen_choicebox(Evas_Object* choicebox)
{
    Eina_List* l;
    Eina_List* l_next;
    Evas_Object* o;

    EINA_LIST_FOREACH_SAFE(choiceboxes, l, l_next, o)
        if(o == choicebox)
        {
            choiceboxes = eina_list_remove_list(choiceboxes, l);
            return;
        }
}

static void _eoi_choicebox_del(void* data, Evas* canvas,
                               Evas_Object* choicebox, void* event_info)
{
    _eoi_unregister_fullscreen_choicebox(choicebox);
}

static bool _is_currently_hinted()
{
    if(!numbering_enabled)
        return false;

    if(numbering_always)
        true;

	Ecore_X_Window root = ecore_x_window_root_first_get();
    ecore_x_randr_get_screen_info_prefetch(root);
    ecore_x_randr_get_screen_info_fetch();
    Ecore_X_Randr_Rotation r = ecore_x_randr_screen_rotation_get(root);
    Ecore_X_Screen_Size s = ecore_x_randr_current_screen_size_get(root);

    return !(width == s.width && height == s.height && rotation == r);
}

void eoi_register_fullscreen_choicebox(Evas_Object* choicebox)
{
    _eoi_numbering_read_config();

    evas_object_event_callback_add(choicebox, EVAS_CALLBACK_DEL,
                                   &_eoi_choicebox_del, NULL);
    choiceboxes = eina_list_prepend(choiceboxes, choicebox);

    if(_is_currently_hinted())
        choicebox_set_hinted(choicebox, true);
}

void eoi_process_resize(Ecore_Evas* window)
{
    bool is_hinted = _is_currently_hinted();

    Eina_List *l;
    Evas_Object* o;

    EINA_LIST_FOREACH(choiceboxes, l, o)
        choicebox_set_hinted(o, is_hinted);
}
