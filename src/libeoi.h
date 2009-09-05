#ifndef EOI_H
#define EOI_H

#include <Ecore_Evas.h>
#include <Evas.h>

Evas_Object* eoi_main_window_create(Evas* canvas);
Evas_Object* eoi_settings_left_create(Evas* canvas);
Evas_Object* eoi_settings_right_create(Evas* canvas);

/* Choicebox numbering */

void eoi_register_fullscreen_choicebox(Evas_Object* choicebox);
void eoi_process_resize(Ecore_Evas* window);

#endif
