/*
 * rarian-reg-full.c
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
#include <string.h>
#include <ctype.h>

/* TODO: Figure out why this is required here */
#define I_KNOW_RARIAN_0_8_IS_UNSTABLE
#include "rarian-reg-full.h"
#include "rarian-reg-utils.h"
#include "rarian-utils.h"


static void       process_line_full (char *line, RrnRegFull *reg);
static void       process_pair_full (RrnRegFull *reg, char *key, char *value);

static void     process_sect_line_full (char *line, RrnSectFull *sect);
static void     process_sect_pair_full (RrnSectFull *sect, char *key, 
					char *value);
static int      rrn_reg_add_sect_full (RrnRegFull *reg, RrnSectFull *sect);
static RrnSectFull * rrn_sect_new_full ();
static void     rrn_sect_free_full (RrnSectFull *sect);
static void     process_path_full (RrnRegFull *reg);
static void     process_section_path_full (RrnListEntry *owner_paths, RrnSectFull *section);

RrnRegFull *
rrn_reg_new_full ()
{
    RrnRegFull *reg;

    reg = malloc (sizeof (RrnRegFull));
    reg->name = NULL;
    reg->uri = NULL;
    reg->comment = NULL;
    reg->identifier = NULL;
    reg->type        = NULL;
    reg->icon        = NULL;
    reg->categories  = NULL;
    reg->weight   = 0;
    reg->heritage    = NULL;
    reg->lang = NULL;
    reg->children = NULL;
    
    return reg;
}

RrnRegFull *
rrn_reg_parse_file_full (char *filename)
{
    RrnRegFull *reg = NULL;
    RrnSectFull *sect = NULL;
    RrnSectFull *orphaned_sections = NULL;
    int mode = 0;
    FILE *file;
    char *buf;

    if (access(filename, R_OK)) {
        fprintf (stderr, "WARNING: cannot access file %s\n", filename);
        return NULL;
    }
    reg = rrn_reg_new_full ();

    file = fopen (filename, "r");

    buf = malloc (sizeof (char) * 1024);
    while (fgets (buf, 1023, file)) {
    	char *real = NULL;
    	while (buf[strlen(buf)-1] != '\n') {
    		char *tmp;
    		char *result = NULL;
    		tmp = strdup (buf);
    		buf = fgets (buf, 1023, file);
			result = malloc (sizeof (char) * (strlen(tmp)+strlen(buf)+2));
			strcpy (result, tmp);
			strcat (result, buf);
			free (tmp);
			free (buf);
    		buf = result;
    	}
		real = buf;
    	while (*real && isspace(*real) && *real != '\n') {
    		real++;
    	}
		if (!real || real[0] == '\n' || real[0] == '#') {
			/* Black Line or comment.  Ignore. */
		} else if (real[0] == '[') {
			if (!strncmp (real, "[Document]", 10)) {
				mode = 0;

				if (sect) {
						if (rrn_reg_add_sect_full (reg, sect) == 1) {
							sect->prev = NULL;
							sect->next = orphaned_sections;
							if (orphaned_sections)
								orphaned_sections->prev = sect;
							orphaned_sections = sect;
						}
					sect = NULL;
				}
			} else if (!strncmp (real, "[Section]", 9)) {
				mode = 1;
					if (sect) {
						if (rrn_reg_add_sect_full (reg, sect) == 1) {
							sect->prev = NULL;
							sect->next = orphaned_sections;
							if (orphaned_sections)
								orphaned_sections->prev = sect;
							orphaned_sections = sect;
						}
						sect = NULL;
					}
					sect = rrn_sect_new_full ();
			} else {
				fprintf (stderr, "Unknown section header: %s.  Ignoring\n", real);
			}
		} else if (strchr ( real, '=')) {
			if (mode == 0) {
				process_line_full (buf, reg);
			} else {
				process_sect_line_full (buf, sect);
			}
		} else {
			fprintf (stdout, "WARNING: Don't know how to handle line: %s\n", buf);
		}
    }
    fclose (file);
    free (buf);
	if (sect) {
		if (rrn_reg_add_sect_full (reg, sect) == 1) {
			sect->prev = NULL;
			sect->next = orphaned_sections;
			if (orphaned_sections)
				orphaned_sections->prev = sect;
			orphaned_sections = sect;
		}
	}

    if (!reg->name || !reg->uri || !reg->type || !reg->categories ) {
        rrn_reg_free_full (reg);
        reg = NULL;
    }
    sect = orphaned_sections;
    while (sect) {
    	if (rrn_reg_add_sect_full (reg, sect) == 1) {
    		fprintf (stderr, "Error: Orphaned section %s not added.\n",
    				 sect->name);
    	}
    	sect = sect->next;
    }

	if (reg && !reg->identifier) {
		char *last;
		char *sep;
		char *basename;

		sep = strrchr (filename, '/');
		last = strrchr (filename, '.');
		if (!last || !sep || last < sep) {
			fprintf (stderr, "Error: Can't cut put basename properly\n");
			rrn_reg_free_full (reg);
			return NULL;
		}
		basename = rrn_strndup (sep + 1, last - (sep + 1));
		reg->identifier = malloc (sizeof (char)*(strlen(basename)+11));
		sprintf (reg->identifier, "org.other.%s", basename);
		free (basename);

	}
	if (reg)
		process_path_full (reg);


    return reg;
}

static void
process_line_full (char *line, RrnRegFull *reg)
{
	char *ret = NULL;
    char *key;
    char *value;
    char *tmp;

	tmp = strchr (line, '=');
    if (!tmp) {
        fprintf (stderr, "WARNING: Malformed line: \n%s\n", line);
        return;
    }

    if (line[strlen(line)-1] == '\n') {
        line[strlen(line)-1] = '\0';
    }

    key = rrn_strndup (line, tmp - line);
    tmp++;
    value = strdup (tmp);
    process_pair_full (reg, key, value);
	free (key);
	free (value);

	return;
}

static char *
find_language (char *key)
{
    char *lbrace = NULL, *rbrace = NULL;
    char *result;

    result = strdup ("C");
    lbrace = strchr (key, '[');
    if (!lbrace)
        return result;
    lbrace++;
    rbrace = strchr (key, ']');
    if (!rbrace)
        return result;
    free (result);
    result = rrn_strndup (lbrace, rbrace-lbrace);
    return result;
}

RrnListEntry *
rrn_full_add_field (RrnListEntry *current, char *text, char *lang)
{
    RrnListEntry *entry = NULL;
    RrnListEntry *iter = NULL;
    iter = current;
    while (iter) {
        if (!strcmp(iter->lang, lang))
            return current;
        iter = iter->next;
    }
    entry = malloc (sizeof (RrnListEntry));
    if (text) {
        entry->text = strdup (text);
    } else {
        entry->text = strdup ("");
    }
    entry->lang = strdup (lang);
    entry->next = current;

    return entry;

}

static char **
process_categories_full (char *cat_string)
{
	char **result = NULL;
	char **tmp = NULL;
	int ncats = 0;
	char *current_break = cat_string;
	char *semi;
	int i;
	do {
		semi = strchr (current_break, ';');
		if (result) {
			tmp = malloc (sizeof (char *)*ncats);
			for (i=0; i< ncats; i++) {
				tmp[i] = strdup (result[i]);
				free (result[i]);
			}
			free (result);
		}
		ncats += 1;
		result = malloc (sizeof (char *)*(ncats+1));
		if (tmp) {
			for (i=0; i<ncats-1; i++) {
				result[i] = strdup (tmp[i]);
				free (tmp[i]);
			}
			free (tmp);
		}
		result[ncats-1] = rrn_strndup (current_break, semi - current_break);
		result[ncats] = NULL;
		current_break = semi+1;
	} while (semi);

	return result;
}

static void
process_pair_full (RrnRegFull *reg, char *key, char *value)
{
  char *lang = NULL;

  lang = find_language (key);

  if (!strncmp (key, "Name", 4)) {
    reg->name = rrn_full_add_field (reg->name, value, lang);
  } else if (!strncmp (key, "Comment", 7)) {
    reg->comment = rrn_full_add_field (reg->comment, value, lang);
  } else if (!strncmp (key, "DocPath", 7)) {
    reg->uri = rrn_full_add_field (reg->uri, value, lang);
  } else if (!strcmp (key, "DocIdentifier")) {
    reg->identifier = strdup (value);
  } else if (!strcmp (key, "DocType")) {
    reg->type = strdup (value);
  } else if (!strcmp (key, "Categories")) {
    reg->categories = process_categories_full (value);
  } else if (!strcmp (key, "DocWeight")) {
    reg->weight = atoi (value);
  } else if (!strcmp (key, "DocHeritage")) {
    reg->heritage = strdup (value);
  } else if (!strcmp (key, "NoDisplay")) {
    reg->hidden = TRUE;
  } else if (!strcmp (key, "DocDefaultSection")) {
    reg->default_section = strdup (value);
  } else if (!strcmp (key, "DocLang")) {
    if (!reg->lang) {
      reg->lang = strdup (value);
    }
  } else if (!strcmp (key, "Icon")){
    reg->icon = strdup (value);
  } else {
    fprintf (stderr, "WARNING: Unknown element %s: %s\n", key, value);
  }
  return;
}

void
rrn_reg_free_full (RrnRegFull *reg)
{
	RrnSectFull *sect = reg->children;
	char **tmp = reg->categories;
    free (reg->name);
    free (reg->comment);
    free (reg->uri);
    free (reg->type);
    free (reg->identifier);
    free (reg->heritage);
	free (reg->lang);
	if (tmp) {
		while (*tmp) {
			free (*tmp);
			tmp++;
		}
	}
	free (reg->categories);
	while (sect) {
		RrnSectFull *s1 = sect->next;
		rrn_sect_free_full (sect);
		sect = s1;
	}
    free (reg);
    return;
}

/* Section stuff */
RrnSectFull *
rrn_sect_new_full ()
{
	RrnSectFull * sect;
	sect = malloc (sizeof (RrnSectFull));

	sect->name = NULL;
	sect->identifier = NULL;
	sect->uri = NULL;
	sect->next = NULL;
	sect->prev = NULL;
	sect->children = NULL;
	sect->owner = NULL;

	return sect;
}

static RrnSectFull *
find_sect_full (RrnSectFull *start, char *id)
{
	RrnSectFull *ret = start;

	while (ret) {
		if (!strcmp (ret->identifier, id)) {
			return ret;
		}
		ret = ret->next;
	}
	return NULL;
}

static void
rrn_reg_add_sections_full (RrnRegFull *reg, RrnSectFull *sects)
{
	RrnSectFull *real_orphans = NULL;
	RrnSectFull *next = NULL;
	int depth = 0;
	do {
		depth ++;
		if (depth > 4) {
			/* The pathological case where someone's done something really,
			 * stupid and defined a subsection with the section without
			 * definind the section.
			 * We break after 4 iterations.
			 */
			return;
		}
		next = sects;
		while (sects) {
			next = sects->next;
			if (rrn_reg_add_sect_full (reg, sects) == 1) {
				sects->prev = NULL;
				sects->next = real_orphans;
				if (real_orphans)
					real_orphans->prev = sects;
				real_orphans = sects;
			};
			sects = next;
		}
		sects = real_orphans;
	} while (sects);
	return;
}

static int
rrn_sects_add_sect_full (RrnSectFull *current, RrnSectFull *sect)
{
	char *cur_sect = NULL;
	RrnSectFull *append_sect;
	char *tmp = NULL;
	char *dot = NULL;

	cur_sect = sect->owner;

	append_sect = current;
	do {
		dot = strchr (cur_sect, '.');
		tmp = rrn_strndup (cur_sect, dot-cur_sect);
		append_sect = find_sect_full (append_sect, tmp);
		cur_sect = dot;
		free (tmp);
	} while (cur_sect && append_sect);
	if (append_sect) {
		sect->prev = NULL;
		sect->next = append_sect->children;
		if (append_sect->children)
			append_sect->children->prev = sect;
		append_sect->children = sect;
	} else {
		return 1;
	}
	return 0;
}

static int
rrn_reg_add_sect_full (RrnRegFull *reg, RrnSectFull *sect)
{
	if (!sect->owner || !strcmp (reg->identifier, sect->owner)) {
		sect->prev = NULL;
		sect->next = reg->children;
		if (reg->children)
			reg->children->prev = sect;
		reg->children = sect;
	} else {
		char *cur_sect = NULL;
		RrnSectFull *append_sect;
		char *tmp = NULL;
		char *dot = NULL;

		if (!strncmp (sect->owner, reg->identifier, strlen (reg->identifier))) {
			cur_sect = &sect->owner[strlen(reg->identifier)+1];
		} else {
			cur_sect = sect->owner;
		}
		append_sect = reg->children;
		do {
			dot = strchr (cur_sect, '.');
			tmp = rrn_strndup (cur_sect, dot-cur_sect);
			append_sect = find_sect_full (append_sect, tmp);
			cur_sect = dot;
			free (tmp);
		} while (cur_sect && append_sect);
		if (append_sect) {
			sect->prev = NULL;
			sect->next = append_sect->children;
			if (append_sect->children)
				append_sect->children->prev = sect;
			append_sect->children = sect;
		} else {
			return 1;
		}
	}
	return 0;
}

RrnSectFull *
rrn_sect_parse_file_full (char *filename)
{
    RrnSectFull *sect = NULL;
    RrnSectFull *orphaned_sections = NULL;
    FILE *file;
    char *buf;
	char *real_owner = NULL;

    if (access(filename, R_OK)) {
        fprintf (stderr, "WARNING: cannot access file %s\n", filename);
        return NULL;
    }

    file = fopen (filename, "r");

    buf = malloc (sizeof (char) * 1024);
    while (fgets (buf, 1023, file)) {
    	char *real = NULL;
    	while (buf[strlen(buf)-1] != '\n') {
    		char *tmp;
    		char *result = NULL;
    		tmp = strdup (buf);
    		buf = fgets (buf, 1023, file);
			result = malloc (sizeof (char) * (strlen(tmp)+strlen(buf)+2));
			strcpy (result, tmp);
			strcat (result, buf);
			free (tmp);
			free (buf);
    		buf = result;
    	}
		real = buf;
    	while (*real && isspace(*real) && *real != '\n') {
    		real++;
    	}
		if (!real || real[0] == '\n' || real[0] == '#') {
			/* Black Line or comment.  Ignore. */
		} else if (real[0] == '[') {
			if (!strncmp (real, "[Section]", 9)) {
				if (sect) {
					if (rrn_sects_add_sect_full (orphaned_sections, sect) == 1) {
							sect->prev = NULL;
							sect->next = orphaned_sections;
							if (orphaned_sections)
								orphaned_sections->prev = sect;
							orphaned_sections = sect;
						}
				}

				sect = rrn_sect_new_full ();
			} else {
				fprintf (stderr, "Unknown section header: !%s!.  Ignoring\n", real);
			}
		} else if (strchr ( real, '=')) {
			process_sect_line_full (buf, sect);
		} else {
			fprintf (stderr, "WARNING: Don't know how to handle line: %s\n", buf);
		}
    }
    fclose (file);
    free (buf);

	if (sect) {
		if (rrn_sects_add_sect_full (orphaned_sections, sect) == 1) {
			sect->prev = NULL;
			sect->next = orphaned_sections;
			if (orphaned_sections)
				orphaned_sections->prev = sect;
			orphaned_sections = sect;
		}
	}

    return orphaned_sections;
}

static void
process_sect_line_full (char *line, RrnSectFull *sect)
{
	char *ret = NULL;
    char *key;
    char *value;
    char *tmp;

	tmp = strchr (line, '=');
    if (!tmp) {
        fprintf (stderr, "WARNING: Malformed line: \n%s\n", line);
        return;
    }

    if (line[strlen(line)-1] == '\n') {
        line[strlen(line)-1] = '\0';
    }

    key = rrn_strndup (line, tmp - line);
    tmp++;
    value = strdup (tmp);
    process_sect_pair_full (sect, key, value);
	free (key);
	free (value);

	return;

}

static void
process_sect_pair_full (RrnSectFull *sect, char *key, char *value)
{
    char *lang = NULL;

    lang = find_language (key);

    if (!strncmp (key, "SectionName", 11) || !strncmp (key, "sectionname", 11)) {
        sect->name = rrn_full_add_field (sect->name, value, lang);
    } else if (!strcmp (key, "SectionIdentifier") || !strcmp (key, "sectionidentifier")) {
        sect->identifier = strdup (value);
    } else if (!strncmp (key, "SectionPath", 11) || !strncmp (key, "sectionpath", 11)) {
        sect->uri = rrn_full_add_field (sect->uri, value, lang);
    } else if (!strcmp (key, "SectionDocument") || !strcmp (key, "sectiondocument")) {
        sect->owner = strdup (value);
    } else {
        fprintf (stderr, "WARNING: Unknown element for section %s: %s\n", key, value);
    }
    return;
}

static void
process_path_full (RrnRegFull *reg)
{
	char *prefix = NULL;
	RrnSectFull *child = reg->children;
	RrnListEntry *entry = reg->uri;
	while (entry) {
	  if (!strncmp ("file:", entry->text, 5)) {
	    /* No processing needs done.  The URI is already in the file: scheme */
	    return;
	  }
	  if ((prefix = strchr (entry->text, ':')) && (prefix-(entry->text) < 7)) {
	    /* Probably has another (non-file:) URI scheme.  If so, we're going to
	     * return as is
	     */
	    return;
	  }
	  /* Otherwise, promote to file: URI scheme, reusing the prefix vble */
	  prefix = malloc (sizeof (char) * (strlen(entry->text)+6));
	  sprintf (prefix, "file:%s", entry->text);
	  free (entry->text);
	  entry->text = prefix;
	  entry = entry->next;
	}
	while (child) {
	  process_section_path_full (reg->uri, child);
	  child = child->next;
	}
}

static void
process_section_path_full (RrnListEntry *owner_paths, RrnSectFull *section)
{
	char *tmp = NULL;
	char *new_uri = NULL;
	char *prefix = NULL;
	char *filename = NULL;
	char *owner_path = NULL;

	RrnSectFull *child = section->children;

	RrnListEntry *entry = section->uri;

	while (entry) {
	  RrnListEntry *parent_entry = owner_paths;

	  while (parent_entry && strcmp (parent_entry->lang, entry->lang)) {
	    parent_entry = parent_entry->next;
	  }
	  if (!parent_entry) {
	    parent_entry = owner_paths;
	    while (strcmp (parent_entry->lang, "C")) {
	      parent_entry = parent_entry->next;
	    }
	    if (!parent_entry) {
	      fprintf (stderr, "Error: Cannot find a suitable parent for child %s\n"
		       "This usually results from bad script files.  Please correct and\n"
		       "try again.  Ignoring this child (and it's children) completely.\n",
		       entry->text);
	      return;
	    }
	  }
	  owner_path = parent_entry->text;
	  
	  if (!strncmp ("file:", entry->text, 5))
	    goto done;
	  if ((prefix = strchr (entry->text, ':')) && (prefix-(entry->text) < 7)) {
	    /* Probably has another (non-file:) URI scheme.  If so, we're going to
	     * return as is
	     */
	    goto done;
	  }
	  if (entry->text[0] == '/') {
	    /* Absolute path */
	    new_uri = malloc (sizeof (char) * strlen(entry->text)+6);
	    sprintf (new_uri, "file:/%s", entry->text);
	    free (entry->text);
	    entry->text = new_uri;
	    goto done;
	  }
	  
	  /* If we get through to here, the path is relative to the parents
	   * own path.  We need to disect and repair
	   */
	  
	  /* First, cut out the owner's file */
	  tmp = strrchr (owner_path, '/');
	  prefix = rrn_strndup (owner_path, tmp-owner_path);
	  if (!tmp) {
	    fprintf (stderr, "Warning: cannot cut up path for the %s section\n"
		     "This generally indicates a problem with the scroll\n"
		     " file for this section, or its parent document.\n"
		     "The path will not be fixed.\n", entry->text);
	    return;
	  }
	  new_uri = malloc (sizeof (char) * (strlen(prefix)+strlen(entry->text)+2));
	  sprintf (new_uri, "%s/%s", prefix, entry->text);
	  free (entry->text);
	  entry->text = new_uri;
done:
	  entry = entry->next;
	}

	/* In all cases, we want to iterate through the children of the children
	 * just to make sure they're all right
	 */
	while (child) {
		process_section_path_full (section->uri, child);
		child = child->next;
	}
}



void
rrn_sect_free_full (RrnSectFull *sect)
{
	RrnSectFull *child = sect->children;
	free (sect->name);
	free (sect->identifier);
	free (sect->uri);
	free (sect->owner);
	while (child) {
		RrnSectFull *c1 = child->next;
		rrn_sect_free_full (child);
		child = c1;
	}
	free (sect);
}
