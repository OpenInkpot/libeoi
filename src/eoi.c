#include <Edje.h>

#include "eoi.h"

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
