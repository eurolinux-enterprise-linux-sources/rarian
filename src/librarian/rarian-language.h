/*
 * rarian-language.h
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


#ifndef __RARIAN_LANGUAGE_H
#define __RARIAN_LANGUAGE_H

#ifdef __cplusplus
extern "C" {
#endif

void rrn_language_init (char *lang);

int  rrn_language_use (char *current_language, char *proposed);

char ** rrn_language_get_dirs (char *base);

  char ** rrn_language_get_langs (void);

void rrn_language_shutdown (void);

#ifdef __cplusplus
}
#endif

#endif /*__RARIAN_LANGUAGE_H */
