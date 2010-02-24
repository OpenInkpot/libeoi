/*
 * Copyright © 2010 Mikhail Gusarov <dottedmag@dottedmag.net>
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
#include <err.h>
#include <stdarg.h>

char *
eoi_xasprintf(const char *fmt, ...)
{
    char *res;
    va_list ap;
    va_start(ap, fmt);
    int ret = vasprintf(&res, fmt, ap);
    va_end(ap);
    if (ret == -1) {
        err(1, "xasprintf");
        return NULL;
    }
    return res;
}
