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
#ifndef EOI_H
#define EOI_H

#include <Ecore_Evas.h>
#include <Evas.h>

Evas_Object *eoi_main_window_create(Evas * canvas);
Evas_Object *eoi_settings_left_create(Evas * canvas);
Evas_Object *eoi_settings_right_create(Evas * canvas);

void eoi_main_window_footer_show(Evas_Object *window);
void eoi_main_window_footer_hide(Evas_Object *window);

/* Choicebox numbering */

void eoi_register_fullscreen_choicebox(Evas_Object * choicebox);
void eoi_process_resize(Ecore_Evas * window);

/* Resize callbacks */
void eoi_resize_callback_add(Evas *window,
    void(*callback)(Evas *, int, int));
void eoi_resize_callback_del(Evas *window,
    void(*callback)(Evas *, int, int));

#endif
