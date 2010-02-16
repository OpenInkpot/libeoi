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

void eoi_register_fullscreen_choicebox(Evas_Object *choicebox);

/* "simple" resize callbacks */

typedef int eoi_resize_callback_token_t;
typedef void (*eoi_resize_handler_t)(Ecore_Evas *window,
                                     int w, int h, void *param);

eoi_resize_callback_token_t
eoi_resize_callback_add(Ecore_Evas *window,
                        eoi_resize_handler_t callback,
                        void *param);

void eoi_resize_callback_del(Ecore_Evas *window,
                             eoi_resize_callback_token_t token);

/* "object" resize callbacks */

typedef void (*eoi_object_resize_handler_t)(
    Ecore_Evas *window, Evas_Object *object, int w, int h, void *param);

/*
 * Passed callback will be called with given Evas_Object* and unregistered when
 * object is destroyed.
 */
void
eoi_resize_object_register(Ecore_Evas *window, Evas_Object *object,
                           eoi_object_resize_handler_t handler, void *param);

/* Passed object will be resized to the full size of window each time window
   is resized. Object deletion is handled too. */
void
eoi_fullwindow_object_register(Ecore_Evas *window, Evas_Object *object);


/* destroy callbacks */

void
eoi_evas_destroy_callback_add(Evas *window, void (*callback)(Evas*));

void
eoi_evas_destroy_callback_del(Evas *window, void (*callback)(Evas*));

#endif
