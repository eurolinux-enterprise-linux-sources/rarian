/*
 * rarian-sk-preinstall.cpp
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


#include <tinyxml.h>

#include <stdio.h>

static char *new_url = NULL;

static bool am_parsing = false;

void
print_usage (char *proc_name)
{
  printf ("Usage: %s [-n] <INSTALLED_DOC_NAME> <OLD OMF FILE> <NEW OMF FILENAME>\n", proc_name);
  printf ("\nExample: %s /usr/share/help/beanstalk.xml beanstalk.omf new-beanstalk.omf\n", proc_name);
  printf ("The -n flag is now ignored (it is no longer needed).\n");
  exit (0);
}

void
get_attribute (TiXmlElement *elem)
{
    TiXmlAttribute* pAttrib=elem->FirstAttribute();
    if (strcmp (pAttrib->Value(), "")) {

      elem->SetAttribute("url", new_url);
    }
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
	   if (!strcmp (pParent->Value(), "omf")) {
	       am_parsing = true;
	   } else if (!am_parsing) {
	       printf ("ERROR: Does not appear to be a valid OMF file.  Aborting\n");
           exit (6);
	   }
	   if (!strcmp (pParent->Value(), "identifier")) {
	       get_attribute (pParent->ToElement());
	   }
	default:
	  break;
    }

	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		process_node ( pChild);
	}
}

void
process_new_url (char *input)
{
  if (!strncmp (input, "file:", 5)) {
    new_url = strdup (input);
  } else {
    int i = 0;
    char *t = input;

    while (*t == '/') {
      i++;
      t++;
    }
    if (i == 1) {
      /* Normal path.  Add file:/ to the start */
      new_url = (char *) malloc (sizeof(char) * (strlen (input) + 7));
      sprintf (new_url, "file:/%s", input);
    } else {
      /* Don't know what to do.  Just copy and append file: to it */
      new_url = (char *) malloc (sizeof(char) * (strlen(input) + 6));
      sprintf (new_url, "file:%s", input);
    }
  }
}

int main (int argc, char * argv[])
{
  int skip = 0;
  if (argc < 3 || argc > 4) {
    print_usage (argv[0]);
  }

  if (!strcmp(argv[1], "-n"))
    skip = 1;

  process_new_url (argv[1+skip]);

  
  bool loadok;
  
  TiXmlDocument doc (argv[2+skip]);
  loadok = doc.LoadFile(TIXML_ENCODING_UTF8);
  
  if (!loadok) {
    fprintf (stderr, "ERROR: Cannot parse %s.  Is it valid?\n", argv[2+skip]);
    exit (2);
  }
  
  TiXmlNode *pParent = doc.FirstChildElement();
  
  process_node (pParent);
  
  doc.SaveFile(argv[3+skip]);
  
  free (new_url);

  exit (0);
}
