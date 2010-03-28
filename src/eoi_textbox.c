/*
 * eoi_textbox -- TextBox object for evas
 *
 * © 2009 Alexander Kerner <lunohod@openinkpot.org>
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

#include <stdio.h>
#include <Eina.h>
#include <Evas.h>
#include <Edje.h>

#include "libeoi_textbox.h"
#include "libeoi_themes.h"

typedef struct {
    Evas_Object *parent;
    Evas_Object *textblock;
    Evas_Object *mask;
    int cur_page;
    int total_pages;
    eoi_textbox_page_updated_t page_handler;
    Eina_List *pages;
    Evas_Coord parent_w;
    Evas_Coord parent_h;
} eoi_textblock_info_t;

static Eina_List *
paginator(const Evas_Object * obj)
{
    Evas_Coord cur_y = 0, y, h;

    Eina_List *pages = NULL;

    pages = eina_list_append(pages, 0);

    eoi_textblock_info_t *info = evas_object_data_get(obj, "info");

    Evas_Textblock_Cursor *c =
        evas_object_textblock_cursor_new(info->textblock);
    for (int i = 0;
         evas_object_textblock_line_number_geometry_get(info->textblock, i,
                                                        NULL, &y, NULL,
                                                        &h); i++) {
        evas_textblock_cursor_line_set(c, i);

        if (cur_y + info->parent_h < y + h) {
            cur_y = y;
            pages = eina_list_append(pages, (const void *) y);
        }
    }
    evas_textblock_cursor_free(c);

    return pages;
}

static void
hide_last_line(const Evas_Object * obj)
{
    Evas_Coord x, y, w, h;
    Evas_Coord parent_x, parent_y;

    eoi_textblock_info_t *info = evas_object_data_get(obj, "info");

    evas_object_geometry_get(info->parent, &parent_x, &parent_y, NULL,
                             NULL);
    evas_object_geometry_get(info->textblock, NULL, &y, NULL, NULL);

    evas_object_hide(info->mask);

    int cur_y = parent_y - y;

    for (int i = 0;
         evas_object_textblock_line_number_geometry_get(info->textblock, i,
                                                        &x, &y, &w, &h);
         i++) {
        if (cur_y + info->parent_h < y + h) {
            evas_object_move(info->mask, parent_x + x,
                             parent_y + y - cur_y);
            evas_object_resize(info->mask, w, h);
            evas_object_show(info->mask);
            evas_object_raise(info->mask);

            return;
        }
    }
}

static void
textblock_resize_cb(void *data,
                    Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
    Evas_Coord ty, th;
    Evas_Coord parent_x, parent_y, parent_w;

    eoi_textblock_info_t *info = evas_object_data_get(obj, "info");
    if (!info)
        return;

    info->parent_w = w;
    info->parent_h = h;

    for (int i = 0;
         evas_object_textblock_line_number_geometry_get(info->textblock,
                                                        i,
                                                        NULL,
                                                        &ty,
                                                        NULL, &th); i++);

    evas_object_geometry_get(info->parent, &parent_x, &parent_y, &parent_w,
                             NULL);
    evas_object_move(info->textblock, parent_x, parent_y);
    evas_object_resize(info->textblock, w, ty + th);

    eina_list_free(eina_list_nth_list(info->pages, 0));

    info->pages = paginator(obj);
    info->cur_page = 0;
    info->total_pages = eina_list_count(info->pages);

    hide_last_line(obj);

    if (info->page_handler)
        info->page_handler(info->parent, info->cur_page, info->total_pages,
                           NULL);
}

Evas_Object *
eoi_textbox_new(Evas * canvas,
                const char *theme_file,
                const char *group_name,
                eoi_textbox_page_updated_t page_handler)
{
    Evas_Object *edje = eoi_create_themed_edje(canvas, theme_file, group_name);
    evas_object_show(edje);

    Evas_Object *t =
        (Evas_Object *) edje_object_part_object_get(edje, "text");
    evas_object_clip_set(t, edje);
    evas_object_intercept_resize_callback_add(t,
                                              textblock_resize_cb, NULL);
    Evas_Object *bg =
        (Evas_Object *) edje_object_part_object_get(edje, "bg");

    Evas_Object *mask = evas_object_rectangle_add(canvas);
    evas_object_name_set(mask, "mask");
    int r, g, b, a;
    evas_object_color_get(bg, &r, &g, &b, &a);
    evas_object_color_set(mask, r, g, b, a);
    evas_object_clip_set(mask, edje);

    eoi_textblock_info_t *info =
        (eoi_textblock_info_t *) malloc(sizeof(eoi_textblock_info_t));

    info->parent = edje;
    info->textblock = (Evas_Object *) t;
    info->mask = mask;
    info->page_handler = page_handler;
    info->cur_page = -1;
    info->total_pages = -1;
    info->pages = NULL;

    evas_object_data_set(edje, "info", info);
    evas_object_data_set(t, "info", info);

    return edje;
}

void
eoi_textbox_free(Evas_Object * obj)
{
    eoi_textblock_info_t *info = evas_object_data_get(obj, "info");

    evas_object_clip_unset(info->mask);
    evas_object_hide(info->mask);
    evas_object_del(info->mask);

    evas_object_hide(info->parent);
    evas_object_del(info->parent);

    eina_list_free(eina_list_nth_list(info->pages, 0));

    free(info);
}

void
eoi_textbox_text_set(Evas_Object * obj, const char *text)
{
    edje_object_part_text_set(obj, "text", text);
}

bool
eoi_textbox_page_next(Evas_Object * obj)
{
    eoi_textblock_info_t *info = evas_object_data_get(obj, "info");

    if (info->cur_page + 1 >= info->total_pages)
        return false;

    info->cur_page++;

    Evas_Coord x;
    Evas_Coord parent_x, parent_y;
    evas_object_geometry_get(info->parent, &parent_x, &parent_y, NULL,
                             NULL);
    evas_object_geometry_get(info->textblock, &x, NULL, NULL, NULL);
    evas_object_move(info->textblock, x,
                     parent_y -
                     (int)
                     eina_list_data_get(eina_list_nth_list
                                        (info->pages, info->cur_page)));
    hide_last_line(obj);

    if (info->page_handler)
        info->page_handler(info->parent, info->cur_page, info->total_pages,
                           NULL);

    return true;
}

bool
eoi_textbox_page_prev(Evas_Object * obj)
{
    eoi_textblock_info_t *info = evas_object_data_get(obj, "info");

    if (info->cur_page <= 0)
        return false;

    info->cur_page--;

    Evas_Coord x;
    Evas_Coord parent_x, parent_y;
    evas_object_geometry_get(info->parent, &parent_x, &parent_y, NULL,
                             NULL);
    evas_object_geometry_get(info->textblock, &x, NULL, NULL, NULL);
    evas_object_move(info->textblock, x,
                     parent_y -
                     (int)
                     eina_list_data_get(eina_list_nth_list
                                        (info->pages, info->cur_page)));
    hide_last_line(obj);

    if (info->page_handler)
        info->page_handler(info->parent, info->cur_page, info->total_pages,
                           NULL);

    return true;
}
