/*
 * rarian-language.c
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
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <rarian-language.h>
#include "rarian-utils.h"

typedef struct _Lang Lang;

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

struct _Lang
{
	char *base;
    Lang *next;
    Lang *prev;
};

static Lang *lang_list;
static int nlangs = 0 ;

static int
check_lang (char *lang)
{
  Lang *iter = lang_list;

  while (iter) {
    if (!strcmp (iter->base, lang))
      return TRUE;
    iter = iter->next;
  }
  return FALSE;
}

static void
add_lang (char *language)
{
  Lang *lang = NULL;

  lang = malloc (sizeof (Lang));
  lang->base = language;
  lang->prev = NULL;
  if (lang_list) {
    lang->next = lang_list;
    lang_list->prev = lang;
  } else {
    lang->next = NULL;
  }
  lang_list = lang;
  
}

void
rrn_language_init (char *lang)
{
  char *loc = NULL;
  char *current = NULL;
  char *tmp = NULL;
  Lang *iter = lang_list;
  
  if (lang == NULL) {
    loc = getenv ("LANGUAGE");
    if (!loc || !strcmp (loc, "")) {
      loc = getenv ("LC_ALL");
    }
    if (!loc || !strcmp (loc, "")) {
      loc = getenv ("LANG");
    }
    
  } else {
    loc = strdup (lang);
  }
  nlangs = 0;
  
  if (!loc || !strcmp (loc, "")) {
    loc = strdup("C");
  }
  
  current = loc;
  do {
    Lang *lang;
    char *at_mod;
    char *dot_mod;
    char *ter_mod;
    char *full_lang = NULL;
    char *exploded = NULL;

    tmp = strchr (current, ':');
      
    if (tmp)
      full_lang = rrn_strndup (current, tmp-current);
    else
      full_lang = strdup (current);
      
    at_mod = strrchr (full_lang, '@');
    dot_mod = strrchr (full_lang, '.');
    ter_mod = strrchr (full_lang, '_');
      
    /* Full lang first */
    if (!check_lang (full_lang)) {
      add_lang (full_lang);
    }

    /* Lang sans modifier */
    if (at_mod) {
      exploded = rrn_strndup (full_lang, at_mod - full_lang);
      if (!check_lang (exploded)) {
	add_lang (exploded);
      }
    }

    /* Lang sans modifier and codeset */
    if (dot_mod) {
      exploded = rrn_strndup (full_lang, dot_mod - full_lang);
      if (!check_lang (exploded)) {
	add_lang (exploded);
      }
    }

    /* Lang sans modifier, codeset and territory */
    if (ter_mod) {
      exploded = rrn_strndup (full_lang, ter_mod - full_lang);
      if (!check_lang (exploded)) {
	add_lang (exploded);
      }
    }

    if (tmp)
      current = tmp+1;
    else
      current = NULL;
  } while (current);

  tmp = strdup ("C");
  if (!check_lang (tmp)) {
    add_lang (tmp);
  }

  iter = lang_list;
  while (iter) {
    Lang *tmp = iter->next;
    nlangs += 1;

    iter->next = iter->prev;
    iter->prev = tmp;
    if (iter->prev == NULL) {
      lang_list = iter;
    }
    iter = iter->prev;
  }
  iter = lang_list;

}

int
rrn_language_use (char *current_language, char *proposed)
{
	Lang *iter = lang_list;

	if (!lang_list)
	  rrn_language_init(NULL);

	iter = lang_list;

	while (iter) {
		if (current_language && !strcmp (current_language, iter->base)) {
			return 0;
		}
		if (!strcmp (proposed, iter->base)) {
			return 1;
		}
		iter = iter->next;
	}
	return 0;
}

char **
rrn_language_get_langs (void)
{
	char **ret = NULL;
	Lang *iter;
	int i = 0;
	
	if (!lang_list)
	  rrn_language_init(NULL);

	iter =  lang_list;
	ret = malloc (sizeof (char*) * (nlangs+1));

	while (iter) {
	  ret[i] = iter->base;
	  i++;
	  iter = iter->next;
	}
	ret[i] = NULL;
	return ret;


}

char **
rrn_language_get_dirs (char *base)
{
	char **ret = NULL;
	Lang *iter;
	int i = 0;

	if (!lang_list)
	  rrn_language_init(NULL);
	
	iter = lang_list;

	ret = malloc (sizeof (char*) * (nlangs+1));

	while (iter) {
		char *new_path = NULL;

		new_path = malloc (sizeof (char) *(strlen(base)+9+strlen (iter->base)));
		sprintf (new_path, "%s/LOCALE/%s", base, iter->base);
		ret[i] = new_path;
		i++;
		iter = iter->next;
	}
	ret[i] = NULL;
	return ret;
}


void
rrn_language_shutdown (void)
{
	Lang *iter = lang_list;

	while (iter) {
		Lang *next = iter->next;
		free (iter->base);
		free (iter);
		iter = next;

	}
	lang_list = NULL;
}
