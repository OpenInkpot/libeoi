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

#include <Evas.h>
#include <Ecore_Evas.h>
#include "libeoi.h"

#define DESTROY_CALLBACK_LIST "libeoi-ecore-close-callbacks"

#define RESIZE_CALLBACK_LIST "libeoi-ecore-resize-callbacks"
#define RESIZE_CALLBACK_NEXT_ID "libeoi-ecore-resize-next-id"

static void
_destroy_event(Ecore_Evas *ee)
{
    Eina_List *callbacks = ecore_evas_data_get(ee, DESTROY_CALLBACK_LIST);
    Evas *evas = ecore_evas_get(ee);
    if(callbacks)
    {
        int w, h;
        evas_output_size_get(evas, &w, &h);
        Eina_List* tmp;
        void(*each)(Evas *);
        EINA_LIST_FOREACH(callbacks, tmp, each)
            each(evas);
        eina_list_free(callbacks);
    }

    /* Also clean up RESIZE callbacks if exists */
    callbacks = ecore_evas_data_get(ee, RESIZE_CALLBACK_LIST);
    if(callbacks)
        eina_list_free(callbacks);
}

void
eoi_evas_destroy_callback_add(Evas *window, void (*callback)(Evas*))
{
    Ecore_Evas *ee = ecore_evas_ecore_evas_get(window);
    Eina_List *callbacks = ecore_evas_data_get(ee, DESTROY_CALLBACK_LIST);
    if(!callbacks)
        ecore_evas_callback_destroy_set(ee, _destroy_event);
    callbacks = eina_list_append(callbacks, callback);
    ecore_evas_data_set(ee, DESTROY_CALLBACK_LIST, callbacks);
}


void
eoi_evas_destroy_callback_del(Evas *window, void (*callback)(Evas*))
{
    Ecore_Evas *ee = ecore_evas_ecore_evas_get(window);
    Eina_List *callbacks = ecore_evas_data_get(ee, DESTROY_CALLBACK_LIST);
    if(callbacks)
    {
        callbacks = eina_list_remove(callbacks, callback);
        ecore_evas_callback_destroy_set(ee, _destroy_event);
    }
}

/* Resize callback */

typedef struct {
    eoi_resize_handler_t callback;
    void *param;
} resize_handler_info_t;

typedef struct {
    Ecore_Evas *window;
    int w;
    int h;
} resize_callback_info_t;

static Eina_Bool
_resize_event_cb(const Eina_Hash *hash, const void *key,
                     void *data, void *fdata)
{
    resize_handler_info_t *handler = (resize_handler_info_t *)data;
    resize_callback_info_t *info = (resize_callback_info_t *)fdata;
    (handler->callback)(info->window, info->w, info->h, handler->param);
    return EINA_TRUE;
}

static void
_resize_event(Ecore_Evas *window)
{
    Eina_Hash *callbacks = ecore_evas_data_get(window, RESIZE_CALLBACK_LIST);
    if (!callbacks)
        return;

    Evas *evas = ecore_evas_get(window);
    resize_callback_info_t info = { window };
    evas_output_size_get(evas, &info.w, &info.h);

    eina_hash_foreach(callbacks, &_resize_event_cb, &info);
}

eoi_resize_callback_token_t
eoi_resize_callback_add(Ecore_Evas *window, eoi_resize_handler_t callback,
                        void *param)
{
    Eina_Hash *callbacks = ecore_evas_data_get(window, RESIZE_CALLBACK_LIST);
    if (!callbacks) {
        ecore_evas_callback_resize_set(window, &_resize_event);
        callbacks = eina_hash_int32_new(EINA_FREE_CB(free));
        ecore_evas_data_set(window, RESIZE_CALLBACK_LIST, callbacks);
        ecore_evas_data_set(window, RESIZE_CALLBACK_NEXT_ID, 1);
    }
    int next_id = (int)ecore_evas_data_get(window, RESIZE_CALLBACK_NEXT_ID);

    resize_handler_info_t *info = malloc(sizeof(resize_handler_info_t));
    info->callback = callback;
    info->param = param;
    eina_hash_add(callbacks, &next_id, info);

    ecore_evas_data_set(window, RESIZE_CALLBACK_NEXT_ID, (void*)(next_id + 1));
    return next_id;
}

void
eoi_resize_callback_del(Ecore_Evas *window,
                        eoi_resize_callback_token_t token)
{
    Eina_Hash *callbacks = ecore_evas_data_get(window, RESIZE_CALLBACK_LIST);
    if (callbacks)
        eina_hash_del_by_key(callbacks, &token);
}

/* "Object" resize callbacks */

typedef struct {
    Evas_Object *object;
    eoi_object_resize_handler_t handler;
    eoi_resize_callback_token_t token;
    void *param;
} eoi_resize_object_info_t;

static void
_resize_object_cb(Ecore_Evas *window, int w, int h, void *param)
{
    eoi_resize_object_info_t *info = param;
    (*info->handler)(window, info->object, w, h, info->param);
}

static void
_resize_object_del(void *data, Evas *canvas, Evas_Object *object,
                   void *event_info)
{
    eoi_resize_object_info_t *info = data;
    Ecore_Evas *window = ecore_evas_ecore_evas_get(canvas);
    eoi_resize_callback_del(window, info->token);
    free(info);
}

void
eoi_resize_object_register(Ecore_Evas *window, Evas_Object *object,
                           eoi_object_resize_handler_t handler, void *param)
{
    eoi_resize_object_info_t *info = malloc(sizeof(eoi_resize_object_info_t));
    info->object = object;
    info->handler = handler;
    info->param = param;

    info->token = eoi_resize_callback_add(window, &_resize_object_cb, info);
    evas_object_event_callback_add(object, EVAS_CALLBACK_DEL,
                                   &_resize_object_del, info);
}

static void
_fullwindow_resize_cb(Ecore_Evas *window, Evas_Object* object, int w, int h,
                                 void *param)
{
    evas_object_resize(object, w, h);
}

void
eoi_fullwindow_object_register(Ecore_Evas *window, Evas_Object *object)
{
    eoi_resize_object_register(window, object, &_fullwindow_resize_cb, NULL);
}
