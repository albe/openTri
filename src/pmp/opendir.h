/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
"opendir" c code (~3 lines of perl -> ~100 lines of c :s)
*/


#ifndef opendir_h
#define opendir_h


#include <pspiofilemgr.h>
#include <sys/unistd.h>
#include <string.h>
#include <ctype.h>
#include "mem64.h"


#define SORT_DEFAULT 0
#define SORT_NAME 1
#define SORT_SIZE 2
#define SORT_MDATE 3
#define SORT_CDATE SORT_DEFAULT
#define SORT_MASK 0xff
#define SORT_REVERSE 0x100


struct opendir_struct
	{
	SceUID directory;

	SceIoDirent *directory_entry;
	
	unsigned int number_of_directory_entries;
	};


void opendir_safe_constructor(struct opendir_struct *p);
void opendir_close(struct opendir_struct *p);
char *opendir_open(struct opendir_struct *p, char *directory, char **filter, int sort);


#endif
