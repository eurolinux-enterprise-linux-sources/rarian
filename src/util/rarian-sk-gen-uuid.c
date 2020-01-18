/*
 * rarian-sk-gen-uuid.c
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

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[])
{
  int i;
  char *rand_string;//[38];
  char *tmp;
  unsigned int r;

  r = time(NULL);

  tmp = malloc (sizeof(char) *2);
  rand_string = malloc (sizeof(char) * 39);
  *rand_string = '0';

  srand (r);
  for (i=0; i<8; i++) {
    sprintf(tmp, "%x", (int) (((rand()*1.0) / (RAND_MAX*1.0)) * 16));
    strcat (rand_string, tmp);
  }
  rand_string[i] = '-';
  for (i=9; i < 13; i++) {
    sprintf(tmp, "%x", (int) (((rand()*1.0) / (RAND_MAX*1.0)) * 16));
    strcat (rand_string, tmp);
  }
  rand_string[i] = '-';
  for (i=14; i < 18; i++) {
    sprintf(tmp, "%x", (int) (((rand()*1.0) / (RAND_MAX*1.0)) * 16));
    strcat (rand_string, tmp);
  }
  rand_string[i] = '-';
  for (i=19; i < 23; i++) {
    sprintf(tmp, "%x", (int) (((rand()*1.0) / (RAND_MAX*1.0)) * 16));
    strcat (rand_string, tmp);
  }
  rand_string[i] = '-';
  for (i=24; i < 37; i++) {
    sprintf(tmp, "%x", (int) (((rand()*1.0) / (RAND_MAX*1.0)) * 16));
    strcat (rand_string, tmp);
  }

  rand_string[i] = '0';
  printf ("%s\n", rand_string);
  return 0;
}
