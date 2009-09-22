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

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <langinfo.h>

#include <Eina.h>
#include <Evas.h>

#include <libkeys.h>

#include "libeoi_textbox.h"
#include "libeoi_help.h"

#define THEME_EDJ (THEME_DIR "/help.edj")

typedef struct
{
    Evas_Object* textbox;
    char* application;
    eoi_help_page_updated_t page_handler;
    eoi_help_closed_t closed;
    keys_t* keys;
    keys_t* navigation;
    Eina_List* history;
} eoi_help_info_t;

static void load_page(eoi_help_info_t* info, const char* page);


static void help_page_update_handler(Evas_Object* tb,
                                     int cur_page,
                                     int total_pages,
                                     void* param)
{
    eoi_help_info_t* info = evas_object_data_get(tb, "help-info");
    if(info->page_handler)
        info->page_handler(tb, cur_page, total_pages, info->application, param);
}

static void key_handler(void* data, Evas* evas, Evas_Object* obj, void* event_info)
{
    eoi_help_info_t* info = data;

    if(!info->navigation)
    {
        char* app;
        asprintf(&app, "help/%s", info->application);
        info->navigation = keys_alloc(app);
        free(app);
    }

    if(!info->keys)
        info->keys = keys_alloc("eoi_help");


    const char* action =
        keys_lookup_by_event(info->navigation, eina_list_data_get(info->history), event_info);

    if(action)
    {
        load_page(info, action);
        return;
    }

    action = keys_lookup_by_event(info->keys, "default", event_info);
    if(!action)
        return;

    if(!strcmp(action, "PageDown"))
        eoi_textbox_page_next(info->textbox);
    else if(!strcmp(action, "PageUp"))
        eoi_textbox_page_prev(info->textbox);
    else if(!strcmp(action, "Back"))
    {
        if(eina_list_prev(info->history))
        {
            free(eina_list_data_get(info->history));
            info->history = eina_list_prev(info->history);
            char* page = strdup(eina_list_data_get(info->history));
            info->history =
                eina_list_remove_list(info->history, eina_list_next(info->history));

            free(eina_list_data_get(info->history));
            info->history = eina_list_prev(info->history);
            info->history =
                eina_list_remove_list(info->history, eina_list_next(info->history));

            load_page(info, page);
            free(page);
        } else
            eoi_help_free(info->textbox);
    }
    else if(!strcmp(action, "Close"))
        eoi_help_free(info->textbox);
}

static char* load_text(const char* filename)
{
    FILE* f;
    struct stat sb;

    if(stat(filename, &sb) == -1)
        return NULL;
    
    int len = sb.st_size;
    if(len <= 0)
        return NULL;

    char* text = (char*)malloc(len + 1);
    char* ptr = text;

    f = fopen(filename, "r");
    if(!f)
        return NULL;

    while(!feof(f) && fgets(ptr, len, f) && strlen(ptr))
    {
        len -= strlen(ptr);
        ptr += strlen(ptr);

        if(*(ptr - 1) == '\n')
            *(ptr - 1) = ' ';
    }

    fclose(f);
    
    return text;
}

static void load_page(eoi_help_info_t* info, const char* page)
{
    char* lang = strdup(nl_langinfo(_NL_ADDRESS_LANG_AB));
    if(!lang)
        lang = strdup("en");

    if(!strlen(lang) || !strcmp(lang, "C"))
    {
        free(lang);
        lang = strdup("en");
    }

    char* text = NULL;
    for(int i = 0; i < 2 && !text; i++, lang = strdup("en"))
    {
        char* filename;
        asprintf(&filename, "/usr/share/help/%s/%s/%s.help", info->application, lang, page);

        text = load_text(filename);

        free(lang);
        free(filename);
    }

    if(!text)
        return;

    eoi_textbox_text_set(info->textbox, text);
    free(text);
    info->history = eina_list_last(eina_list_append(info->history, strdup(page)));
}


Evas_Object* eoi_help_new(Evas* canvas,
                          const char* application,
                          eoi_help_page_updated_t page_handler,
                          eoi_help_closed_t closed)
{
    eoi_help_info_t* info =
        (eoi_help_info_t*)malloc(sizeof(eoi_help_info_t));

    info->textbox = eoi_textbox_new(canvas,
                                    THEME_EDJ,
                                    "help", 
                                    help_page_update_handler);

    info->application = strdup(application);
    info->page_handler = page_handler;
    info->closed = closed;
    info->history = NULL;
    info->keys = NULL;
    info->navigation = NULL;

    evas_object_event_callback_add(info->textbox,
                                   EVAS_CALLBACK_KEY_UP,
                                   &key_handler,
                                   (void*)info);

    load_page(info, "index");

    evas_object_data_set(info->textbox, "help-info", info);

    return info->textbox;
}

void eoi_help_free(Evas_Object* help)
{
    eoi_help_info_t* info = evas_object_data_get(help, "help-info");
    free(info->application);
    keys_free(info->keys);
    keys_free(info->navigation);

    Eina_List* l;
    char* data;
    EINA_LIST_FOREACH(info->history, l, data)
        free(data);
    eina_list_free(info->history);

    if(info->closed)
        info->closed();

    eoi_textbox_free(info->textbox);

    free(info);
}
