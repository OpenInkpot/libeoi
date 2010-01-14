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

#ifndef LIBEOI_TEXTBOX_H
#define LIBEOI_TEXTBOX_H

#include <stdbool.h>
#include <Evas.h>

typedef void (*eoi_textbox_page_updated_t) (Evas_Object * textbox,
                                            int cur_page,
                                            int total_pages, void *param);

Evas_Object *eoi_textbox_new(Evas * canvas,
                             const char *theme_file,
                             const char *group_name,
                             eoi_textbox_page_updated_t page_handler);
void eoi_textbox_free(Evas_Object * textbox);

void eoi_textbox_text_set(Evas_Object * textbox, const char *text);
bool eoi_textbox_page_next(Evas_Object * textbox);
bool eoi_textbox_page_prev(Evas_Object * textbox);

#endif
