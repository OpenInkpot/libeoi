/*
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

#define _GNU_SOURCE
#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include <Edje.h>

#include "libeoi_themes.h"
#include "libeoi_utils.h"

#define SYS_THEMES_DIR DATADIR "/eoi/themes"
#define USER_THEMES_DIR "/.e/themes"
#define THEME_VAR "EOI_THEME"

Evas_Object *
eoi_create_themed_edje(Evas *canvas, const char *edje, const char *group)
{
    const char *home = getenv("HOME");

    const char *theme = getenv(THEME_VAR);
    if (!theme)
        theme = "default";

    char *files[4] = {
        xasprintf("%s" USER_THEMES_DIR "/%s/%s.edj", home, theme, edje),
        xasprintf(SYS_THEMES_DIR "/%s/%s.edj", theme, edje),
        xasprintf("%s" USER_THEMES_DIR "/default/%s.edj", home, edje),
        xasprintf(SYS_THEMES_DIR "/default/%s.edj", edje),
    };

    Evas_Object *o = edje_object_add(canvas);
    bool edje_loaded = false;

    for (int i = 0; i < sizeof(files) / sizeof(files[0]); ++i) {
        if (edje_object_file_set(o, files[i], group)) {
            edje_loaded = true;
            break;
        }
    }

    if (!edje_loaded) {
        evas_object_del(o);
        fprintf(stderr, "libeoi: Can't load theme '%s' (group '%s')\n",
                edje, group);
        o = NULL;
    }

    for (int i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
        free(files[i]);

    return o;
}
