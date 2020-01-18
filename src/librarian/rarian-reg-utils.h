/*
 * rarian-reg-utils.h
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

#ifndef __RARIAN_REG_UTILS_H
#define __RARIAN_REG_UTILS_H

#ifndef I_KNOW_RARIAN_0_8_IS_UNSTABLE
#error Rarian is unstable.  To use it, you must acknowledge that
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RrnReg  RrnReg;
typedef struct _RrnSect RrnSect;

struct _RrnReg
{
  char *name;
  char *uri;
  char *comment;
  char *identifier;
  char *type;
  char *icon;
  char **categories;
  int weight;
  char *heritage;
  char *omf_location;
  char *ghelp_name;
  char *lang;
  char *default_section;
  int   hidden;
  RrnSect *children;
};

struct _RrnSect
{
	char *name;
	char *identifier;
	char *uri;
	char *owner;
	RrnSect *next;
	RrnSect *prev;
	RrnSect *children;

	/* Used internally to determine whether to overwrite
	 * the section
	 */
	int priority;
};

RrnReg * rrn_reg_new ();
RrnSect * rrn_sect_new ();

RrnReg *  rrn_reg_parse_file (char *filename);
RrnSect * rrn_sect_parse_file (char *filename);

RrnSect * rrn_reg_add_sections (RrnReg *reg, RrnSect *sects);

void        rrn_reg_free (RrnReg *reg);
void        rrn_sect_free (RrnSect *sect);

#ifdef __cplusplus
}
#endif

#endif /*__RARIAN_REG_UTILS_H */
