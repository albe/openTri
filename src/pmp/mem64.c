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
64 bytes aligned malloc, with the me is crucial to well enclose data (due to cache)
*/


#include "mem64.h"


void *malloc_64(int size)
	{
	int mod_64 = size & 0x3f;

	if (mod_64 != 0) size += 64 - mod_64;

	return(memalign(64, size));
	}


void free_64(void *p)
	{
	free(p);
	}
