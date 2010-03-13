#include <Edje.h>

#include "libeoi_dialog.h"
#include "libeoi_themes.h"

Evas_Object *
eoi_dialog_create(const char *name, Evas_Object *swallow)
{
    Evas *canvas = evas_object_evas_get(swallow);
    Evas_Object *dlg = eoi_create_themed_edje(canvas, "eoi-dialog", "dialog");
    evas_object_name_set(dlg, name);
    edje_object_part_swallow(dlg, "contents", swallow);
    return dlg;
}

void
eoi_dialog_title_set(Evas_Object *dlg, const char *title)
{
    edje_object_part_text_set(dlg, "title", title);
}
