/*
 * rarian-omf.h
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

#ifndef RARIAN_OMF_H__
#define RARIAN_OMF_H__

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

#include <rarian-reg-utils.h>

#ifdef __cplusplus
extern "C" {
#endif

RrnReg *rrn_omf_parse_file (char *path);

#ifdef __cplusplus
}
#endif

#endif /* RARIAN_OMF_H__ */
