/*
 * eoi_help -- Help object for evas
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

#ifndef LIBEOI_HELP_H
#define LIBEOI_HELP_H

#include <Evas.h>
#include <libkeys.h>

typedef void (*eoi_help_page_updated_t)(Evas_Object* help,
                                        int cur_page,
                                        int total_pages,
                                        const char* header,
                                        void* param);

typedef void (*eoi_help_closed_t)(Evas_Object* help);

Evas_Object* eoi_help_new(Evas* canvas,
                          const char* application,
                          eoi_help_page_updated_t page_handler,
                          eoi_help_closed_t closed);
void eoi_help_free(Evas_Object* help);

/*
 * Open new help window over canvas
 *    canvas --  canvas to draw on
 *    application -- name of current app, used for select help files
 *    page --- name of page to start with,  "index" if NULL supplied
 *    app_keys_info -- application keys to be substituted into help text
 */
void eoi_help_show(Evas* canvas,
                   const char* application,
                   const char* page,
                   const char* help_win_title,
                   keys_t* app_keys_info);

#endif
