/*
 * rarian-reg-utils.c
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

#include "rarian-reg-utils.h"
#include "rarian-language.h"
#include "rarian-utils.h"


static void     process_line (char *line, RrnReg *reg);
static void     process_pair (RrnReg *reg, char *key, char *value);
static void     process_sect_line (char *line, RrnSect *sect);
static void     process_sect_pair (RrnSect *sect, char *key, char *value);
static int      rrn_reg_add_sect (RrnReg *reg, RrnSect *sect);
static void     process_path (RrnReg *reg);
static void     process_section_path (char *owner_path, RrnSect *section);


RrnReg *
rrn_reg_new ()
{
    RrnReg *reg;

    reg = malloc (sizeof (RrnReg));
    reg->name = NULL;
    reg->uri = NULL;
    reg->comment = NULL;
    reg->identifier = NULL;
    reg->type        = NULL;
    reg->icon        = NULL;
    reg->categories  = NULL;
    reg->weight   = 0;
    reg->heritage    = NULL;
    reg->omf_location = NULL;
    reg->lang = NULL;
    reg->ghelp_name = NULL;
    reg->default_section = NULL;
    reg->hidden = FALSE;
    reg->children = NULL;

    return reg;
}

RrnReg *
rrn_reg_parse_file (char *filename)
{
    RrnReg *reg = NULL;
    RrnSect *sect = NULL;
    RrnSect *orphaned_sections = NULL;
    int mode = 0;
    FILE *file;
    char *buf;

    if (access(filename, R_OK)) {
        fprintf (stderr, "WARNING: cannot access file %s\n", filename);
        return NULL;
    }
    reg = rrn_reg_new ();

    /* First, figure out it's ghelp name */
    {
      char *dot = strrchr (filename, '.');
      char *sep = strrchr (filename, '/');
      if (!dot || !sep) {
	fprintf (stderr, "Error parsing file %s for ghelp name.  Ignoring\n", filename);
      } else {
	sep++;
	reg->ghelp_name = rrn_strndup (sep, dot-sep);
      }
    }

    file = fopen (filename, "r");

    buf = malloc (sizeof (char) * 1024);
    while (fgets (buf, 1023, file)) {
    	char *real = NULL;
    	while (buf[strlen(buf)-1] != '\n') {
    		char *tmp;
    		char *result = NULL;
    		tmp = strdup (buf);
    		if (fgets (buf, 1023, file)) {
			result = malloc (sizeof (char) * (strlen(tmp)+strlen(buf)+2));
			strcpy (result, tmp);
			strcat (result, buf);
			free (tmp);
			free (buf);
			buf = result;
		} else {
			free (buf);
			buf = tmp;
			break;
		}
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
						if (rrn_reg_add_sect (reg, sect) == 1) {
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
						if (rrn_reg_add_sect (reg, sect) == 1) {
							sect->prev = NULL;
							sect->next = orphaned_sections;
							if (orphaned_sections)
								orphaned_sections->prev = sect;
							orphaned_sections = sect;
						}
						sect = NULL;
					}
					sect = rrn_sect_new ();
			} else {
				fprintf (stderr, "Unknown section header: %s.  Ignoring\n", real);
			}
		} else if (strchr ( real, '=')) {
			if (mode == 0) {
				process_line (buf, reg);
			} else {
				process_sect_line (buf, sect);
			}
		} else {
			fprintf (stdout, "WARNING: Don't know how to handle line: %s\n", buf);
		}
    }
    fclose (file);
    free (buf);
	if (sect) {
		if (rrn_reg_add_sect (reg, sect) == 1) {
			sect->prev = NULL;
			sect->next = orphaned_sections;
			if (orphaned_sections)
				orphaned_sections->prev = sect;
			orphaned_sections = sect;
		}
	}

    if (!reg->name || !reg->uri || !reg->type || !reg->categories ) {
        rrn_reg_free (reg);
        reg = NULL;
    }
    sect = orphaned_sections;
    while (sect) {
    	if (rrn_reg_add_sect (reg, sect) == 1) {
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
			rrn_reg_free (reg);
			return NULL;
		}
		basename = rrn_strndup (sep + 1, last - (sep + 1));
		reg->identifier = malloc (sizeof (char)*(strlen(basename)+11));
		sprintf (reg->identifier, "org.other.%s", basename);
		free (basename);
		printf ("identifier is %s\n", reg->identifier);

	}
	if (reg)
		process_path (reg);

	if (reg && reg->omf_location) {
	  /* Replace -* with something more sensible */
	  char *star_loc;
	  char *tmp;
	  char *tmp2;
	  
	  star_loc = strrchr (reg->omf_location, '*');

	  /*tmp = malloc (sizeof(char) * (strlen(reg->omf_location) - 
	    strlen(star_loc)));*/
	  tmp = rrn_strndup (reg->omf_location, (star_loc - reg->omf_location));
	  star_loc++;
	  if (reg->lang) {
	    tmp2 = malloc (sizeof(char) * (strlen(tmp) +
					   strlen(star_loc) +
					   strlen (reg->lang))+1);
	    sprintf (tmp2, "%s%s%s", tmp, reg->lang, star_loc);
	  } else {
	    tmp2 = malloc (sizeof(char) * (strlen(tmp) +
					   strlen(star_loc)));
	    sprintf (tmp2, "%sC%s", tmp, star_loc);
	  }
	  free (reg->omf_location);
	  reg->omf_location = strdup (tmp2);
	  free (tmp);
	  free (tmp2);
	}
    return reg;
}

static void
process_line (char *line, RrnReg *reg)
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

	rrn_strip(key);
	rrn_strip(value);

    process_pair (reg, key, value);
	free (key);
	free (value);

	return;
}

static void
process_field (char **current, char **lang, char *key, char *value)
{
    char *lbrace = NULL, *rbrace = NULL, *l = NULL;
    lbrace = strchr (key, '[');
    lbrace++;
    rbrace = strchr (key, ']');
    if ((!lbrace || !rbrace)) {
        if (!*current) {
            *current = strdup (value);
			if (lang) {
				if (*lang)
					free (*lang);
	            *lang = strdup ("C");
			}
        }
        return;
    }
    l = rrn_strndup (lbrace, rbrace - lbrace);
    if (rrn_language_use ( (lang ? *lang : NULL), l) == 1) {
        if (*current) {
            free (*current);
        }
        if (lang && *lang) {
        	free (*lang);
        }

        *current = strdup (value);
		if (lang) {
			*lang = strdup (l);
		}

    }
    free (l);
}

static char **
process_categories (char *cat_string)
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
		if (semi)
		  result[ncats-1] = rrn_strndup (current_break, semi - current_break);
		else
		  result[ncats-1] = strdup (current_break);
		result[ncats] = NULL;
		current_break = semi+1;
	} while (semi);

	return result;
}

static void
process_pair (RrnReg *reg, char *key, char *value)
{
  if (!strncmp (key, "Name", 4)) {
    process_field (&(reg->name), &(reg->lang), key, value);
  } else if (!strncmp (key, "Comment", 7)) {
    process_field (&(reg->comment), &(reg->lang), key, value);
  } else if (!strncmp (key, "DocPath", 7)) {
    process_field (&(reg->uri), &(reg->lang), key, value);
  } else if (!strcmp (key, "DocIdentifier")) {
    reg->identifier = strdup (value);
  } else if (!strcmp (key, "DocType")) {
    reg->type = strdup (value);
  } else if (!strcmp (key, "Categories")) {
    reg->categories = process_categories (value);
  } else if (!strcmp (key, "DocWeight")) {
    reg->weight = atoi (value);
  } else if (!strcmp (key, "x-DocHeritage")) {
    reg->heritage = strdup (value);
  } else if (!strcmp (key, "x-Scrollkeeper-omf-loc")) {
    reg->omf_location = strdup(value);
  } else if (!strcmp (key, "DocLang")) {
    if (!reg->lang) {
      reg->lang = strdup (value);
    }
  } else if (!strcmp (key, "NoDisplay")) {
    if (!strcmp (key, "true"))
      reg->hidden = TRUE;
  } else if (!strcmp (key, "DocDefaultSection")) {
    reg->default_section = strdup (value);
  } else if (!strcmp (key, "Icon")) {
    reg->icon = strdup (value);
  } else {
    fprintf (stderr, "WARNING: Unknown element %s: %s\n", key, value);
  }
  return;
}

void
rrn_reg_free (RrnReg *reg)
{
  RrnSect *sect = reg->children;
  char **tmp = reg->categories;
  free (reg->name);
  free (reg->comment);
  free (reg->uri);
  free (reg->type);
  free (reg->identifier);
  free (reg->heritage);
  free (reg->omf_location);
  free (reg->lang);
  if (reg->default_section)
    free (reg->default_section);
  if (reg->ghelp_name) 
    free (reg->ghelp_name);
  if (tmp) {
    while (*tmp) {
      free (*tmp);
      tmp++;
    }
  }
  free (reg->categories);
  while (sect) {
    RrnSect *s1 = sect->next;
    rrn_sect_free (sect);
    sect = s1;
  }
  free (reg);
  return;
}

/* Section stuff */
RrnSect *
rrn_sect_new ()
{
	RrnSect * sect;
	sect = malloc (sizeof (RrnSect));

	sect->name = NULL;
	sect->identifier = NULL;
	sect->uri = NULL;
	sect->next = NULL;
	sect->prev = NULL;
	sect->children = NULL;
	sect->owner = NULL;
	sect->priority = 0;
}

RrnSect *
find_sect (RrnSect *start, char *id)
{
	RrnSect *ret = start;

	while (ret) {
		if (!strcmp (ret->identifier, id)) {
			return ret;
		}
		ret = ret->next;
	}
	return NULL;
}

RrnSect *
rrn_reg_add_sections (RrnReg *reg, RrnSect *sects)
{
	RrnSect *real_orphans = NULL;
	RrnSect *next = NULL;
	int depth = 0;
	do {
		depth ++;
		if (depth > 4) {
			/* The pathological case where someone's done something really,
			 * stupid and defined a subsection with the section without
			 * definind the section.
			 * We break after 4 iterations.
			 */
			return sects;
		}
		next = sects;
		while (sects) {
			next = sects->next;
			if (rrn_reg_add_sect (reg, sects) == 1) {
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
	return sects;
}

int
rrn_sects_add_sect (RrnSect *current, RrnSect *sect)
{
	char *cur_sect = NULL;
	RrnSect *append_sect;
	char *tmp = NULL;
	char *dot = NULL;

	cur_sect = sect->owner;

	append_sect = current;
	do {
		dot = strchr (cur_sect, '.');
		tmp = rrn_strndup (cur_sect, dot-cur_sect);
		append_sect = find_sect (append_sect, tmp);
		cur_sect = dot;
		free (tmp);
	} while (cur_sect && append_sect);
	if (append_sect) {
		RrnSect *iter = append_sect->children;
		while (iter) {
			if (!strcmp (iter->identifier, sect->identifier)) {
				sect->prev = iter->prev;
				sect->next = iter->next;
				if (iter->prev) {
					iter->prev->next = sect;
				}
				if (iter->next) {
					iter->next->prev = sect;
				}
				return 0;
			}
			iter = iter->next;
		}
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

int
rrn_reg_add_sect (RrnReg *reg, RrnSect *sect)
{
	if (!sect->owner || !strcmp (reg->identifier, sect->owner)) {
		RrnSect *iter = reg->children;
		while (iter) {
			if (!strcmp (iter->identifier, sect->identifier)) {
				if (iter->priority < sect->priority) {
					process_section_path (reg->uri, sect);
					sect->prev = iter->prev;
					sect->next = iter->next;
					if (iter->prev) {
						iter->prev->next = sect;
					}
					if (iter->next && iter->next->prev == iter) {
						iter->next->prev = sect;
					}
					if (reg->children == iter) {
						reg->children = sect;
					}
				}
				return 0;
			}
			iter = iter->next;
		}
		process_section_path (reg->uri, sect);
		sect->prev = NULL;
		sect->next = reg->children;
		if (reg->children)
			reg->children->prev = sect;
		reg->children = sect;
	} else {
		char *cur_sect = NULL;
		RrnSect *append_sect;
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
			append_sect = find_sect (append_sect, tmp);
			cur_sect = dot;
			free (tmp);
		} while (cur_sect && append_sect);
		if (append_sect) {
			RrnSect *iter = append_sect->children;
			while (iter) {
				if (!strcmp (iter->identifier, sect->identifier)) {
					rrn_sect_free (sect);
					return 2;
				}
				iter = iter->next;
			}
			process_section_path (append_sect->uri, sect);
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

RrnSect *
rrn_sect_parse_file (char *filename)
{
    RrnSect *sect = NULL;
    RrnSect *orphaned_sections = NULL;
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
					if (rrn_sects_add_sect (orphaned_sections, sect) == 1) {
							sect->prev = NULL;
							sect->next = orphaned_sections;
							if (orphaned_sections)
								orphaned_sections->prev = sect;
							orphaned_sections = sect;
						}
				}

				sect = rrn_sect_new ();
				sect->priority = 1;
			} else {
				fprintf (stderr, "Unknown section header: !%s!.  Ignoring\n", real);
			}
		} else if (strchr ( real, '=')) {
			process_sect_line (buf, sect);
		} else {
			fprintf (stderr, "WARNING: Don't know how to handle line: %s\n", buf);
		}
    }
    fclose (file);
    free (buf);

	if (sect) {
		if (rrn_sects_add_sect (orphaned_sections, sect) == 1) {
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
process_sect_line (char *line, RrnSect *sect)
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
    process_sect_pair (sect, key, value);
	free (key);
	free (value);

	return;

}

static void
process_sect_pair (RrnSect *sect, char *key, char *value)
{
    if (!strncmp (key, "SectionName", 11) || !strncmp (key, "sectionname", 11)) {
        process_field (&(sect->name), NULL, key, value);
    } else if (!strcmp (key, "SectionIdentifier") || !strcmp (key, "sectionidentifier")) {
        sect->identifier = strdup (value);
    } else if (!strncmp (key, "SectionPath", 11) || !strncmp (key, "sectionpath", 11)) {
        process_field (&(sect->uri), NULL, key, value);
    } else if (!strcmp (key, "SectionDocument") || !strcmp (key, "sectiondocument")) {
        sect->owner = strdup (value);
    } else {
        fprintf (stderr, "WARNING: Unknown element for section %s: %s\n", key, value);
    }
    return;
}

static void
process_path (RrnReg *reg)
{
	char *prefix = NULL;
	RrnSect *child = reg->children;
	if (!strncmp ("file://", reg->uri, 7)) {
		/* No processing needs done.  The URI is already in the file: scheme */
		return;
	}
	if (!strncmp ("file:", reg->uri, 5)) {
	  /* We got the wrong number of slashes.  Fix here. */
	  char *new_prefix = &(reg->uri[5]);
	  char *result;
	  while (*new_prefix && *new_prefix == '/') {
	    new_prefix++;
	  }
	  new_prefix--;
	  result = malloc (sizeof(char) * (strlen(reg->uri)+7));
	  sprintf(result, "file://%s", new_prefix);
	  free (reg->uri);
	  reg->uri = result;
	  return;
	}

	if ((prefix = strchr (reg->uri, ':')) && 
	    (prefix - reg->uri) < 7) {
		/* Probably has another (non-file:) URI scheme.  If so, we're going to
		 * return as is
		 */
			return;
	}
	/* Otherwise, promote to file: URI scheme, reusing the prefix vble */
	prefix = malloc (sizeof (char) * (strlen(reg->uri)+8));
	sprintf (prefix, "file://%s", reg->uri);
	free (reg->uri);
	reg->uri = prefix;
	while (child) {
		process_section_path (reg->uri, child);
		child = child->next;
	}

}

static void
process_section_path (char *owner_path, RrnSect *section)
{
	char *tmp = NULL;
	char *new_uri = NULL;
	char *prefix = NULL;
	char *filename = NULL;

	RrnSect *child = section->children;

	if (!strncmp ("file:", section->uri, 5))
		goto done;
	if ((prefix = strchr (section->uri, ':')) && 
	    prefix-(section->uri) < 7) {
		/* Probably has another (non-file:) URI scheme.  If so, we're going to
		 * return as is
		 */
			goto done;
	}
	if (section->uri[0] == '/') {
		/* Absolute path */
		new_uri = malloc (sizeof (char) * strlen(section->uri)+6);
		sprintf (new_uri, "file:/%s", section->uri);
		free (section->uri);
		section->uri = new_uri;
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
						 "The path will not be fixed.\n", section->uri);
		return;
	}
	new_uri = malloc (sizeof (char) * (strlen(prefix)+strlen(section->uri)+2));
	sprintf (new_uri, "%s/%s", prefix, section->uri);
	free (section->uri);
	section->uri = new_uri;

done:
	/* In all cases, we want to iterate through the children of the children
	 * just to make sure they're all right
	 */
	while (child) {
		process_section_path (section->uri, child);
		child = child->next;
	}
}

void
rrn_sect_free (RrnSect *sect)
{
	RrnSect *child = sect->children;
	free (sect->name);
	free (sect->identifier);
	free (sect->uri);
	free (sect->owner);
	while (child) {
		RrnSect *c1 = child->next;
		rrn_sect_free (child);
		child = c1;
	}
	free (sect);
}
