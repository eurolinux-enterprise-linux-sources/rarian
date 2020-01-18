/*
 * rarian-utils.h
 * This file is part of Rarian
 *
 * Copyright (C) 2006 - Don Scorgie
 *
 * Rarian is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Rarian is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __RARIAN_UTILS_H
#define __RARIAN_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

 /* A collection of useful functions that have no real place
  * anywhere else
  */

 /* The string functions are shamelessly cribbed from glib,
  * changing their names to fit into rarian */

/* removes leading spaces */
char*                rrn_chug        (char        *string);
/* removes trailing spaces */
char*                rrn_chomp       (char        *string);

  char *               rrn_strndup     (char      *string, int len);


#define rrn_strip( string )    rrn_chomp (rrn_chug (string))


#ifdef __cplusplus
}
#endif

#endif /*__RRN_UTILS_H */
