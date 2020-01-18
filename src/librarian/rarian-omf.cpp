/*
 * rarian-omf.cpp
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


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include <../util/tinyxml.h>

#include <stdio.h>
#include <sys/types.h>
#include <time.h>

/* TODO: Figure out why this is required here */
#define I_KNOW_RARIAN_0_8_IS_UNSTABLE
#include "rarian-omf.h"

static char *lang;
static char *sk_series;
static char *new_series;
static char *type;

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

static char *          omf_process_category (char *omf_cat);

static void
get_attribute (TiXmlElement *elem, ElemType e, RrnReg *reg)
{
  TiXmlAttribute* pAttrib=elem->FirstAttribute();
  
  while (pAttrib != NULL) {
    if (e == REG_URI && !strcmp (pAttrib->Name(), "url")) {
      reg->uri = strdup (pAttrib->Value());
    } else if (e == REG_LANG && !strcmp(pAttrib->Name(), "code")) {
	reg->lang = strdup (pAttrib->Value());
    } else if (e == REG_SERIES && !strcmp(pAttrib->Name(), "seriesid")) {
	reg->identifier = (char *) malloc (sizeof(char) * (strlen(pAttrib->Value()) + 18));
	sprintf (reg->identifier, "org.scrollkeeper.%s", pAttrib->Value());
    } else if (e == REG_TYPE && !strcmp (pAttrib->Name(), "mime")) {
	reg->type = strdup (pAttrib->Value());
    } else if (e == REG_CATEGORIES && !strcmp (pAttrib->Name(), "category")) {
      
      /* OMF files can only have 1 category. */
      reg->categories = (char **) malloc (sizeof(char *) * 2);
      reg->categories[0] = omf_process_category ((char *) pAttrib->Value());
      reg->categories[1] = NULL;
    } else {
      /* Ignore unknown elements */
    }

    pAttrib = pAttrib->Next();
  }
}

static void
get_text (TiXmlNode* pElement, ElemType e, RrnReg *reg)
{

    if (!pElement) {
        if (e == REG_NAME) {
            reg->name = strdup (" ");
        } else {
            reg->comment = strdup (" ");
        }
        return;
    }
    if (e == REG_NAME) {
        reg->name = strdup (pElement->Value());
    } else if (e == REG_DESC ) {
        reg->comment = strdup (pElement->Value());
    }

}

static int
process_node (TiXmlNode *pParent, RrnReg *reg)
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
	       return 1;
	   }
	   if (!strcmp (pParent->Value(), "title")) {
	       e = REG_NAME;
	       get_text (pParent->FirstChild(), e, reg);
	   } else if (!strcmp (pParent->Value(), "description")) {
	       e = REG_DESC;
	       get_text (pParent->FirstChild(), e, reg);
	   } else if (!strcmp (pParent->Value(), "identifier")) {
	       e = REG_URI;
	       get_attribute (pParent->ToElement(), e, reg);
	   } else if (!strcmp (pParent->Value(), "language")) {
	       e = REG_LANG;
	       get_attribute (pParent->ToElement(), e, reg);
	   } else if (!strcmp (pParent->Value(), "relation")) {
	       e = REG_SERIES;
	       get_attribute (pParent->ToElement(), e, reg);
	   } else if (!strcmp (pParent->Value(), "format")) {
	       e = REG_TYPE;
	       get_attribute (pParent->ToElement(), e, reg);
	   } else if (!strcmp (pParent->Value(), "subject")) {
	   	   e = REG_CATEGORIES;
	   	   get_attribute (pParent->ToElement(), e, reg);
	   }
	   break;
	default:
    	break;
    }

	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
	  int ret;
	  ret = process_node ( pChild, reg);
	  if (ret != 0)
	    return ret;
	}
	return 0;
}

RrnReg *
rrn_omf_parse_file (char *path)
{
    bool loadok;
    RrnReg *reg;
    TiXmlDocument doc (path);

    reg = rrn_reg_new ();

    loadok = doc.LoadFile(TIXML_ENCODING_UTF8);

    if (!loadok) {
        fprintf (stderr, "ERROR: Cannot parse %s.  Is it valid?\n", path);
	rrn_reg_free (reg);
	return NULL;
    }

    TiXmlNode *pParent = doc.FirstChildElement();

    if (process_node (pParent, reg) != 0) {
      rrn_reg_free (reg);
      return NULL;
    }

    if (!reg->identifier) {
      reg->identifier = (char *) malloc (sizeof(char) * 35);
      sprintf (reg->identifier, "org.scrollkeeper.unknown%d", rand());
    }

    return reg;
}

static char *
omf_process_category (char *omf_cat)
{
  /* Converts omf categories into fd.o style categories 
   * using my own special and rather obscure methodology
   * - Full omf categories and their current equivalents
   *   can be found in the docs directory of the source in the
   *   file "omf_equivalence.txt"
   * I'm sorry.
   */
  char *result = NULL;
  char *next = NULL;
  
  if (!strncmp (omf_cat, "Applications", 12)) {
    next = &(omf_cat[12]);
    while (*next == '|') next++;
    if (!next || *next == 0) {
      result = strdup ("Utility");
    } else if (!strncmp (next, "Amusement", 9)) {
      result = strdup ("Game");
    } else if (!strncmp (next, "Education", 9)) {
      next = &(next[9]);
      while (*next == '|') next++;
      if (!next || *next == 0) {
	result = strdup ("Education");
      } else if (!strncmp (next, "Arts", 4)) {
	result = strdup ("Art");
      } else if (!strncmp (next, "Computer Science", 16) ||
		 !strncmp (next, "Technology", 10)) {
	result = strdup ("ComputerScience");
      } else if (!strncmp (next, "English", 7) || 
		 !strncmp (next, "Language", 8)) {
	result = strdup ("Languages");
      } else if (!strncmp (next, "Maths", 5)) {
	result = strdup ("Math");
      } else if (!strncmp (next, "Science", 7)) {
	result = strdup ("Science");
      } else if (!strncmp (next, "Other", 5)) {
	result = strdup ("Education");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Games", 5)) {
      next = &(next[5]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strncmp (next, "Other", 5)) {
	result = strdup ("Game");
      } else if (!strncmp (next, "Arcade", 6) ||
		 !strncmp (next, "Fighting", 8)) {
	result = strdup ("ArcadeGame");
      } else if (!strncmp (next, "Board", 5)) {
	result = strdup ("BoardGame");
      } else if (!strncmp (next, "First Person Shooters", 21) ||
		 !strncmp (next, "Role-Playing", 12)) {
	result = strdup ("RolePlay");
      } else if (!strncmp (next, "Puzzles", 7)) {
	result = strdup ("LogicGame");
      } else if (!strncmp (next, "Simulation", 10)) {
	result = strdup ("Simulation");
      } else if (!strncmp (next, "Strategy", 8)) {
	result = strdup ("StrategyGame");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Internet", 8)) {
      next = &(next[8]);
      while (*next == '|') next++;
      if (!next || *next == 0 ||
	  !strncmp (next, "Fax", 3) ||
	  !strncmp (next, "Other", 5)) {
	result = strdup ("Network");
      } else if (!strncmp (next, "Chat", 4)) {
	result = strdup ("Chat");
      } else if (!strncmp (next, "Email", 5)) {
	result = strdup ("Email");
      } else if (!strncmp (next, "File Sharing", 12) ||
		 !strncmp (next, "FTP", 3)) {
	result = strdup ("FileTransfer");
      } else if (!strncmp (next, "Internet Phone", 14)) {
	result = strdup ("Telephony");
      } else if (!strncmp (next, "Messaging", 9)) {
	result = strdup ("InstantMessaging");
      } else if (!strncmp (next, "News", 4)) {
	result = strdup ("News");
      } else if (!strncmp (next, "Video Conferencing", 18)) {
	result = strdup ("VideoConference");
      } else if (!strncmp (next, "Web", 3)) {
	result = strdup ("WebBrowser");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Multimedia", 10)) {
      next = &(next[10]);
      while (*next == '|') next++;
      if (!next || *next == 0 || 
	  !strncmp (next, "Other", 5)) {
	result = strdup ("AudioVideo");
      }	else if (!strcmp (next, "Graphics|Conversion") ||
		 !strcmp (next, "Sound|Analysis") ||
		 !strcmp (next, "Sound|Conversion") ||
		 !strcmp (next, "Sound|Editing") ||
		 !strcmp (next, "Video|Conversion") ||
		 !strcmp (next, "Video|Editing")) {
	result = strdup ("AudioVideoEditing");
      } else if (!strncmp (next, "Graphics", 8)) {
	next = &(next[8]);
	while (*next == '|') next++;
	if (!next || *next == 0 ||
	    !strncmp (next, "Other", 5)) {
	  result = strdup ("Video");
	} else if (!strncmp (next, "3D Modelling", 12) ||
		   !strncmp (next, "3D Rendering", 12)) {
	  result = strdup ("3DGraphics");
	} else if (!strncmp (next, "CAD", 3)) {
	  result = strdup ("Engineering");
	} else if (!strncmp (next, "Capture", 7)) {
	  result = strdup ("Recording");
	} else if (!strncmp (next, "Drawing", 7) ||
		   !strncmp (next, "Editing", 7)) {
	  result = strdup ("2DGraphics");
	} else if (!strncmp (next, "Viewing", 7)) {
	  result = strdup ("Viewer");
	} else {
	  goto failed;
	}
      } else if (!strncmp (next, "Sound", 5)) {
	next = &(next[5]);
	while (*next == '|') next++;
	if (!next || *next == 0 ||
	    !strncmp (next, "Other", 5)) {
	  result = strdup ("Audio");
	} else if (!strncmp (next, "CD Mastering", 12)) {
	  result = strdup ("DiscBurning");
	} else if (!strncmp (next, "MIDI", 4)) {
	  result = strdup ("Midi");
	} else if (!strncmp (next, "Mixers", 6)) {
	  result = strdup ("Mixer");
	} else if (!strncmp (next, "Players", 7)) {
	  result = strdup ("Player");
	} else if (!strncmp (next, "Recording", 9)) {
	  result = strdup ("Recorder");
	} else if (!strncmp (next, "Speech", 6)) {
	  result = strdup ("Audio");
	} else {
	  goto failed;
	}
      } else if (!strncmp (next, "Video", 5)) {
	next = &(next[5]);
	while (*next == '|') next++;
	if (!next || *next == 0 ||
	    !strncmp (next, "Other", 5)) {
	  result = strdup ("Video");
	} else if (!strncmp (next, "Capture", 7)) {
	  result = strdup ("Recorder");
	} else if (!strncmp (next, "Display", 7)) {
	  result = strdup ("Player");
	} else {
	  goto failed;
	}

      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Office", 6)) {
      next = &(next[6]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other")) {
	result = strdup ("Office");
      } else if (!strncmp (next, "Calendar", 8)) {
	result = strdup ("Calendar");
      } else if (!strncmp (next, "Data Processing", 15)) {
	result = strdup ("Office");
      } else if (!strncmp (next, "Database", 8)) {
	result = strdup ("Database");
      } else if (!strncmp (next, "Email", 5)) {
	result = strdup ("Email");
      } else if (!strncmp (next, "Financial", 9)) {
	result = strdup ("Finance");
      } else if (!strncmp (next, "PIM", 3)) {
	result = strdup ("ContactManagement");
      } else if (!strncmp (next, "Plotting", 8)) {
	result = strdup ("Office");
      } else if (!strncmp (next, "Presentation", 12)) {
	result = strdup ("Presentation");
      } else if (!strncmp (next, "Publishing", 10)) {
	result = strdup ("Publishing");
      } else if (!strncmp (next, "Web Publishing", 14)) {
	result = strdup ("Publishing");
      } else if (!strncmp (next, "Word Processing", 15)) {
	result = strdup ("WordProcessor");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Scientific", 10)) {
      next = &(next[10]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other")) {
	result = strdup ("Science");
      } else if (!strncmp (next, "Astronomy", 9)) {
	result = strdup ("Astronomy");
      } else if (!strncmp (next, "Astrophysics", 12)) {
	result = strdup ("Astronomy");
      } else if (!strncmp (next, "Biology", 7)) {
	result = strdup ("Biology");
      } else if (!strncmp (next, "Chemistry", 9)) {
	result = strdup ("Chemistry");
      } else if (!strncmp (next, "EDA", 3)) {
	result = strdup ("Engineering");
      } else if (!strncmp (next, "Genetics", 8)) {
	result = strdup ("Biology");
      } else if (!strncmp (next, "Math", 4)) {
	result = strdup ("Math");
      } else if (!strncmp (next, "Physics", 7)) {
	result = strdup ("Physics");
      } else if (!strncmp (next, "Visualisation", 13)) {
	result = strdup ("DataVisualisation");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Security", 8)) {
      /* First, "don't care" type one, where all
       * categories go to the same category */
      result = strdup ("Security");
    } else if (!strncmp (next, "Text Editors", 12)) {
      /* Another don't care */
      result = strdup ("TextEditor");
    } else if (!strncmp (next, "Utilities", 9)) {
      next = &(next[9]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other")) {
	result = strdup ("Utility");
      } else if (!strncmp (next, "Archiving", 9)) {
	result = strdup ("Archiving");
      } else if (!strncmp (next, "Calculating", 11)) {
	result = strdup ("Calculator");
      } else if (!strncmp (next, "Clocks", 6)) {
	result = strdup ("Clock");
      } else if (!strncmp (next, "Compression", 11)) {
	result = strdup ("Compression");
      } else if (!strncmp (next, "File Utilities", 14)) {
	result = strdup ("FileTools");
      } else if (!strncmp (next, "Monitors", 8)) {
	result = strdup ("Monitor");
      } else if (!strncmp (next, "Printing", 8)) {
	result = strdup ("Printing");
      } else if (!strncmp (next, "Terminals", 9)) {
	result = strdup ("TerminalEmulator");
      } else if (!strncmp (next, "Text Utilities", 13)) {
	result = strdup ("ConsoleOnly");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "X", 1)) {
      next = &(next[1]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other")) {
	result = strdup ("Utility");
      } else if (!strncmp (next, "Configuration", 13) ||
		 !strncmp (next, "Fonts", 5) ||
		 !strncmp (next, "Login Managers", 14) ||
		 !strncmp (next, "Window Managers", 15)) {
	result = strdup ("DesktopSettings");
      } else if (!strncmp (next, "Screensavers", 12)) {
	result = strdup ("Screensaver");
      } else {
	goto failed;
      }
    } else {
      goto failed;
    }
  } else if (!strncmp (omf_cat, "CDE", 3)) {
    result = strdup ("DesktopSettings");
  } else if (!strncmp (omf_cat, "Development", 11)) {
    next = &(omf_cat[11]);
    while (*next == '|') next++;
    if (!next || *next == 0 || !strcmp (next, "Other")) {
      result = strdup ("Development");
    } else if (!strncmp (next, "Databases", 9)) {
      result = strdup ("Database");
    } else if (!strncmp (next, "Development Tools", 17)) {
      next = &(next[17]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other") ||
	  !strcmp (next, "Code Generators") ||
	  !strcmp (next, "Configuration") ||
	  !strcmp (next, "Packaging") ||
	  !strcmp (next, "RAD")) {
	result = strdup ("Development");
      } else if (!strncmp (next, "Build Tools", 11)) {
	result = strdup ("Building");
      } else if (!strncmp (next, "Debuggers", 9)) {
	result = strdup ("Debugger");
      } else if (!strncmp (next, "IDEs", 4)) {
	result = strdup ("IDE");
      } else if (!strncmp (next, "Profiling", 9)) {
	result = strdup ("Profiling");
      } else if (!strncmp (next, "Version Control", 15)) {
	result = strdup ("RevisionControl");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Environments", 12) ||
	       !strncmp (next, "Kernels", 7) ||
	       !strncmp (next, "Libraries", 9) ||
	       !strncmp (next, "System Calls", 12)) {
      /* Another don't care.  All go into dev. */
      result = strdup ("Development");
    } else {
      goto failed;
    }
  } else if (!strncmp (omf_cat, "General", 7)) {
    next = &(omf_cat[7]);
    while (*next == '|') next++;
    if (!next || *next == 0 || !strcmp (next, "Other")) {
      result = strdup ("Documentation");
    } else if (!strncmp (next, "Licenses", 8)) {
      result = strdup ("Documentation");
    } else if (!strncmp (next, "Linux", 5)) {
      /* Another don't care.  All go in core */
      result = strdup ("Core");
    } else {
      goto failed;
    }
  } else if (!strncmp (omf_cat, "GNOME", 5)) {
    next = &(omf_cat[6]);
    while (*next == '|') next++;
    if (!next || *next == 0 || !strcmp (next, "Other") ||
	!strncmp (next, "Desktop", 7)) {
      result = strdup ("Core");
    } else if (!strncmp (next, "Accessibility", 13)) {
      result = strdup ("Accessibility");
    } else if (!strncmp (next, "Applications", 12)) {
      /* Grr.  Fake categories galore */
      next = &(next[12]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other") ||
	  !strncmp (next, "Accessories", 11)) {
	result = strdup ("Utility");
      } else if (!strncmp (next, "Amusement", 9) ||
		 !strncmp (next, "Games", 5)) {
	result = strdup ("Game");
      } else if (!strncmp (next, "Clock", 5)) {
	result = strdup ("Clock");
      } else if (!strncmp (next, "Monitors", 8)) {
	result = strdup ("Monitor");
      } else if (!strncmp (next, "Multimedia", 10)) {
	result = strdup ("AudioVideo");
      } else if (!strncmp (next, "Network", 7) || 
		 !strncmp (next, "Internet", 8)) {
	result = strdup ("Network");
      } else if (!strncmp (next, "Utility", 7)) {
	result = strdup ("Utility");
      } else if (!strncmp (next, "Accessories", 11)) {
	/* Grr. people ignoring categorisation */
	result = strdup ("Applet");
      }
    } else if (!strncmp (next, "Applets", 7)) {
      next = &(next[7]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other")) {
	result = strdup ("Applet");
      } else if (!strncmp (next, "Amusement", 9)) {
	result = strdup ("Game");
      } else if (!strncmp (next, "Clock", 5)) {
	result = strdup ("Clock");
      } else if (!strncmp (next, "Monitors", 8)) {
	result = strdup ("Monitor");
      } else if (!strncmp (next, "Multimedia", 10)) {
	result = strdup ("AudioVideo");
      } else if (!strncmp (next, "Network", 7) || 
		 !strncmp (next, "Internet", 8)) {
	result = strdup ("Network");
      } else if (!strncmp (next, "Utility", 7)) {
	result = strdup ("Utility");
      } else if (!strncmp (next, "Accessories", 11)) {
	/* Grr. people ignoring categorisation */
	result = strdup ("Applet");
      } else {
	goto failed;
      }
    } else if (!strncmp (next, "Utilities", 9)) {
      result = strdup ("Utility");
    } else if (!strncmp (next, "Development", 11)) {
      /* Don't care */
      result = strdup ("Development");
    } else if (!strncmp (next, "Games", 5)) {
      result = strdup ("Game");
    } else if (!strncmp (next, "Graphics", 8)) {
      result = strdup ("Graphics");
    } else if (!strncmp (next, "Internet", 8)) {
      result = strdup ("Network");
    } else if (!strncmp (next, "Multimedia", 10)) {
      result = strdup ("AudioVideo");
    } else if (!strncmp (next, "Settings", 8)) {
      result = strdup ("Settings");
    } else if (!strncmp (next, "System", 6)) {
      result = strdup ("System");
    } else {
      goto failed;
    }
  } else if (!strncmp (omf_cat, "KDE", 3)) {
    next = &(omf_cat[3]);
    while (*next == '|') next++;
    if (!next || *next == 0 || !strcmp (next, "Other") ||
	!strcmp (next, "Programs") ||
	!strcmp (next, "Applications")) {
      result = strdup ("KDE");
    } else if (!strncmp (next, "Utilities", 9)) {
      result = strdup ("Utility");
    } else if (!strncmp (next, "Games", 5)) {
      result = strdup ("Game");
    } else if (!strncmp (next, "Graphics", 8)) {
      result = strdup ("Graphics");
    } else if (!strncmp (next, "Internet", 8)) {
      result = strdup ("Network");
    } else if (!strncmp (next, "Multimedia", 10)) {
      result = strdup ("AudioVideo");
    } else if (!strncmp (next, "Settings", 8)) {
      result = strdup ("Settings");
    } else if (!strncmp (next, "System", 6)) {
      result = strdup ("System");
    } else if (!strncmp (next, "Development", 11)) {
      result = strdup ("Development");
    } else {
      goto failed;
    }
  } else if (!strncmp (omf_cat, "System", 6)) {
    next = &(omf_cat[6]);
    while (*next == '|') next++;
    if (!next || *next == 0 || !strcmp (next, "Other")) {
      result = strdup ("System");
    } else if (!strncmp (next, "Administration", 14)) {
      next = &(next[14]);
      while (*next == '|') next++;
      if (!next || *next == 0 || !strcmp (next, "Other") ||
	  !strcmp (next, "Users")) {
	result = strdup ("Settings");
      } else if (!strncmp (next, "Backups", 7)) {
	result = strdup ("Archiving");
      } else if (!strncmp (next, "Filesystems", 11)) {
	result = strdup ("Filesystem");
      } else if (!strncmp (next, "Networking", 10)) {
	result = strdup ("Network");
      } else if (!strncmp (next, "Configuration", 13)) {
	/* Yet another don't care */
	result = strdup ("Settings");
      } else if (!strncmp (next, "Hardware", 8)) {
	/* Another don't care */
	result = strdup ("HardwareSettings");
      } else if (!strncmp (next, "Package Management", 18)) {
	result = strdup ("PackageManager");
      } else if (!strncmp (next, "Security", 8)) {
	result = strdup ("Security");
      } else if (!strncmp (next, "Services", 8)) {
	next = &(next[8]);
	while (*next == '|') next++;
	if (!next || *next == 0) {
	  result = strdup ("System");
	} else if (!strncmp (next, "Printing", 8)) {
	  result = strdup ("Printing");
	} else {
	  result = strdup ("System");
	}
      } else {
	goto failed;
      }
    } else {
      goto failed;
    }
  } else {
  failed:
    fprintf (stderr, "OMF category \'%s\' not recognised, ignoring.\n", 
         omf_cat);
  }

  return result;
}
