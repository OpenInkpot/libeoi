#ifndef LIBEOI_ENTRY_H
#define LIBEOI_ENTRY_H

#include <Evas.h>

typedef void (*entry_handler_t) (Evas_Object *entry,
                                 const char *text, void *param);

Evas_Object *entry_new(Evas *canvas, entry_handler_t handler,
                       const char *name, const char *text, void *data);

#endif
