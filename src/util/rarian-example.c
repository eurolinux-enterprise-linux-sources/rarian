/*
 * rarian-example.c
 * This file is part of Rarian
 *
 * Copyright (C) 2006 - Don Scorgie
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

#include <stdio.h>
#include <stdlib.h>
#define I_KNOW_RARIAN_0_8_IS_UNSTABLE
#include <rarian.h>

void
print_section (RrnSect *sect, int depth)
{
	int i=0;
	RrnSect *child;
	for (i=0; i < depth; i++) {
		printf (" ");
	}
	printf ("Child: %s\n", sect->name);
	for (i=0; i < depth; i++) {
		printf (" ");
	}
	printf ("-> %s\n\n", sect->uri);
	child = sect->children;

	while (child) {
		print_section (child, depth+1);
		child = child->next;
	}

}

int
for_each_cb (RrnReg *reg, void * user_data)
{
  RrnSect *sect;
  char **cats;
    if (!reg) {
        printf ("Error: No reg passed in!\n");
        exit (6);
    }
    printf ("Document: %s\n", reg->name);
    printf ("-> %s\n", reg->uri);
    printf ("-> ghelp:%s\n", reg->ghelp_name);
    cats = reg->categories;
    if (cats && *cats)
      printf ("Categories:\n");
    while (cats && *cats) {
      printf ("-> %s\n", *cats);
      cats++;
    }
    printf ("\n");

    sect = reg->children;
    while (sect) {
      print_section (sect, 1);
      sect = sect->next;
    }

    return TRUE;
}

info_for_each (RrnInfoEntry *entry, void *data)
{
  if (entry->section)
    printf ("Info page: %s\n\tPath: %s#%s\n\tComment: %s\n",
	    entry->doc_name, entry->base_filename,
	    entry->section, entry->comment);
  else
    printf ("Info page: %s\n\tPath: %s\n\tComment: %s\n",
	    entry->doc_name, entry->base_filename,
	    entry->comment);
  return TRUE;
}

man_for_each (RrnManEntry *entry, void *data)
{
  printf ("Man page %s\n", entry->name);
  printf ("\tPath: %s\n", entry->path);
  return TRUE;
}

int
main(int argc, char *argv[])
{
  char **cats =  rrn_info_get_categories ();
  char **iter = cats;
  
  rrn_man_for_each ((RrnManForeachFunc) man_for_each, NULL);
  
  while (iter && *iter) {
    printf ("Info category: %s\n", *iter);
    iter++;
  }

  rrn_info_for_each ((RrnInfoForeachFunc) info_for_each, NULL);
  
  rrn_for_each((RrnForeachFunc) for_each_cb, NULL);

  rrn_shutdown ();
  rrn_info_shutdown ();
  rrn_man_shutdown ();
  
  return 0;
}
