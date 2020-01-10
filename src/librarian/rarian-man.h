/*
 * rarian-man.h
 * This file is part of Rarian
 *
 * Copyright (C) 2007 - Don Scorgie
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

#ifndef RARIAN_MAN_H__
#define RARIAN_MAN_H__

#ifndef I_KNOW_RARIAN_0_8_IS_UNSTABLE
#error Rarian is unstable.  To use it, you must acknowledge that
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

  typedef struct _RrnManEntry {
    char *name;
    char *path;
    char *section;
    char *comment;
  } RrnManEntry;

  typedef int (* RrnManForeachFunc) (void * reg, void *data);

  void rrn_man_for_each (RrnManForeachFunc funct, void * user_data);
  void rrn_man_for_each_in_category (char *category, 
				       RrnManForeachFunc funct, 
				       void * user_data);
  char **rrn_man_get_categories (void);
  RrnManEntry *rrn_man_find_from_name (char *name, char *sect);
  void rrn_man_shutdown ();

#ifdef __cplusplus
}
#endif

#endif /* RARIAN_INFO_H__ */
