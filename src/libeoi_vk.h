#ifndef LIBEOI_VK_H
#define LIBEOI_VK_H

#include <Evas.h>

typedef void (*input_cb_t) (const char *input, size_t size, void *data);

Evas_Object *open_vk(Evas *canvas, input_cb_t input_callback,
                     const char *title, void *data);

#endif
