/*
 * rarian-reg-full.h
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

#ifndef __RARIAN_REG_FULL_H
#define __RARIAN_REG_FULL_H

#ifdef __cplusplus
extern "C" {
#endif

/* A special type of RrnReg that contains all the translations
 * of all the fields, instead of the interesting ones
 */
typedef struct _RrnRegFull    RrnRegFull;
typedef struct _RrnListEntry  RrnListEntry;

typedef struct _RrnSectFull RrnSectFull;

struct _RrnListEntry
{
    RrnListEntry * next;
    char *text;
    char *lang;
};

struct _RrnRegFull
{
  RrnListEntry *name;
  RrnListEntry *uri;
  RrnListEntry *comment;
  char *icon;
  char *identifier;
  char *type;
  char **categories;
  int weight;
  char *heritage;
  char *lang;
  char *default_section;
  int   hidden;
  RrnSectFull *children;
};

struct _RrnSectFull
{
	RrnListEntry *name;
	char *identifier;
	RrnListEntry *uri;
	char *owner;
	RrnSectFull *next;
	RrnSectFull *prev;
	RrnSectFull *children;
};

RrnRegFull * rrn_reg_new_full ();

RrnRegFull *  rrn_reg_parse_file_full (char *filename);

RrnRegFull *rrn_find_series_full (char * series_id);

RrnListEntry *rrn_full_add_field (RrnListEntry *current, char *text, char *lang);

void        rrn_reg_free_full (RrnRegFull *reg);

#ifdef __cplusplus
}
#endif

#endif /*__RARIAN_REG_FULL_H */
