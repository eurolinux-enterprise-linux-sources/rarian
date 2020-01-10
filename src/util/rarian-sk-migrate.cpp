/*
 * rarian-sk-migrate.cpp
 * This file is part of Rarian
 *
 * Copyright (C) 2006 - Don scorgie
 *
 * Rarian is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Rarian is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rarian; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include <tinyxml.h>
#define I_KNOW_RARIAN_0_8_IS_UNSTABLE
#include <rarian.h>
#include <rarian-reg-full.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static RrnRegFull *reg = NULL;
static RrnReg     *tmp_reg = NULL;
static char *lang;
static char *sk_series;
static char *new_series;
static char *type;
static char *categories;

static bool am_parsing = false;

enum ElemType {
    REG_NAME,
    REG_URI,
    REG_DESC,
    REG_LANG,
    REG_SERIES,
    REG_TYPE,
    REG_CATEGORIES
};

void
dump_extended_keyfile (char *path, char *base)
{
    RrnListEntry *iter;

    printf ("# File generated from scrollkeeper files %s/%s-*.omf\n", path, base);
    printf ("# This should be replaced by the new keyfile at some point");

    printf ("\n[Document]\n\n");

    iter = reg->name;
    while (iter) {
        if (!iter->lang || !strcmp (iter->lang, "C")) {
            printf ("Name=%s\n", iter->text);
        } else {
            printf ("Name[%s]=%s\n", iter->lang, iter->text);
        }
        iter = iter->next;
    }
    iter = reg->comment;
    while (iter) {
        if (!iter->lang || !strcmp (iter->lang, "C")) {
            printf ("Comment=%s\n", iter->text);
        } else {
            printf ("Comment[%s]=%s\n", iter->lang, iter->text);
        }
        iter = iter->next;
    }
    iter = reg->uri;
    while (iter) {
        if (!iter->lang || !strcmp (iter->lang, "C")) {
            printf ("DocPath=%s\n", iter->text);
        } else {
            printf ("DocPath[%s]=%s\n", iter->lang, iter->text);
        }
        iter = iter->next;
    }
    if (type) {
        printf ("DocType=%s\n", type);
    } else {
        printf ("DocType=\n");
    }
    if (new_series) {
        printf ("DocIdentifier=%s\n", new_series);
    } else {
        fprintf (stderr, "ERROR: new series is undefined!\n");
    }
    if (categories) {
    	printf ("Categories=%s\n", categories);
    }    
    if (new_series) {
      printf ("x-DocHeritage=%s\n", new_series);
    }
    printf ("x-Scrollkeeper-omf-loc=%s/%s-*.omf\n", path, base);

}


void
add_info ()
{
    if (!lang) {
        lang = strdup ("C");
    }
    reg->uri = rrn_full_add_field (reg->uri, tmp_reg->uri, lang);
    reg->comment = rrn_full_add_field (reg->comment, tmp_reg->comment, lang);
    reg->name = rrn_full_add_field (reg->name, tmp_reg->name, lang);

}

void
get_attribute (TiXmlElement *elem, ElemType e)
{
    TiXmlAttribute* pAttrib=elem->FirstAttribute();
    if (e == REG_URI && strcmp (pAttrib->Value(), "")) {
        tmp_reg->uri = strdup (pAttrib->Value());
    } else if (e == REG_URI && !strcmp(pAttrib->Value(), "")){
        tmp_reg->uri = strdup ("");
    } else if (e == REG_LANG) {
      if (strcmp (pAttrib->Value(), "")) {
        lang = strdup (pAttrib->Value());
      }
    } else if (e == REG_SERIES) {
      if (strcmp (pAttrib->Value(), "")) {
        if (!sk_series) {
	  sk_series = strdup (pAttrib->Value());
        }
      }
    } else if (e == REG_TYPE && strcmp (pAttrib->Value(), "")) {
        if (!type) {
            type = strdup (pAttrib->Value());
        }
    } else if (e == REG_TYPE && !strcmp (pAttrib->Value(), "")) {
        /* Do nothing */
    } else if (e == REG_CATEGORIES && strcmp (pAttrib->Value(), "")) {
		if (!categories) {
			categories = strdup (pAttrib->Value());
		}
	} else if (e == REG_CATEGORIES && !strcmp (pAttrib->Value(), "")) {
		/* Do Nothing */
    } else {
      printf ("ERROR: Trying to get attribute from unknown entry type.  Exiting\n");
        exit (7);
    }
}

void
get_text (TiXmlNode* pElement, ElemType e)
{

    if (!pElement) {
        if (e == REG_NAME) {
            tmp_reg->name = strdup (" ");
        } else {
            tmp_reg->comment = strdup (" ");
        }
        return;
    }
    if (e == REG_NAME) {
        tmp_reg->name = strdup (pElement->Value());
    } else if (e == REG_DESC ) {
        tmp_reg->comment = strdup (pElement->Value());
    }

}

void
process_node (TiXmlNode *pParent)
{
    TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	int num;
	ElemType e;


	switch ( t )
	{
	case TiXmlNode::DOCUMENT:
		break;

	case TiXmlNode::ELEMENT:
	   if (!strcmp (pParent->Value(), "omf")) {
	       am_parsing = true;
	   } else if (!am_parsing) {
	       printf ("ERROR: Does not appear to be a valid OMF file.  Aborting\n");
           exit (6);
	   }
	   if (!strcmp (pParent->Value(), "title")) {
	       e = REG_NAME;
           get_text (pParent->FirstChild(), e);
	   } else if (!strcmp (pParent->Value(), "description")) {
	       e = REG_DESC;
	       get_text (pParent->FirstChild(), e);
	   } else if (!strcmp (pParent->Value(), "identifier")) {
	       e = REG_URI;
	       get_attribute (pParent->ToElement(), e);
	   } else if (!strcmp (pParent->Value(), "language")) {
	       e = REG_LANG;
	       get_attribute (pParent->ToElement(), e);
	   } else if (!strcmp (pParent->Value(), "relation")) {
	       e = REG_SERIES;
	       get_attribute (pParent->ToElement(), e);
	   } else if (!strcmp (pParent->Value(), "format")) {
	       e = REG_TYPE;
	       get_attribute (pParent->ToElement(), e);
	   } else if (!strcmp (pParent->Value(), "subject")) {
	   	   e = REG_CATEGORIES;
	   	   get_attribute (pParent->ToElement(), e);
	   }
	   break;
	default:
    	break;
    }

	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		process_node ( pChild);
	}
}

void
process_file (char *path, char *fname)
{
    char *name = NULL;
    int size = 0;
    bool loadok;

    size += strlen (path);
    size += strlen (fname);
    size += 2;

	tmp_reg = rrn_reg_new ();

    name = (char *) malloc (sizeof (char) * size);
    sprintf (name, "%s/%s", path, fname);

    TiXmlDocument doc (name);
    loadok = doc.LoadFile(TIXML_ENCODING_UTF8);

    if (!loadok) {
        fprintf (stderr, "ERROR: Cannot parse %s.  Is it valid?\n", name);
        exit (2);
    }

    TiXmlNode *pParent = doc.FirstChildElement();

    process_node (pParent);

    add_info ();

    rrn_reg_free (tmp_reg);
    free (name);
}

int main (int argc, char * argv[])
{
    DIR * dirp = NULL;
    struct dirent * dp = NULL;
    struct stat buf;
    char *path = NULL;

    if (argc != 3 || access (argv[1], R_OK)) {
        fprintf (stderr, "ERROR: Cannot access directory %s\n", argv[1]);
    }
	reg = rrn_reg_new_full ();

    dirp = opendir (argv[1]);
    while (1) {
        if ((dp = readdir(dirp)) != NULL) {
            stat(dp->d_name,&buf);
            if (buf.st_mode == S_IFREG && !strncmp (dp->d_name, argv[2], strlen (argv[2]))) {
                process_file (argv[1], dp->d_name);
            }
        } else {
            break;
        }
    }

    if (sk_series) {
        new_series = (char *) malloc (sizeof (char) * 255);
        sprintf (new_series, "org.scrollkeeper.%s", sk_series);
    } else {
		time_t t1;

  		time(&t1);

        new_series = (char *) malloc (sizeof (char) * 255);
        sprintf (new_series, "org.scrollkeeper.undefined.%d", (long) t1);
    }
    dump_extended_keyfile (argv[1], argv[2]);

    exit (0);
}
