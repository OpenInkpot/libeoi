#ifndef LIBEOI_DIALOG_H
#define LIBEOI_DIALOG_H

#include <Evas.h>

/*
 * Creates dialog object, sets its name and swallows parameter into it. Returned
 * dialog may be disposed by passing to evas_object_del.
 */
Evas_Object *eoi_dialog_create(const char *name, Evas_Object *swallow);

/*
 * Sets the title of dialog.
 */
void eoi_dialog_title_set(Evas_Object *dialog, const char *title);

#endif
