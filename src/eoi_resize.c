#include <Evas.h>
#include <Ecore_Evas.h>
#include "libeoi.h"

#define CALLBACK_LIST "libeoi-ecore-resize-callbacks"

static void
_resize_event(Ecore_Evas *ee)
{
    Eina_List *callbacks = ecore_evas_data_get(ee, CALLBACK_LIST);
    if(!callbacks)
        return;
    Evas *evas = ecore_evas_get(ee);
    int w, h;
    evas_output_size_get(evas, &w, &h);
    Eina_List* tmp;
    void(*each)(Evas *, int, int);
    EINA_LIST_FOREACH(callbacks, tmp, each)
        each(evas, w, h);
}

void eoi_resize_callback_add(Evas *window,
    void (*callback)(Evas *, int, int))
{
    Ecore_Evas *ee = ecore_evas_ecore_evas_get(window);
    Eina_List *callbacks = ecore_evas_data_get(ee, CALLBACK_LIST);
    if(!callbacks)
        ecore_evas_callback_resize_set(ee, _resize_event);
    callbacks = eina_list_append(callbacks, callback);
    ecore_evas_data_set(ee, CALLBACK_LIST, callbacks);
}

void eoi_resize_callback_del(Evas *window,
    void (*callback)(Evas *, int, int))
{

    Ecore_Evas *ee = ecore_evas_ecore_evas_get(window);
    Eina_List *callbacks = ecore_evas_data_get(ee, CALLBACK_LIST);
    callbacks = eina_list_remove(callbacks, callback);
    ecore_evas_data_set(ee, CALLBACK_LIST, callbacks);
}
