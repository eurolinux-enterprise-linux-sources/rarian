/*
 * rarian-info.c
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

#include "config.h"
#include <stdio.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /*HAVE_MALLOC_H*/
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "rarian-info.h"
#include "rarian-utils.h"

typedef struct _InfoLink InfoLink;


struct _InfoLink
{
  RrnInfoEntry *reg;
  InfoLink *next;
  InfoLink *prev;
};

static InfoLink * info_head = NULL;
static InfoLink * info_tail = NULL;
static char **categories = NULL;
static char *current_category = NULL;
static char *current_path = NULL;
RrnInfoEntry *current_entry = NULL;

static void
set_category (const char *new_cat)
{
  char *stripped = strdup (new_cat);
  char **cats = categories;
  int ncats = 1;
  char **tmp_copy = NULL;
  char **tmp_copy_iter = NULL;
  if (current_category)
    free (current_category);

  stripped = rrn_strip (stripped);
  while (cats && *cats) {
    if (!strcmp (stripped, *cats)) {
      current_category = strdup (*cats);
      free (stripped);
      return;
    }
    ncats++;
    cats++;
  }
  /* If we get here, we got a new category and have to do some work */
  cats = categories;
  tmp_copy = malloc (sizeof(char *) * (ncats+1));
  memset (tmp_copy, 0, sizeof(char *) * (ncats+1));
  tmp_copy_iter = tmp_copy;

  cats = categories;
  while (cats && *cats) {
    char *tmp = *cats;
    *tmp_copy_iter = strdup (tmp);
    cats++;
    tmp_copy_iter++;
    free (tmp);
  }
  *tmp_copy_iter = strdup (stripped);
  current_category = strdup (stripped);
  free (categories);
  categories = tmp_copy;
  free (stripped);
  return;
}

static void
process_initial_entry (const char *line)
{
  char *tmp = (char *) line;
  char *end_name = NULL;
  char *begin_fname = NULL;
  char *end_fname = NULL;
  char *begin_section = NULL;
  char *end_section = NULL;
  char *comment = NULL;

  if (!tmp) {
    fprintf (stderr, "Error: Malformed line!  Ignoring\n");
    return;
  }    

  if (!current_category) {
    /* The docs are appearing before the first category.  Ignore
       them. */
    fprintf (stderr, "Error: Documents outwith categories.  Ignoring\n");
    return;
  }

  tmp++;
  end_name = strchr(tmp, ':');
  if (!end_name) {
    fprintf (stderr, "Error: Malformed line (no ':').  Ignoring entry\n");
    return;
  }
  begin_fname = strchr(end_name, '(');
  if (!begin_fname) {
    fprintf (stderr, "Error: Malformed line (no filename).  Ignoring entry\n");
    return;
  }
  end_fname = strchr(begin_fname, ')');
  if (!end_fname) {
    fprintf (stderr, "Error: Malformed line (no filename close).  Ignoring entry\n");
    return;
  }
  end_section = strchr(end_fname, '.');

  if (!end_section) {
    fprintf (stderr, "Error: Malformed line (no section).  Ignoring entry\n");
    return;
  }

  current_entry->category = strdup(current_category);
  current_entry->base_path = strdup(current_path);
  current_entry->base_filename = NULL;
  current_entry->doc_name = rrn_strip(rrn_strndup(tmp, (end_name - tmp)));
  current_entry->name = rrn_strip(rrn_strndup(begin_fname+1, 
					    (end_fname-begin_fname-1)));
  if (end_section == end_fname+1)
    current_entry->section = NULL;
  else {
    current_entry->section = rrn_strip(rrn_strndup(end_fname+1,
					   (end_section - end_fname -1)));
  }
  comment = rrn_strip(strdup (end_section+1));
  if (strlen (comment) > 0) {
    current_entry->comment = comment;
  } else {
    free(comment);
    current_entry->comment = NULL;
  }
}

static void
process_add_desc (const char *line)
{
  char *current;
  char *cpy = NULL;
  cpy = rrn_strip(strdup(line));
  if (!cpy)
    return;

  if (current_entry->comment) {
    current = malloc (sizeof(char) * (strlen(current_entry->comment) +
				      strlen(cpy) + 2));
    sprintf (current, "%s %s", current_entry->comment, cpy);
    free (current_entry->comment);
  } else {
    current = strdup(cpy);
  }
  current_entry->comment = current;
  free (cpy);
}

static int
process_check_file()
{
  char *filename = NULL;
  char *tmp = NULL;
  InfoLink *iter;
  struct stat fileinfo;

  if (!current_entry->name) {
    return FALSE;
  }

  /* First, look for an additional part on the filename */
  tmp = strchr(current_entry->name, '/');
  if (tmp) {
    char *new_base = NULL;
    char *addition;
    char *name;
    addition = rrn_strndup (current_entry->name, tmp - current_entry->name);
    name = strdup(tmp+1);
    new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
				       strlen(addition)+ 2));
    sprintf (new_base, "%s/%s", current_entry->base_path, addition);
    free(current_entry->base_path);
    free(current_entry->name);
    free(addition);
    current_entry->base_path = new_base;
    current_entry->name = name;
  }

  /* Search for duplicate files.  If we find one, we 
   * use the older one and forget this one
   */
  iter = info_head;

  while (iter) {
    if (!strcmp (iter->reg->doc_name, current_entry->doc_name)) {
      return FALSE;
    }
    iter = iter->next;
    
  }

  
  /* Search for all the types we know of in all the
   * locations we know of to find the file, starting with
   * the most popular and working down.
   * If and when we find it, we set the encoding
   * (loose) and return */
  /* Use the largest possible storage for filename */
  filename = malloc(sizeof(char) * (strlen(current_entry->base_path) +
				    (strlen(current_entry->name)*2) + 15));


  sprintf (filename, "%s/%s.info.gz", current_entry->base_path,
	   current_entry->name);
  if (!stat(filename, &fileinfo)) {
    current_entry->compression = INFO_ENCODING_GZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s.gz", current_entry->base_path,
	   current_entry->name);
  if (!stat(filename, &fileinfo)) {
    current_entry->compression = INFO_ENCODING_GZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s.info.bz2", current_entry->base_path,
	   current_entry->name);
  if (!stat(filename, &fileinfo)) {
    current_entry->compression = INFO_ENCODING_BZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s.bz2", current_entry->base_path,
	   current_entry->name);
  if (!stat(filename, &fileinfo)) {
    current_entry->compression = INFO_ENCODING_BZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s.info.lzma", current_entry->base_path,
		  current_entry->name);
  if (!stat(filename, &fileinfo)) {
    current_entry->compression = INFO_ENCODING_LZMA;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s.lzma", current_entry->base_path,
		  current_entry->name);
  if (!stat(filename, &fileinfo)) {
    current_entry->compression = INFO_ENCODING_LZMA;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s.info", current_entry->base_path,
	   current_entry->name);
  if (!stat(filename, &fileinfo)) {
    current_entry->compression = INFO_ENCODING_NONE;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s/%s.info.gz", current_entry->base_path,
	   current_entry->name, current_entry->name);
  if (!stat(filename, &fileinfo)) {
    /* Add to base path */
    char *new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
					     (strlen(current_entry->name) *2) +
					     2));
    sprintf (new_base, "%s/%s", current_entry->base_path,
	     current_entry->name);
    free(current_entry->base_path);
    current_entry->base_path = new_base;

    current_entry->compression = INFO_ENCODING_GZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s/%s.gz", current_entry->base_path,
	   current_entry->name, current_entry->name);
  if (!stat(filename, &fileinfo)) {
    /* Add to base path */
    char *new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
					     (strlen(current_entry->name) *2) +
					     2));
    sprintf (new_base, "%s/%s", current_entry->base_path,
	     current_entry->name);
    free(current_entry->base_path);
    current_entry->base_path = new_base;

    current_entry->compression = INFO_ENCODING_GZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s/%s.info.bz2", current_entry->base_path,
	   current_entry->name, current_entry->name);
  if (!stat(filename, &fileinfo)) {
    /* Add to base path */
    char *new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
					     (strlen(current_entry->name) *2) +
					     2));
    sprintf (new_base, "%s/%s", current_entry->base_path,
	     current_entry->name);
    free(current_entry->base_path);
    current_entry->base_path = new_base;

    current_entry->compression = INFO_ENCODING_BZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
  sprintf (filename, "%s/%s/%s.bz2", current_entry->base_path,
	   current_entry->name, current_entry->name);
  if (!stat(filename, &fileinfo)) {
    /* Add to base path */
    char *new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
					     (strlen(current_entry->name) *2) +
					     2));
    sprintf (new_base, "%s/%s", current_entry->base_path,
	     current_entry->name);
    free(current_entry->base_path);
    current_entry->base_path = new_base;

    current_entry->compression = INFO_ENCODING_BZIP;
    current_entry->base_filename = filename;
    return TRUE;
  }
    sprintf (filename, "%s/%s/%s.info.lzma", current_entry->base_path,
		    current_entry->name, current_entry->name);
    if (!stat(filename, &fileinfo)) {
    /* Add to base path */
    char *new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
					    (strlen(current_entry->name) *2) +
					    2));
    sprintf (new_base, "%s/%s", current_entry->base_path,
		    current_entry->name);
    free(current_entry->base_path);
    current_entry->base_path = new_base;
    
    current_entry->compression = INFO_ENCODING_LZMA;
    current_entry->base_filename = filename;
    return TRUE;
    }
        
    sprintf (filename, "%s/%s/%s.lzma", current_entry->base_path,
		    current_entry->name, current_entry->name);
    if (!stat(filename, &fileinfo)) {
	    /* Add to base path */
    char *new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
					    (strlen(current_entry->name) *2) +
					    2));
    sprintf (new_base, "%s/%s", current_entry->base_path,
    current_entry->name);
    free(current_entry->base_path);
    current_entry->base_path = new_base;
    
    current_entry->compression = INFO_ENCODING_LZMA;
    current_entry->base_filename = filename;
    return TRUE;
    }

  sprintf (filename, "%s/%s/%s.info", current_entry->base_path,
	   current_entry->name, current_entry->name);
  if (!stat(filename, &fileinfo)) {
    /* Add to base path */
    char *new_base = malloc (sizeof(char) * (strlen(current_entry->base_path) +
					     (strlen(current_entry->name) *2) +
					     2));
    sprintf (new_base, "%s/%s", current_entry->base_path,
	     current_entry->name);
    free(current_entry->base_path);
    current_entry->base_path = new_base;

    current_entry->compression = INFO_ENCODING_NONE;
    current_entry->base_filename = filename;
    return TRUE;
  }
  free(filename);
  return FALSE;
  

}

static void
free_entry (RrnInfoEntry *entry)
{
  if (entry->name)
    free(entry->name);
  if (entry->base_path)
    free(entry->base_path);
  if (entry->base_filename)
    free (entry->base_filename);
  if (entry->category)
    free (entry->category);
  if (entry->section)
    free(entry->section);
  if (entry->doc_name)
    free(entry->doc_name);
  if (entry->comment)
    free(entry->comment);
  free(entry);

}

static void
process_add_entry (void)
{
  InfoLink *link = NULL;

  link = malloc (sizeof(InfoLink));
  link->reg = current_entry;
  link->next = NULL;
  link->prev = NULL;
  if (info_tail && info_head) {
    info_tail->next = link;
    link->prev = info_tail;
    info_tail = link;
  } else {
    /* Initial link */
    info_head = info_tail = link;
  }
}

static void
process_info_dir (const char *dir)
{

  char *filename;
  FILE *fp = NULL;
  char *line = NULL;
  int started = FALSE;
  int ret = FALSE;

  filename = (char *) malloc(sizeof(char) * strlen(dir)+5);

  sprintf(filename, "%s/dir", dir);
  fp = fopen(filename, "r");
  if (!fp) {
    free (filename);
    return;
  }
  if (current_path)
    free (current_path);
  current_path = strdup (dir);
  line = (char *) malloc(sizeof(char) * 1024);
  while (fgets (line, 1023, fp)) {
    if (!started) {
      if (!strncmp (line, "* Menu", 6) ||
	  !strncmp (line, "* menu", 6)) {
	started = TRUE;
      }
      continue;
    }
    if ((*line != '*') && !isspace(*line)) {
      /* New category */
      set_category (line);
    } else if ((*line == '*')) {
      /* New entry */
      if (current_entry) {
	if (process_check_file()) {
	  process_add_entry ();
	} else {
	  free_entry (current_entry);
	}
	current_entry = NULL;
      }
      current_entry = malloc (sizeof(RrnInfoEntry));
      current_entry->name = NULL;
      current_entry->base_path = NULL;
      current_entry->base_filename = NULL;
      current_entry->category = NULL;
      current_entry->section = NULL;
      current_entry->doc_name = NULL;
      current_entry->comment = NULL;


      process_initial_entry (line);
      

    } else if (strlen(line) > 1) {
      /* Continuation of description */
      process_add_desc (line);
    } else {
      /* Blank line, ignore */
    }
  }
  if (process_check_file()) {
    process_add_entry ();
  } else {
    free_entry (current_entry);
  }
  current_entry = NULL;
  free (line);
  fclose(fp);
  free (filename);
}

static void
sanity_check_categories ()
{
  char **cats = categories;
  char **iter = cats;
  char **new_cats = NULL;
  InfoLink *l;
  int ncats = 1;

  while (iter && *iter) {
    l = info_head;
    while (l) {
      if (!strcmp(l->reg->category, *iter)) {
	char **tmp = NULL;
	char **cats_iter = new_cats;
	char **tmp_iter = NULL;
	ncats++;
	tmp = (char **) malloc(sizeof(char *) * (ncats+1));
	memset(tmp, 0, sizeof(char *) * (ncats+1));
	tmp_iter = tmp;
	while (cats_iter && *cats_iter) {
	  *tmp_iter = *cats_iter;
	  tmp_iter++;
	  cats_iter++;
	}
	*tmp_iter = *iter;
	tmp_iter = tmp;
	if (new_cats)
	  free(new_cats);
	new_cats = tmp;

	break;
      }
      l = l->next;
    }
    /* If we got here, we have an empty category and should remove 
     * it 
     */
    iter++;
  }
  free(categories);
  categories = new_cats;
}

static void
rrn_info_init (void)
{
  char *default_dirs = "/usr/info:/usr/share/info:/usr/local/info:/usr/local/share/info";
  char *info_dirs = NULL;
  char *split = NULL;
  int free_info_dirs = FALSE;

  info_dirs = (char *) getenv ("INFOPATH");
  

  if (!info_dirs || !strcmp (info_dirs, "")) {
    free_info_dirs = TRUE;
    info_dirs = strdup (default_dirs);
  }
  
  split = info_dirs;
  do {
    char *next = strchr(split, ':');
    char *dirname = NULL;

    if (next)
      dirname = rrn_strndup(split, (next-split));
    else
      dirname = strdup (split);
    process_info_dir (dirname);
    free (dirname);

    split = strchr(split, ':');
    if (split)
      split++;
  } while (split);
  
  if (free_info_dirs)
    free (info_dirs);

  /* Sanity check our categories.  Yes, I put this
  * comment in then came up with the function name */
  sanity_check_categories ();
  
}


char **
rrn_info_get_categories (void)
{
  if (!categories)
    rrn_info_init();
  return categories;

}

void rrn_info_for_each (RrnInfoForeachFunc funct, void * user_data)
{
  InfoLink *l;
  if (!categories)
    rrn_info_init();
  l = info_head;
  while (l) {
    int res;
    res = funct (l->reg, user_data);
    if (res == FALSE)
      break;
    l = l->next;
  }  
  return;
}


void 
rrn_info_for_each_in_category (char *category, 
				 RrnInfoForeachFunc funct, 
				 void * user_data)
{
  InfoLink *l;
  if (!categories)
    rrn_info_init();
  l = info_head;
  while (l) {
    int res;
    if (!strcmp (l->reg->category, category)) {
      res = funct (l->reg, user_data);
      if (res == FALSE)
	break;
    }
    l = l->next;
  }  
  return;

}

RrnInfoEntry *
rrn_info_find_from_uri (char *uri, char *section)
{
  InfoLink *l;
  InfoLink *best_result = NULL;
  if (!categories)
    rrn_info_init();

  l = info_head;

  while (l) {
    if ((l->reg->doc_name && !strcmp (uri, l->reg->doc_name)) ||
	(!strcmp (uri, l->reg->name))) {
      if (!section || (*section && l->reg->section && !strcmp (l->reg->section, section))) {
	return l->reg;
      } else {
	best_result = l;
      }
    }
    l = l->next;
  }
  
  if (best_result)
     return best_result->reg;

  return NULL;
}

void 
rrn_info_shutdown ()
{
  InfoLink *l = info_head;

  while (l) {
    InfoLink *next = l->next;
    free_entry (l->reg);
    free (l);
    l = next;
  }
  info_head = info_tail = NULL;

  free(categories);
  categories = NULL;
}
