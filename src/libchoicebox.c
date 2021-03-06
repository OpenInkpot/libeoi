/*
 * choicebox -- virtual listbox smart object for evas
 *
 * Copyright © 2009,2010 Mikhail Gusarov <dottedmag@dottedmag.net>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#define _GNU_SOURCE 1

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <Edje.h>
#include <libchoicebox.h>
#include <err.h>

#include "libeoi_themes.h"
#include "libeoi_utils.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

typedef struct {
    int size;
    int sel;
    int top_item;
    int pagesize;
    bool need_hints;
} choicebox_state_t;

typedef struct {
    /*
     * Invariant: st.sel == -1 || (st.sel >= st.top_item
     *                            && st.sel < (st.top_item + st.page_size))
     * Invariant: st.size == 0 || st.top_item < st.size
     */
    choicebox_state_t st;

    /* Data */
    choicebox_handler_t handler;
    choicebox_draw_handler_t draw_handler;
    choicebox_page_updated_t page_handler;
    choicebox_close_handler_t close_handler;
    void *param;

    /* Theme info */
    char *frame_theme_file;
    char *frame_theme_group;
    char *filler_frame_theme_group;
    int item_minh;
    int filler_h;
    long margin_top;
    long margin_right;
    long margin_bottom;
    long margin_left;

    char *item_theme_file;
    char *item_theme_group;

    /* Widgets */
    Evas_Object *clipper;
    Evas_Object *background;
    Eina_Array *frames;
    Eina_Array *fillers;

} choicebox_t;

typedef struct {
    bool is_used;
    bool is_selected;
    bool need_hints;
    int num;
} _choicebox_item_info_t;

static _choicebox_item_info_t
_choicebox_calc_item_info(int nth, const choicebox_state_t * state)
{
    _choicebox_item_info_t item_info = { };

    if (nth >= state->pagesize) {
        item_info.num = -1;
        return item_info;
    }

    item_info.num = state->top_item + nth;
    if (item_info.num == state->sel)
        item_info.is_selected = true;
    if (item_info.num < state->size)
        item_info.is_used = true;
    item_info.need_hints = state->need_hints;

    return item_info;
}

static void
_choicebox_update_item(Evas_Object * o, int nth,
                       const _choicebox_item_info_t * old,
                       const _choicebox_item_info_t * new)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    Evas_Object *frame = eina_array_data_get(data->frames, nth);

    if (old->is_selected && !new->is_selected)
        edje_object_signal_emit(frame, "deselect", "");
    if (!old->is_selected && new->is_selected)
        edje_object_signal_emit(frame, "select", "");

    if (!new->is_used) {
        if (old->is_used)
            edje_object_signal_emit(frame, "empty", "");
        return;
    }

    if (!old->is_used || (old->need_hints != new->need_hints)) {
        char buf[512];
        snprintf(buf, 512, "set_number,%d,%s",
                 nth, new->need_hints ? "hinted" : "non-hinted");
        edje_object_signal_emit(frame, buf, "");
    }

    if (!old->is_used || old->num != new->num) {
        Evas_Object *item =
            edje_object_part_swallow_get(frame, "contents");
        (*data->draw_handler) (o, item, new->num, nth, data->param);
    }
}

static void
_run_page_handler(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);

    int page = data->st.pagesize ? DIV_CEIL(data->st.top_item,
                                            data->st.pagesize) : 0;
    int pages = MAX(page + 1,
                    data->st.pagesize ? DIV_CEIL(data->st.size,
                                                 data->st.pagesize) : 0);

    (*data->page_handler) (o, page, pages, data->param);
}

static void
_choicebox_update(Evas_Object * o, const choicebox_state_t * new)
{
    choicebox_t *data = evas_object_smart_data_get(o);

    choicebox_state_t old = data->st;
    data->st = *new;

    if (old.need_hints != new->need_hints) {
        edje_object_signal_emit(data->background,
                                new->need_hints ? "hinted" : "non-hinted", "");
        for(int i = 0; i < new->pagesize - 1; ++i) {
            Evas_Object *filler = eina_array_data_get(data->fillers, i);
            edje_object_signal_emit(filler,
                                    new->need_hints ? "hinted": "non-hinted", "");
        }
    }

    int i;
    for (i = 0; i < new->pagesize; ++i) {
        _choicebox_item_info_t old_i = _choicebox_calc_item_info(i, &old);
        _choicebox_item_info_t new_i = _choicebox_calc_item_info(i, new);
        _choicebox_update_item(o, i, &old_i, &new_i);
    }

    if (old.size != new->size
        || old.top_item != new->top_item || old.pagesize != new->pagesize)
        _run_page_handler(o);
}

/**
 * Activates given item.
 *
 * No bounds checking.
 */
static void
_choicebox_activate(Evas_Object * o, int item_num, bool is_alt)
{
    choicebox_t *data = evas_object_smart_data_get(o);

    if (data->handler)
        (*data->handler) (o, item_num, is_alt, data->param);
}

static void
_frame_del(Evas_Object * frame)
{
    Evas_Object *item = edje_object_part_swallow_get(frame, "contents");
    evas_object_smart_member_del(frame);
    evas_object_smart_member_del(item);
    evas_object_del(frame);
    evas_object_del(item);
}

static void
_filler_del(Evas_Object *filler)
{
    if (filler) {
        evas_object_smart_member_del(filler);
        evas_object_del(filler);
    }
}

static void
_choicebox_display(Evas_Object * o, int ox, int oy, int ow, int oh)
{
    choicebox_t *data = evas_object_smart_data_get(o);

    choicebox_state_t new = data->st;

    /** Update background **/

    evas_object_move(data->background, ox, oy);
    evas_object_move(data->clipper, ox, oy);
    evas_object_resize(data->background, ow, oh);
    evas_object_resize(data->clipper, ow, oh);

    /** Update items **/

    /* Adjust x/y/w/h according to margins */
    ox += data->margin_left;
    oy += data->margin_top;
    ow -= data->margin_left + data->margin_right;
    oh -= data->margin_top + data->margin_bottom;
    if (ow < 0) ow = 0;
    if (oh < 0) oh = 0;

    /* Calculate pagesize */
    new.pagesize = (oh + data->filler_h) / (data->item_minh + data->filler_h);

    /* Fix the widgets amount */
    int curitems = eina_array_count_get(data->frames);
    if (new.pagesize > curitems) {
        Evas *evas = evas_object_evas_get(o);

        int i;
        for (i = 0; i < new.pagesize - curitems; ++i) {
            Evas_Object *frame = eoi_create_themed_edje(evas,
                                                        data->frame_theme_file,
                                                        data->frame_theme_group);
            if (!frame) {
                errx(1, "Unable to open theme %s(%s)", data->frame_theme_file,
                     data->frame_theme_group);
            }

            evas_object_smart_member_add(frame, o);
            char f[256];
            snprintf(f, 256, "choicebox/%p/frame/%d", o, i);
            evas_object_name_set(frame, f);

            evas_object_show(frame);
            evas_object_clip_set(frame, data->clipper);

            Evas_Object *item = eoi_create_themed_edje(evas,
                                                       data->item_theme_file,
                                                       data->item_theme_group);
            if (!item) {
                errx(1, "Unable to open theme %s(%s)", data->item_theme_file,
                     data->item_theme_group);
            }

            evas_object_smart_member_add(item, o);
            snprintf(f, 256, "choicebox/%p/item/%d", o, i);
            evas_object_name_set(item, f);

            evas_object_show(item);
            evas_object_clip_set(item, data->clipper);

            edje_object_part_swallow(frame, "contents", item);

            eina_array_push(data->frames, frame);

            if (curitems > 0 || i > 0) {
                Evas_Object *filler
                    = eoi_create_themed_edje(evas, data->frame_theme_file,
                                             data->filler_frame_theme_group);
                if (!filler) {
                    errx(1,"Unable to open theme %s(%s)",data->frame_theme_file,
                         data->filler_frame_theme_group);
                }
                evas_object_smart_member_add(filler, o);
                snprintf(f, 256, "choicebox/%p/filler/%d", o, i);
                evas_object_name_set(filler, f);

                evas_object_show(filler);
                evas_object_clip_set(filler, data->clipper);

                eina_array_push(data->fillers, filler);

                edje_object_signal_emit(filler, data->st.need_hints
                                        ? "hinted" : "non-hinted", "");
            }
        }
    }
    if (new.pagesize < curitems) {
        int i;
        for (i = 0; i < curitems - new.pagesize; ++i) {
            _frame_del(eina_array_pop(data->frames));
            _filler_del(eina_array_pop(data->fillers));
        }
    }

    /* Ajust selection if necessary */
    if (new.sel >= new.top_item + new.pagesize)
        new.sel = new.top_item + new.pagesize - 1;

    if (new.pagesize != 0) {
        /* Fix the widgets position */

        for (int i = 0; i < new.pagesize; ++i) {
            Evas_Object *frame = eina_array_data_get(data->frames, i);

            evas_object_move(frame, ox,
                             oy + i * (data->item_minh + data->filler_h));
            evas_object_resize(frame, ow, data->item_minh);

            if (i != new.pagesize - 1) {
                Evas_Object *filler = eina_array_data_get(data->fillers, i);
                evas_object_move(filler, ox, oy + data->item_minh
                                 + i * (data->item_minh + data->filler_h));
                evas_object_resize(filler, ow, data->filler_h);
            }
        }
    }

    _choicebox_update(o, &new);
}

static void
_choicebox_add(Evas_Object * o)
{
    choicebox_t *data = calloc(1, sizeof(choicebox_t));
    if (!data)
        return;

    if (!(data->frames = eina_array_new(10))) {
        free(data);
        return;
    }

    if (!(data->fillers = eina_array_new(10))) {
        free(data);
        eina_array_free(data->frames);
        return;
    }

    evas_object_smart_data_set(o, data);
}

static void
_choicebox_del(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (data) {
        if (data->frames) {
            int size = eina_array_count_get(data->frames);
            while (size--) {
                _frame_del(eina_array_pop(data->frames));
                _filler_del(eina_array_pop(data->fillers));
            }
            eina_array_free(data->frames);
            eina_array_free(data->fillers);
        }

        evas_object_del(data->background);
        evas_object_del(data->clipper);

        free(data->frame_theme_file);
        free(data->frame_theme_group);
        free(data->filler_frame_theme_group);
        free(data->item_theme_file);
        free(data->item_theme_group);

        free(data);
    }
}

static void
_choicebox_move(Evas_Object * o, Evas_Coord x, Evas_Coord y)
{
    int ow, oh;
    evas_object_geometry_get(o, NULL, NULL, &ow, &oh);

    _choicebox_display(o, x, y, ow, oh);
}

static void
_choicebox_resize(Evas_Object * o, Evas_Coord w, Evas_Coord h)
{
    int ox, oy;
    evas_object_geometry_get(o, &ox, &oy, NULL, NULL);

    _choicebox_display(o, ox, oy, w, h);
}

static void
_choicebox_show(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    _run_page_handler(o);
    evas_object_show(data->clipper);
}

static void
_choicebox_hide(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    evas_object_hide(data->clipper);
}

static void
_choicebox_clip_set(Evas_Object * o, Evas_Object * clip)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    evas_object_clip_set(data->clipper, clip);
}

static void
_choicebox_clip_unset(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    evas_object_clip_unset(data->clipper);
}

static Evas_Smart *
_choicebox_smart_get()
{
    static Evas_Smart *smart = NULL;

    static Evas_Smart_Class class_ = {
        .name = "choicebox",
        .version = EVAS_SMART_CLASS_VERSION,
        .add = _choicebox_add,
        .del = _choicebox_del,
        .move = _choicebox_move,
        .resize = _choicebox_resize,
        .show = _choicebox_show,
        .hide = _choicebox_hide,
        .color_set = NULL,
        .clip_set = _choicebox_clip_set,
        .clip_unset = _choicebox_clip_unset,
        .calculate = NULL,
        .member_add = NULL,
        .member_del = NULL
    };

    if (!smart)
        smart = evas_smart_class_new(&class_);
    return smart;
}

static long
edje_object_data_get_long(Evas_Object *o, const char *name)
{
    const char *data = edje_object_data_get(o, name);
    return data ? atoi(data) : 0;
}

static void
hack_update_min_height(Evas_Object * o)
{
    /*
     * HACK: Right now Edje does not set a size_hints on a group object, so
     * let's query it from the "data" section and set manually.
     */
    long minh = edje_object_data_get_long(o, "min_height");
    if (minh)
        edje_extern_object_min_size_set(o, 0, minh);
}

Evas_Object *
choicebox_new(Evas * evas, choicebox_info_t * info, void *param)
{
    Evas_Object *o = evas_object_smart_add(evas, _choicebox_smart_get());
    choicebox_t *data = evas_object_smart_data_get(o);
    if (!data)
        goto err;

    data->st.size = 0;

    /* Data */
    data->handler = info->handler;
    data->draw_handler = info->draw_handler;
    data->page_handler = info->page_handler;
    data->close_handler = info->close_handler;
    data->param = param;

    /* GUI */
    int ow;
    evas_object_geometry_get(o, NULL, NULL, &ow, NULL);

    data->st.sel = -1;
    data->st.top_item = 0;
    data->st.pagesize = 0;

    /* Theme info */
    data->frame_theme_file = strdup(info->frame_theme_file);
    if (!data->frame_theme_file)
        goto err;
    data->frame_theme_group = strdup(info->frame_theme_group);
    if (!data->frame_theme_group)
        goto err;
    data->filler_frame_theme_group
        = xasprintf("%s-filler", info->frame_theme_group);
    data->item_theme_file = strdup(info->item_theme_file);
    if (!data->item_theme_file)
        goto err;
    data->item_theme_group = strdup(info->item_theme_group);
    if (!data->item_theme_group)
        goto err;

    /* Widgets */
    data->clipper = evas_object_rectangle_add(evas);
    evas_object_color_set(data->clipper, 255, 255, 255, 255);
    evas_object_smart_member_add(data->clipper, o);

    char f[256];
    snprintf(f, 256, "choicebox/%p/clipper", o);
    evas_object_name_set(data->clipper, f);

    snprintf(f, 256, "%s-background", info->frame_theme_group);

    if (info->background)
        data->background = info->background;
    else {
        data->background = eoi_create_themed_edje(evas,
                                                  info->frame_theme_file, f);
    }

    if (!data->background)
        goto err;
    evas_object_smart_member_add(data->background, o);

    snprintf(f, 256, "choicebox/%p/background", o);
    evas_object_name_set(data->background, f);

    evas_object_clip_set(data->background, data->clipper);

    evas_object_show(data->background);

    Evas_Object *tmpitem = eoi_create_themed_edje(evas, data->frame_theme_file,
        data->frame_theme_group);
    if (!tmpitem)
        goto err;

    data->margin_left = edje_object_data_get_long(tmpitem, "margin_left");
    data->margin_right = edje_object_data_get_long(tmpitem, "margin_right");
    data->margin_top = edje_object_data_get_long(tmpitem, "margin_top");
    data->margin_bottom = edje_object_data_get_long(tmpitem, "margin_bottom");

    hack_update_min_height(tmpitem);
    evas_object_size_hint_min_get(tmpitem, NULL, &data->item_minh);
    evas_object_del(tmpitem);

    Evas_Object *tmpfiller
        = eoi_create_themed_edje(evas, data->frame_theme_file,
                                 data->filler_frame_theme_group);
    if (!tmpfiller)
        goto err2;
    data->filler_h = edje_object_data_get_long(tmpfiller, "height");
    evas_object_del(tmpfiller);

    return o;

err2:
    evas_object_del(tmpfiller);
    return NULL;

err:
    evas_object_del(o);
    return NULL;
}

void
choicebox_set_hinted(Evas_Object * o, bool is_hinted)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    choicebox_state_t new = data->st;
    new.need_hints = is_hinted;

    _choicebox_update(o, &new);
}

void
choicebox_set_size(Evas_Object * o, int new_size)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    choicebox_state_t new = data->st;
    new.size = new_size;

    if (data->st.pagesize) {
        if (new_size == 0) {
            new.sel = -1;
            new.top_item = 0;
        } else {
            new.top_item = MIN(new.top_item, new_size - 1);

            if (new.sel != -1)
                new.sel = MIN(new.sel, new_size - 1);
        }
    }

    _choicebox_update(o, &new);
}

void
choicebox_invalidate_interval(Evas_Object * o, int item_from, int item_to)
{
    choicebox_t *data = evas_object_smart_data_get(o);

    /* [from, to) = [item_from, item_to) x [first_visible, last_visible) */
    int from = MAX(data->st.top_item, item_from);
    int to = MIN(data->st.top_item + data->st.pagesize,
                 MIN(data->st.size, item_to));
    int i;

    for (i = from; i < to; ++i) {
        int nth = i - data->st.top_item;
        Evas_Object *frame = eina_array_data_get(data->frames, nth);
        Evas_Object *item =
            edje_object_part_swallow_get(frame, "contents");
        (*data->draw_handler) (o, item, i, nth, data->param);
    }
}

void
choicebox_invalidate_item(Evas_Object * o, int item_num)
{
    choicebox_invalidate_interval(o, item_num, item_num + 1);
}

/* Navigating items */

void
choicebox_prev(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (!data->st.pagesize)
        return;
    if (!data->st.size)
        return;

    choicebox_state_t new = data->st;
    if (new.sel == -1) {
        new.sel = MIN(new.size - 1, new.top_item + new.pagesize - 1);
    } else {
        new.sel = MAX(0, new.sel - 1);
        if (new.sel < new.top_item)
            new.top_item = MAX(0, new.top_item - new.pagesize);
    }

    _choicebox_update(o, &new);
}

void
choicebox_next(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (!data->st.pagesize)
        return;
    if (!data->st.size)
        return;

    choicebox_state_t new = data->st;

    if (new.sel == -1) {
        new.sel = new.top_item;
    } else {
        new.sel = MIN(new.size - 1, new.sel + 1);
        if (new.sel >= new.top_item + new.pagesize)
            new.top_item = MIN(new.size - 1, new.top_item + new.pagesize);
    }

    _choicebox_update(o, &new);
}

void
choicebox_prev_pages(Evas_Object * o, int n)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (!data->st.pagesize)
        return;

    choicebox_state_t new = data->st;
    new.top_item = MAX(0, new.top_item - new.pagesize * n);

    if (new.sel != -1)
        new.sel = MAX(0, new.sel - new.pagesize * n);

    _choicebox_update(o, &new);
}

void
choicebox_next_pages(Evas_Object * o, int n)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (!data->st.pagesize)
        return;
    if (!data->st.size)
        return;

    /* Don't turn page if pointer would be beyond last page */
    if (data->st.top_item + data->st.pagesize * n >= data->st.size)
        return;

    choicebox_state_t new = data->st;
    new.top_item = MIN(new.size - 1, new.top_item + new.pagesize * n);

    if (new.sel != -1)
        new.sel = MIN(new.size - 1, new.sel + new.pagesize * n);

    _choicebox_update(o, &new);
}

/* Activating items */

void
choicebox_activate_nth_visible(Evas_Object * o, int nth, bool is_alt)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (nth < 0 || nth >= data->st.pagesize) {
        /* Item beyond the current GUI list size. Harmless, just ignore it */
        return;
    }

    int item_num = data->st.top_item + nth;
    if (item_num >= data->st.size) {
        /* User tried to press the button for item beyond end of list.
           It's ok, just ignore it */
        return;
    }

    _choicebox_activate(o, item_num, is_alt);
}

void
choicebox_activate_current(Evas_Object * o, bool is_alt)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (data->st.sel == -1)     /* No item is current */
        return;

    _choicebox_activate(o, data->st.sel, is_alt);
}

void
choicebox_set_selection(Evas_Object * o, int sel)
{
    choicebox_t *data = evas_object_smart_data_get(o);

    if (sel < -1 || sel > data->st.size)
        return;

    choicebox_state_t new = data->st;
    new.sel = sel;

    if (new.sel != -1 && new.pagesize != 0)
        new.top_item = new.sel / new.pagesize * new.pagesize;

    _choicebox_update(o, &new);
}

int
choicebox_get_selection(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    return data->st.sel;
}

void
choicebox_scroll_to(Evas_Object * o, int item_num)
{
    choicebox_t *data = evas_object_smart_data_get(o);

    if (item_num < 0 || item_num >= data->st.size)
        return;

    choicebox_state_t new = data->st;
    new.top_item = item_num;

    if (new.sel < new.top_item || new.sel > new.top_item + new.pagesize)
        new.sel = -1;

    _choicebox_update(o, &new);
}

int
choicebox_get_scroll_pos(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    return data->st.top_item;
}

void
choicebox_request_close(Evas_Object * o)
{
    choicebox_t *data = evas_object_smart_data_get(o);
    if (data->close_handler)
        (*data->close_handler) (o, data->param);
}
