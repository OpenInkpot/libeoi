#ifndef EOI_BATTERY_H
#define EOI_BATTERY_H

#include <Edje.h>

typedef enum {
    UNKNOWN,
    CHARGING,
    DISCHARGING,
    NOT_CHARGING,
    FULL_CHARGE,
    LOW_CHARGE,
} battery_status_t;

typedef struct {
    battery_status_t status;

    /* This field is reliable only in DISCHARGING and LOW_CHARGE states */
    int charge;
} battery_info_t;

void eoi_get_battery_info(battery_info_t *info);

void eoi_draw_battery_info(Evas_Object *edje);

#endif
