/*
 * rarian-info.h
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

#ifndef RARIAN_INFO_H__
#define RARIAN_INFO_H__

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

  /* Note: This encoding isn't exact.  It will be easily
   * tricked if (e.g.) someone sets an info file to point
   * at info.gz and is really uncompressed etc.
   */
  typedef enum _RrnInfoCompression {
    INFO_ENCODING_NONE = 0,
    INFO_ENCODING_GZIP,
    INFO_ENCODING_BZIP,
    INFO_ENCODING_LZMA,
    INFO_ENCODING_UNKNOWN,
  } RrnInfoCompression;

  typedef struct _RrnInfoEntry {
    char *name;
    char *shortcut_name;
    char *base_filename;
    char *base_path;
    char *section;
    char *doc_name;
    char *comment;
    RrnInfoCompression compression;
    char *category;
  } RrnInfoEntry;

  typedef int (* RrnInfoForeachFunc) (void * reg, void *data);

  char **rrn_info_get_categories (void);
  void rrn_info_for_each (RrnInfoForeachFunc funct, void * user_data);
  void rrn_info_for_each_in_category (char *category, RrnInfoForeachFunc funct, void * user_data);
  RrnInfoEntry *rrn_info_find_from_uri (char *uri, char *section);
  void rrn_info_shutdown ();

#ifdef __cplusplus
}
#endif

#endif /* RARIAN_INFO_H__ */
