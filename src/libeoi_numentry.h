#ifndef LIBEOI_NUMENTRY_H
#define LIBEOI_NUMENTRY_H

#include <Evas.h>

typedef void (*numentry_handler_t) (Evas_Object *entry,
                                 unsigned long value, void *param);

Evas_Object *numentry_new(Evas *canvas, numentry_handler_t handler,
                       const char *name, const char *text, void *data);
#endif
