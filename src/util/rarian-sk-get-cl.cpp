/*
 * rarian-sk-get-cl.cpp
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

#define I_KNOW_RARIAN_0_8_IS_UNSTABLE
#include <rarian.h>
#include <rarian-utils.h>
#include <tinyxml.h>
#include <sys/stat.h>
#include <sys/types.h>

static char *cat = NULL;
static TiXmlElement *docs = NULL;
static int id = 0;

void
print_usage (char *proc_name)
{
  printf ("Usage: %s <LOCALE> <CAT TREE NAME>\n", proc_name);
  exit (0);
}

void
get_attribute (TiXmlElement *elem)
{
    TiXmlAttribute* pAttrib=elem->FirstAttribute();
    if (strcmp (pAttrib->Value(), "")) {
      char *new_cat = NULL;
      char *iter = NULL;
      char *st_iter = NULL;
      char *tmp = NULL;

      cat = strdup (pAttrib->Value());
      /* Fix category to mimic scrollkeeper */
      new_cat = (char *) calloc (sizeof(char), strlen (cat));

      st_iter = cat;
      iter = cat;
      while (iter && *iter) {
	while (iter && *iter && *iter != '|') {
	  iter++;
	}
	tmp = rrn_strndup (st_iter, (iter-st_iter));
	if (*new_cat) {
	  strcat (new_cat, tmp);
	} else {
	  strcpy (new_cat, tmp);
	}
	free (tmp);
	if (*iter == '|')
	  iter++;
	else
	  break;
	st_iter = iter;
      }
      elem->SetAttribute("categorycode", new_cat);

    }
}

int
get_docs (RrnReg *reg, TiXmlNode *pParent)
{
  if (reg->omf_location) {
    TiXmlElement * doc = new TiXmlElement ("doc");
    TiXmlElement * entry;
    TiXmlElement * text;
    char *tmp;

    doc->SetAttribute ("id", id);
    id++;

    /* doctitle */
    entry = new TiXmlElement ("doctitle");
    entry->LinkEndChild (new TiXmlText (reg->name));
    doc->LinkEndChild (entry);

    /* docomf */
    entry = new TiXmlElement ("docomf");
    entry->LinkEndChild (new TiXmlText (reg->omf_location));
    doc->LinkEndChild (entry);

    /* docsource */
    tmp = reg->uri + 7;// Remove file://
    entry = new TiXmlElement ("docsource");
    entry->LinkEndChild (new TiXmlText (tmp));
    doc->LinkEndChild (entry);

    /* docformat */
    entry = new TiXmlElement ("docformat");
    entry->LinkEndChild (new TiXmlText (reg->type));
    doc->LinkEndChild (entry);

    /* docseriesid */
    tmp = reg->identifier + 17;// remove org.scrollkeeper.
    entry = new TiXmlElement ("docseriesid");
    entry->LinkEndChild (new TiXmlText (tmp));
    doc->LinkEndChild (entry);
    pParent->LinkEndChild (doc);

  }

  return 0;
}

void
process_node (TiXmlNode *pParent)
{
    TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	int num;

	switch ( t )
	{
	case TiXmlNode::DOCUMENT:
		break;

	case TiXmlNode::ELEMENT:
	   if (!strcmp (pParent->Value(), "sect")) {
	     get_attribute (pParent->ToElement());
      	     rrn_for_each_in_category ((RrnForeachFunc) get_docs, cat, pParent);
	     free (cat);

	     }
	default:
	  break;
	}

	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		process_node ( pChild);
	}
}


/* Find a suitable file to dump out output to.
 * for now, we're always going to the same file
 */
char *
find_dump_name ()
{
  char *filename = NULL;
  char *user = getenv ("USERNAME");
  char *basepath = NULL;
  int i=0;
  int last = 0;
  unsigned int lasttime = 0;
  int init = 1;

  if (!user) {
    /* if USERNAME isn't set, we use the "default" username: UNKNOWN */
    user = strdup ("UNKNOWN");
  }

  basepath = (char *) malloc (sizeof(char) * (strlen(user) + 18 /*/tmp/scrollkeeper.*/ + 1));
  sprintf (basepath, "/tmp/scrollkeeper-%s", user);
  mkdir(basepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);


  filename = (char *) malloc (sizeof(char) * (strlen(user) + 18 /*/tmp/scrollkeeper.*/ + 10 /*contents.0*/) + 1);

  for (i=0 ;i<5; i++) {
    struct stat sbuf;
    sprintf (filename, "/tmp/scrollkeeper-%s/contents.%d", user, i);
    if (stat(filename,&sbuf) == -1) {
      last = i;
      break;
    }    
    if (sbuf.st_mtime < lasttime || init) {
      init = 0;
      last = i;
      lasttime = sbuf.st_mtime;
    }
  }
  sprintf (filename, "/tmp/scrollkeeper-%s/contents.%d", user, last);

  return filename;
}

int main (int argc, char * argv[])
{
  int skip = 0;

  if (argc < 3 || argc > 4) {
    print_usage (argv[0]);
  }
  if (!strcmp(argv[1], "-v")) {
    skip = 1;
  }

  if (strcmp (argv[2+skip], "scrollkeeper_extended_cl.xml") &&
      strcmp (argv[2+skip], "scrollkeeper_cl.xml")) {
    print_usage (argv[0]);
  }
  bool loadok;

  rrn_set_language (argv[1+skip]);
  
  TiXmlDocument doc (DATADIR"/librarian/rarian-sk-cl.xml");
  loadok = doc.LoadFile(TIXML_ENCODING_UTF8);
  
  if (!loadok) {
    fprintf (stderr, "ERROR: Cannot parse template file.  Aborting.\n");
    exit (2);
  }
  
  TiXmlNode *pParent = doc.FirstChildElement();
  
  process_node (pParent);
  
  char *filename = find_dump_name ();
  doc.SaveFile(filename);
  printf ("%s\n", filename);
  
  exit (0);

}
