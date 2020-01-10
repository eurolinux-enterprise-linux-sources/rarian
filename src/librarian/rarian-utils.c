/*
 * rarian-utils.c
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAS_MALLOC_H */
#include "rarian-utils.h"

char*
rrn_chug (char *string)
{
	char *start;

	for (start = (char*) string; *start && isspace (*start); start++)
		;

	memmove (string, start, strlen ((char *) start) + 1);

	return string;
}

char*
rrn_chomp (char        *string)
{
	int len;

	len = strlen (string);

	while (len--) {
		if (isspace ((char) string[len]))
			string[len] = '\0';
		else
			break;
	}

  return string;
}

char *
rrn_strndup (char *string, int len)
{
  char *new_str = NULL;

  if (string) {
    new_str = (char *) calloc (sizeof(char), len + 1);
      strncpy (new_str, string, len);
      new_str[len] = '\0';
  } else
    new_str = NULL;

  return new_str;
}
