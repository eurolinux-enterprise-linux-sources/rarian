/*
 * rarian-main.h
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

#ifndef __RARIAN_MAIN_H
#define __RARIAN_MAIN_H

#ifndef I_KNOW_RARIAN_0_8_IS_UNSTABLE
#error Rarian is unstable.  To use it, you must acknowledge that
#endif

#include <rarian-reg-utils.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

  typedef int (* RrnForeachFunc) (void * reg, void *data);

  void rrn_set_language (char *lang_code);

  void rrn_for_each (RrnForeachFunc funct, void * user_data);

  void rrn_for_each_in_category (RrnForeachFunc funct, char * category,
				   void *user_data);
  RrnReg * rrn_find_entry_from_uri (char *uri);

  RrnReg * rrn_find_from_name (char *name);

  RrnReg * rrn_find_from_ghelp (char *ghelp);
  
  void rrn_shutdown ();

#ifdef __cplusplus
}
#endif

#endif /*__RARIAN_MAIN_H */
