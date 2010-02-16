/*
 * Copyright © 2009 Alexander V. Nikolaev <avn@daemon.hole.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <libkeys.h>

char *
eoi_subst_keys(const char *text, keys_t * keys, const char *context)
{
    char *bp;
    size_t size;
    FILE *stream;

    stream = open_memstream(&bp, &size);

    while (*text) {
        if (*text == '@') {
            text += 1;
            const char *end = text;
            while (*end != '@') {
                if (!*end) {
                    fputs(text, stream);
                    fprintf(stderr, "substitute keys: malformed input\n");
                    goto done;
                }
                ++end;
            }
            char *macro = strndup(text, end - text);
            Eina_List *reverse = keys_reverse_lookup(keys, context, macro);
            char *keysym = NULL;
            if (reverse) {
                keysym = eina_list_nth(reverse, 0);
            }
            if (keysym)
                fputs(keys_get_key_name(keysym), stream);
            else
                fputs(macro, stream);
            text = end;
            free(macro);
        } else
            fputc(*text, stream);
        ++text;
    }

  done:
    fclose(stream);
//    printf("subst: %s\n", bp);
    return bp;
}
