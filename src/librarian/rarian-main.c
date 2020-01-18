/*
 * rarian-main.c
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
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>

#include "config.h"

#include "rarian.h"
#include "rarian-reg-utils.h"
#include "rarian-language.h"
#include "rarian-utils.h"
#if ENABLE_OMF_READ
#include "rarian-omf.h"
#endif

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

/* Internal structures and lists */

typedef struct _Link Link;


struct _Link
{
	union {
	    RrnReg *reg;
		RrnSect *sect;
		} reg;
    Link *next;
    Link *prev;
};

static Link * head = NULL;
static Link * tail = NULL;

static Link *orphans_head = NULL;
static Link *orphans_tail = NULL;

/* Function Prototypes */

static void rrn_init        (void);
static void scan_directories  (void);
static void scan_directory    (char *dir);
static void process_file      (char *filename);
static void process_section   (char *filename);
static void insert_orphans    (void);
static void reverse_children  (void);
static void process_locale_dirs (char * dir);
#if ENABLE_OMF_READ
static void process_omf_dir     (char *dir);
#endif

void 
rrn_set_language (char *lang_code)
{
  if (head) {
    rrn_shutdown ();
  }
  rrn_language_init (lang_code);
  rrn_init ();
}

static void
rrn_init (void)
{
    scan_directories ();

    return;
}

void
rrn_for_each (RrnForeachFunc funct, void * user_data)
{
    Link *iter;

    if (!head) {
    	rrn_init ();
    }

    iter = head;

    while (iter) {
        int res;
        res = funct (iter->reg.reg, user_data);
        if (res == FALSE)
            break;
        iter = iter->next;
    }

    return;
}

void rrn_for_each_in_category (RrnForeachFunc funct, char * category,
				 void *user_data)
{
    Link *iter;

    if (!head) {
    	rrn_init ();
    }
    iter = head;

    while (iter) {
        int res;
	char **cats;

	cats = iter->reg.reg->categories;
	while (cats && *cats) {
	  if (!strcmp(*cats, category)) {
	    res = funct (iter->reg.reg, user_data);
	    if (res == FALSE)
	      break;
	  }
	  cats++;
	}
        iter = iter->next;
    }

    return;

}

RrnReg * 
rrn_find_entry_from_uri (char *uri)
{
    Link *iter;

    if (!head) {
    	rrn_init ();
    }
    iter = head;

    while (iter) {

      if (!strcmp(iter->reg.reg->uri, uri))
	return iter->reg.reg;
      iter = iter->next;
    }

    return NULL;
}

static void
scan_directories (void)
{
  char *cur_path = NULL;

#if ENABLE_INSTALL
    char *path = NULL;
    char *first_colon = NULL;
	char *next_colon = NULL;
    char *home_dir = NULL;
    char *home_data_dir = NULL;
    char *home_env = NULL;

    home_env = getenv ("XDG_DATA_HOME");
    if (home_env)
      home_data_dir = strdup(home_env);

    if (!home_data_dir || !strcmp (home_data_dir, "")) {
        home_dir = getenv ("HOME");
        if (!home_dir || !strcmp (home_dir, "")) {
            fprintf (stderr, "Warning: HOME dir is not defined."
                     "  Skipping check of XDG_DATA_HOME");
            goto past;
        }
        home_data_dir = malloc (sizeof(char) * (strlen(home_dir)+14));
        sprintf (home_data_dir, "%s/.local/share", home_dir);
    }

    /* Reuse home_dir.  Bad.*/
    home_dir = malloc (sizeof (char) * (strlen(home_data_dir)+6));

    sprintf (home_dir, "%s/help", home_data_dir);

#if ENABLE_OMF_READ
    process_omf_dir (home_data_dir);
#endif

    free (home_data_dir);

    process_locale_dirs (home_dir);
    scan_directory (home_dir);

    free (home_dir);

past:
    path = getenv ("XDG_DATA_DIRS");

    if (!path || !strcmp (path, "")) {
        path = "/usr/local/share/:/usr/share/";
    }
    cur_path = path;

    do {
    	char *int_path = NULL;
    	char *check_path = NULL;
	    first_colon = strchr (cur_path, ':');
	    if (first_colon)
	      int_path = rrn_strndup (cur_path, (first_colon-cur_path));
	    else
	      int_path = strdup (cur_path);
		check_path = malloc (sizeof(char)*(strlen(int_path)+6));
		sprintf (check_path, "%s/help", int_path);
#if ENABLE_OMF_READ
		process_omf_dir (int_path);
#endif
		process_locale_dirs (check_path);

		scan_directory (check_path);
		if (int_path && *int_path) {
			free (int_path);
		}
		if (check_path) {
			free (check_path);
		}
		cur_path = first_colon+1;
	} while (first_colon);
#else
	cur_path = "data/sk-import";
	process_locale_dirs (cur_path);
	scan_directory (cur_path);
#endif
	reverse_children ();
}

static void
process_locale_dirs (char * dir)
{
	DIR *dirp = NULL;
	char **paths_to_check = NULL;
	char **iter = NULL;

	paths_to_check = rrn_language_get_dirs (dir);
	iter = paths_to_check;

	while (*iter) {
		scan_directory (*iter);
		free (*iter);
		iter++;
	}
	free (paths_to_check);

}

static void
scan_directory (char *dir)
{
    DIR * dirp = NULL;
    struct dirent * dp = NULL;
    struct stat buf;
    char *path = NULL;
    dirp = opendir (dir);

    if (access (dir, R_OK)) {
    	return;
    }
    while (1) {
      if ((dp = readdir(dirp)) != NULL) {
	char *full_name = NULL;
	full_name = malloc (sizeof(char)*(strlen (dp->d_name) + strlen(dir) + 2));
	
	sprintf (full_name, "%s/%s", dir, dp->d_name);
	stat(full_name,&buf);

	if (S_ISREG(buf.st_mode)) {
	  char *suffix = NULL;
	  
	  suffix = strrchr (full_name, '.');
	  if (!strcmp (suffix, ".document")) {
	    process_file (full_name);
	  } else if (!strcmp (suffix, ".section")) {
	    process_section (full_name);
	  }
	} else if (S_ISDIR(buf.st_mode) && strcmp (dp->d_name, ".") &&
		   strcmp (dp->d_name, "..") &&
		   strcmp (dp->d_name, "LOCALE")) {

	  scan_directory (full_name);
	}
	free (full_name);
      } else {
	goto done;
      }
    }

done:
	insert_orphans ();
    closedir (dirp);
    free (path);
}

static int
handle_duplicate (RrnReg *reg)
{
    Link *iter;

    iter = head;

    while (iter) {
      if ((iter->reg.reg->heritage && reg->heritage &&
	   !strcmp (iter->reg.reg->heritage, reg->heritage)) ||
	  !strcmp (iter->reg.reg->identifier, reg->identifier)) {
	if (iter->reg.reg->lang && reg->lang &&
	    rrn_language_use (iter->reg.reg->lang, reg->lang)) {
	  rrn_reg_free (iter->reg.reg);
	  iter->reg.reg = reg;
	}
	return TRUE;
      }
      iter = iter->next;
    }

    return FALSE;

}

#if ENABLE_OMF_READ
static void 
process_omf_dir (char *dir)
{
  char *path;
  DIR * dirp = NULL;
  char **langs = NULL;
  char **langs_iter = NULL;
  int lang_found = FALSE;
  int lang_count = 0;
  char *tmp = NULL;

  struct dirent * dp = NULL;
  struct stat buf;
  
  langs = rrn_language_get_langs ();
  path = malloc (sizeof(char) * (strlen (dir)+6));
  
  sprintf (path, "%s/omf", dir);
  
  if (access (path, R_OK)) {
    return;
  }

  langs_iter = langs;
  while (langs_iter && *langs_iter) {
    lang_count++;
    if (!strcmp (*langs_iter, "C")) {
	lang_found = TRUE;
      }    
    langs_iter++;
  }
  if (!lang_found) {
    char **tmp;
    int i = 0;
    tmp = malloc (sizeof (char *) * (lang_count+2));
    langs_iter = langs;
    while (langs_iter && *langs_iter) {
      tmp[i] = strdup (*langs_iter);
      i++;
      langs_iter++;
    }
    tmp[i] = strdup ("C");
    i++;
    tmp[i] = NULL;
    langs = tmp;
  }
  

  dirp = opendir (path);

  while (1) {
    if ((dp = readdir(dirp)) != NULL) {
      char *full_name;
      full_name = malloc (sizeof(char) * (strlen(path) + strlen(dp->d_name) + 5));
    sprintf (full_name, "%s/%s", path, dp->d_name);
      stat(full_name,&buf);
      free (full_name);
      if (S_ISDIR(buf.st_mode) && strcmp (dp->d_name, ".") &&
	  strcmp (dp->d_name, "..")) {
	langs_iter = langs;
	while (langs_iter && *langs_iter) {
	  char *lang = (*langs_iter);
	  /* Add extra 2 for separator and NULL.  Otherwise, it falls over */
	  tmp = malloc (sizeof (char) * (strlen(dir)+(strlen(dp->d_name)*2) +
					 strlen(lang) + 20));
	  sprintf (tmp, "%s/%s/%s-%s.omf", path, dp->d_name, dp->d_name,(*langs_iter));

	  if (!access (tmp, R_OK)) {
	    RrnReg *reg = NULL;
	    reg = rrn_omf_parse_file (tmp);
	    if (reg) {
	      reg->omf_location = strdup (tmp);
	      reg->ghelp_name = strdup (dp->d_name);
	    }
	    if (reg && !handle_duplicate (reg)) {
	      Link *link;

	      link = malloc (sizeof (Link));
	      link->reg.reg = reg;
	      link->next = NULL;
	      
	      if (!tail) {
		if (head) {
		  fprintf (stderr, "ERROR: Tail not pointing anywhere.  "
			   "Aborting");
		  exit (3);
		}
		head = link;
		tail = link;
	      } else {
		tail->next = link;
		tail = link;
		
	      }
	    } 
	  }
	  free (tmp);
	  tmp = NULL;
	  langs_iter++;
	}
      }
    } else {
      break;
    }
  }
done:
  insert_orphans ();
  closedir (dirp);
}
#endif

static void
process_section (char *filename)
{
	RrnSect *sect = NULL;
	Link *link;

	sect = rrn_sect_parse_file (filename);
	if (!sect)
		return;

	link = malloc (sizeof (Link));
    link->reg.sect = sect;
    link->next = NULL;
    link->prev = NULL;

	if (!orphans_head) {
		orphans_head = link;
		orphans_tail = link;
	} else {
		orphans_tail->next = link;
		link->prev = orphans_tail;
		orphans_tail = link;
	}
}

static void
process_file (char *filename)
{
    RrnReg *reg;
    Link *link;

    reg = rrn_reg_parse_file (filename);
    if (!reg)
        return;

	if (handle_duplicate (reg)) {
	  return;
	}

    link = malloc (sizeof (Link));
    link->reg.reg = reg;
    link->next = NULL;

    if (!tail) {
        if (head) {
            fprintf (stderr, "ERROR: Tail not pointing anywhere.  Aborting");
            exit (3);
        }
        head = link;
        tail = link;
    } else {
        tail->next = link;
        tail = link;

    }
}

static void
insert_orphans ()
{
	Link *sect = orphans_head;

	while (sect) {
    	Link *iter = head;

	    while (iter) {
	    	if (!strncmp (iter->reg.reg->identifier, sect->reg.sect->owner,
	    				  strlen(iter->reg.reg->identifier))) {
				break;
			}
			iter = iter->next;
		}
		if (iter) {
			sect->reg.sect = rrn_reg_add_sections (iter->reg.reg,
								 sect->reg.sect);
			if (sect->reg.sect == NULL) {
				Link *tmp = sect->next;
				if (sect->prev)
					sect->prev->next = sect->next;
				if (sect->next)
					sect->next->prev = sect->prev;
				if (sect == orphans_head)
					orphans_head = NULL;
				free (sect);
				sect = tmp;
			}
		} else {
			sect->reg.sect->priority++;
			sect = sect->next;
		}
	}
}

static RrnSect *
reverse_child (RrnSect *child)
{
	RrnSect *local_tail = NULL;
	RrnSect *iter = child;
	RrnSect *tmp = NULL;

	while (iter) {
		if (iter->children)
			iter->children = reverse_child (iter->children);
		tmp = iter->next;
		iter->next = iter->prev;
		iter->prev = tmp;
		if (iter->prev == NULL) {
			return iter;
		}
		iter = iter->prev;
	}
}

static void
reverse_children  ()
{
	Link *iter = head;

	while (iter) {
		if (iter->reg.reg->children) {
			iter->reg.reg->children = reverse_child (iter->reg.reg->children);
		}

		iter = iter->next;
	}
}

RrnReg * 
rrn_find_from_name (char *name)
{
  if (!head)
    rrn_init ();

  return NULL;


}

RrnReg * 
rrn_find_from_ghelp (char *ghelp)
{
    Link *iter;

    if (!head) {
    	rrn_init ();
    }
    iter = head;

    while (iter) {
      if (iter->reg.reg->ghelp_name && !strcmp(iter->reg.reg->ghelp_name, ghelp))
	return iter->reg.reg;
      iter = iter->next;
    }

    return NULL;
}


void
rrn_shutdown ()
{
    Link *next;

    while (head) {
        next = head->next;

        rrn_reg_free (head->reg.reg);
        free (head);
        head = next;
    }
    rrn_language_shutdown ();
    head = tail = NULL;
    return;
}
