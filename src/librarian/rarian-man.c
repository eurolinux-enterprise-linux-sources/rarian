/*
 * rarian-man.c
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "rarian-man.h"
#include "rarian-language.h"
#include "rarian-utils.h"

/*#ifdef HAVE_LIBGDBM
#include <gdbm.h>
static GDBM_FILE *gdbm_handle;
#endif

#ifdef HAVE_LIBNDBM
#include <ndbm.h>
static DBM *ndbm_handle;
#endif*/

typedef struct _ManLink ManLink;

struct _ManLink
{
  RrnManEntry *reg;
  ManLink *next;
  ManLink *prev;
};

static char *keys[43] = {"1", "1p", "1g", "1t", "1x", "1ssl", "1m",
    "2",
    "3", "3o", "3t", "3p", "3blt", "3nas", "3form", "3menu", "3tiff", "3ssl", "3readline",
    "3ncurses", "3curses", "3f", "3pm", "3perl", "3qt", "3x", "3X11",
    "4", "4x", 
    "5", "5snmp", "5x", "5ssl",
    "6", "6x",
    "7", "7gcc", "7x", "7ssl",
    "8", "8l", "9", "0p" 
};

static ManLink *manhead[44];
static ManLink *mantail[44];

static char *avail_dirs[] = {
    "man0p", "man1", "man1p", "man2",  "man3",
    "man3p", "man4", "man5", "man6", "man7", 
    "man8", "man9", "mann", 
    NULL
    };

static char **man_paths = NULL;

int initialised = FALSE;
int mode = -1;
/* Modes: 0 = default
 *        1 = gdbm
 *        2 = ndbm
 */

static void
setup_man_path ()
{
  int outfd[2];
  int infd[2];
  
  int oldstdin, oldstdout;
  fflush(stdin);
  fflush(stdout);
  fflush(stderr);

  pipe(outfd); // Where the parent is going to write to
  pipe(infd); // From where parent is going to read
  
  oldstdin = dup(0); // Save current stdin
  oldstdout = dup(1); // Save stdout
  
  close(0);
  close(1);
  
  dup2(outfd[0], 0); // Make the read end of outfd pipe as stdin
  dup2(infd[1],1); // Make the write end of infd as stdout

  if(!fork()) {
    /* Child process */
    char *argv[]={"manpath"};
    close(outfd[0]); // Not required for the child
    close(outfd[1]);
    close(infd[0]);
    close(infd[1]);
    execlp("manpath", "manpath", (char *) 0);
    exit (0);
  } else {
    /* Parent process */
    char *input = NULL;
    char *colon = NULL;
    char *next = NULL;
    int i, count = 0;
    input = malloc(sizeof(char) * 256);
    close(0); // Restore the original std fds of parent
    close(1);
    dup2(oldstdin, 0);
    dup2(oldstdout, 1);
    
    close(outfd[0]); // These are being used by the child
    close(infd[1]);
    memset(input, 0, sizeof(char)*255);
    
    input[read(infd[0],input,255)] = 0; // Read from child's stdout
    if (*input != '\0') {
      int i;
      i = strlen(input);
      input[i-1]='\0';
    }
    if (!input || *input == '\0') {
      char *env = NULL;
      env = getenv("MANPATH");
      if (env)
	input = strdup(env);
    }
    if (!input || *input == '\0') {
      if (input)
	free(input);
      /* Hard coded default.  Don't argue. */
      input = strdup ("/usr/share/man:/usr/man:/usr/local/share/man:/usr/local/man");
    }
    colon = input;

    while (*colon) {
      if (*colon == ':') 
	count++;
      colon++;
    }
    man_paths = malloc (sizeof(char *) * (count+2)); /* 2 is for final string
						      * + NULL entry */
    colon = input;
    for (i=0; i< count; i++) {
      next = strchr (colon, ':');
      man_paths[i] = rrn_strndup(colon, next-colon);
      colon = next;
      colon++;
    }
    /* Final 2 entries - straight strdup of final entry and NULL */
    man_paths[i] = strdup(colon);
    i++;
    man_paths[i] = NULL;
    free (input);
  }

}
#if 0
#ifdef HAVE_LIBGDBM
static void
setup_gdbm (char *name)
{
  gdbm_handle = gdbm_open(name, 0, GDBM_READER, 0666, 0);
  if (!gdbm_handle) {
    sprintf(stderr, "ERROR: GDBM index %s could not be opened.  Falling back\n", name);
    setup_default();
  } else {
    initialised = TRUE;
    mode = 1;
  }
}
#endif

#ifdef HAVE_LIBNDBM
static void
setup_ndbm (char *name)
{
  char *trunc;

  trunc = malloc (sizeof(char) * (strlen(name)-3));
  trunc = rrn_strndup (name, strlen(name) - 4);
  ndbm_handle = dbm_open(trunc, O_RDONLY, 0666);
  if (!ndbm_handle) {
    sprintf(stderr, "ERROR: NDBM index %s could not be opened.  Falling back\n", trunc);
    setup_default();
  } else {
    initialise = TRUE;
    mode = 2;
  }
  free (trunc);
}
#endif
#endif

static char *strrstr (char *s, char *wanted)
{
    char *scan;
    char *first;
    int len;

    first = wanted;
    len = strlen(wanted);
    for (scan = s + strlen(s) - len ; scan >= s ; scan--)
    {
        if (*scan == *first)
        {
            if (!strncmp (scan, wanted, len))
            {
                return (scan);
            }
        }
    }
    return(NULL);
}



static char *
get_name_for_file (char *filename, char **subsect)
{
  char *suffix;
  char *cut;
  char *sect;
  char *final;

  /* We assume, like reasonable people, that man pages
   * have one of the forms:
   * manname.sect.{gz,bz,bz2,lzma}
   * manname.sect
   * If it doesn't, things will probably break but we return
   * our "best guess" (i.e. everything up to the suffix)
   */
  suffix = strrstr(filename, ".gz");
  if (!suffix) {
    suffix = strrstr(filename, ".bz2");
    if (!suffix) {
      suffix = strrstr(filename, ".bz");
    }
      if (!suffix) {
        suffix = strrstr(filename, ".lzma");
      }
  }
  if (suffix)
    cut = rrn_strndup (filename, suffix-filename);
  else
    cut = strdup (filename);
  sect = strrchr (cut, '.');
  if (!sect)
    return cut;

  final = rrn_strndup (cut, sect-cut);
  sect++;
  *subsect = strdup (sect);
  free (cut);
  return final;

}

static int
check_for_dup (RrnManEntry *reg, int entry)
{
  ManLink *iter = manhead[entry];

  while (iter) {
    if (!strcmp(reg->name, iter->reg->name))
      return TRUE;
    iter = iter->next;

  }
  return FALSE;
}

static int
find_key (char *sect)
{
  int i;
  for (i=0; i<43; i++){
    if (!strcmp (sect, keys[i]))
      return i;
  }
  return i;
}

static void
process_dir(char *dir)
{
  char **dir_iter;
  char *path = NULL;

  DIR * dirp = NULL;
  struct dirent * dp = NULL;
  struct stat buf;
  int current;
  
  current = -1;
  path = malloc(sizeof(char) * (strlen(dir) + 8));

  dir_iter = avail_dirs;
  while (dir_iter && *dir_iter) {
    current++;
    sprintf(path, "%s/%s", dir, *dir_iter);
    if (access (path, R_OK)) {
      dir_iter++;
      continue;
    }

    dirp = opendir (path);
    
    if (!dirp) {
      dir_iter++;
      continue;
    }

    while (1) {
      if ((dp = readdir(dirp)) != NULL) {
	char *full_name;
	
	full_name = malloc (sizeof(char) * (strlen(dp->d_name) + strlen (path) + 3));
	sprintf (full_name, "%s/%s", path, dp->d_name);
			    
	stat(full_name,&buf);
	if (S_ISREG(buf.st_mode) || buf.st_mode & S_IFLNK) {
	  char *tmp = NULL;
	  char *suffix = NULL;
	  RrnManEntry *entry;
	  ManLink *link;
	  char *subsect = NULL;
	  	  
	  entry = malloc (sizeof(RrnManEntry));
	  entry->name = get_name_for_file (dp->d_name, &subsect);
	  entry->path = full_name;
	  if (subsect) {
	    entry->section = subsect;
	    entry->comment = NULL;
	    current = find_key(subsect);
	    if (!check_for_dup (entry, current)) {
	      link = malloc (sizeof(ManLink));
	      link->reg = entry;
	      
	      if (mantail[current]) {
		mantail[current]->next = link;
		link->next = NULL;
		link->prev = mantail[current];
		mantail[current] = link;
	      } else {
		manhead[current] = mantail[current] = link;
		link->prev = link->next = NULL;
	      }
	    } else {
	      free (entry->name);
	      free (entry->path);
	      free (entry->section);
	      if (entry->comment)
		free (entry->comment);
	      free (entry);
	    }
	  }	  
	}
      } else {
	closedir (dirp);
	break;
      }
    }
    dir_iter++;
  }
  free (path);
}

static void
setup_default()
{
  char **path_iter;
  char **langs;
  char **lang_iter;

  path_iter = man_paths;
  langs = rrn_language_get_langs();

  while (path_iter && *path_iter) {
    if (access (*path_iter, R_OK)) {
      path_iter++;
      continue;
    }

    lang_iter = langs;
    while (lang_iter && *lang_iter) {
      char *path;
      path = malloc (sizeof(char) * (strlen(*path_iter) + 
				     strlen(*lang_iter)+2));
      sprintf (path, "%s/%s", *path_iter, *lang_iter);
      if (!access (path, R_OK)) {
	process_dir(path);
      }
      free (path);
      lang_iter++;
    }
    

    process_dir(*path_iter);
    path_iter++;
  }

  /*lang_iter = langs;
  while (lang_iter && *lang_iter) {
    char **next = lang_iter;
    next++;
    free (lang_iter);
    lang_iter = next;
    }*/
  free (langs);
}

static void
rrn_man_init (void)
{
  char *default_dirs = "/var/cache/man:/usr/man";
  char *split = NULL;
#if 0
  char *gdbm_index = "index.db";
  char *ndbm_index = "index.dir";
#endif
  int i;
  int npaths = 0;
  char *ddirs;

  for (i=0; i<44; i++) {
    manhead[i] = mantail[i] = NULL;
  }

  setup_man_path ();

  split = default_dirs;

#if 0
  while (split) {
    char *next = strchr(split, ':');
    char *dirname = NULL;
    char *index = NULL;
    FILE *access = NULL;
    
    if (next)
      dirname = rrn_strndup(split, (next-split));
    else
      dirname = strdup (split);
#ifdef HAVE_LIBGDBM
    index = malloc (sizeof(char) * (strlen(dirname) + 10 /*Len of gdbm_index + additional chars*/));
    sprintf (index, "%s/%s", dirname, gdbm_index);
    access = fopen(index, "r");
    free(dirname);
    if (access) {
      fclose(access);
      setup_gdbm(index);
      free(index);
      return;
    }
#endif
#ifdef HAVE_NDBM
    index = malloc (sizeof(char) * (strlen(dirname) + 11 /*Len of ndbm_index + additional chars*/));
    sprintf (index, "%s/%s", dirname, gdbm_index);
    access = fopen(index, "r");
    free(dirname);
    if (access) {
      fclose(access);
      setup_ndbm(index);
      free(index);
      return;
    }
#endif
    split = strchr(split, ':');
    if (split)
      split++;
  }
#endif /* 0 */
  /* If we get here, we have to do our own thang */
  setup_default();
  initialised = TRUE;

}

void 
rrn_man_for_each (RrnManForeachFunc funct, void * user_data)
{
  ManLink *iter;
  int i;

  if (!initialised)
    rrn_man_init();
  
  for (i=0; i<44; i++) {
    iter = manhead[i];

    while (iter) {
      int res;
      res = funct (iter->reg, user_data);
      if (res == FALSE)
	break;
      iter = iter->next;
    }  
  }
  return;
}


void 
rrn_man_for_each_in_category (char *category, 
				RrnManForeachFunc funct, 
				void * user_data)
{
  ManLink *iter;
  int cat;

  if (!initialised)
    rrn_man_init();

  cat = find_key(category);
  iter = manhead[cat];
    
  while (iter) {
    int res;
    if (!strcmp (iter->reg->section, category)) {
      res = funct (iter->reg, user_data);
      if (res == FALSE)
	break;
    }
    iter = iter->next;
  }  
  return;
}

RrnManEntry *
rrn_man_find_from_name (char *name, char *sect)
{
  ManLink *iter;
  int cat;
  RrnManEntry *res;

  if (!initialised)
    rrn_man_init ();

  if (sect) {
    cat = find_key(sect);
  } else {
    int i=0;
    for (i=0; i<43; i++) {
      iter = manhead[i];
    
      while (iter) {
	if (!strcmp (iter->reg->name, name)) {
	  return iter->reg;
	}
	iter = iter->next;
      }  
    }
    return NULL;
  }
  iter = manhead[cat];
    
  while (iter) {
    if (!strcmp (iter->reg->name, name)) {
      return iter->reg;
    }
    iter = iter->next;
  }  


  return NULL;
}

char **
rrn_man_get_categories (void)
{
  if (!initialised)
    rrn_man_init();

  return keys;
}


void 
rrn_man_shutdown ()
{
  ManLink *iter;
  int i;
  initialised = FALSE;

  for (i=0; i< 44; i++) {
    iter = manhead[i];
    while (iter) {
      ManLink *tmp = iter->next;
      free (iter->reg->name);
      free (iter->reg->path);
      free (iter->reg->section);
      if (iter->reg->comment)
	free (iter->reg->comment);
      free (iter->reg);
      free (iter);
      iter = tmp;
    }
    manhead[i] = mantail[i] = NULL;
  }
  rrn_language_shutdown ();
  return;
}
