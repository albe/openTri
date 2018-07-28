/*
 * triRefcount.c: Code for refcounting ressources
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 Alexander Berl 'Raphael' <raphael@fx-world.org>
 *
 * $Id: $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include "triRefcount.h"
#include "triMemory.h"


typedef struct triRefcount
{
	triChar*	id;
	triVoid*	data;
	triS32		refs;
	
	struct triRefcount* next;
} triRefcount;


static triRefcount* triRefcountList = 0;


triVoid triRefcountCreate( const triChar* id, triVoid* data )
{
	if (id==0 || data==0) return;
	triRefcount* r = triMalloc(sizeof(triRefcount));
	if (r==0) return;
	r->id = triMalloc(strlen(id)+1);
	strcpy(r->id, id);
	r->id[strlen(id)] = 0;
	r->data = data;
	r->refs = 1;
	r->next = triRefcountList;
	triRefcountList = r;
}


// returns pointer to data if already in refcount list
// else NULL (create a new refcount for that id)
triVoid* triRefcountRetain( const triChar* id )
{
	if (id==0) return(0);
	triRefcount* r = triRefcountList;
	
	while (r!=0)
	{
		if (stricmp(id, r->id)==0)
		{
			r->refs++;
			return(r->data);
		}
		r = r->next;
	}
	return(0);
}



// returns pointer to data if already in refcount list
// else NULL (create a new refcount for that ptr)
triVoid* triRefcountRetainPtr( const triVoid* ptr )
{
	if (ptr==0) return(0);
	triRefcount* r = triRefcountList;
	
	while (r!=0)
	{
		if (ptr == r->data)
		{
			r->refs++;
			return(r->data);
		}
		r = r->next;
	}
	return(0);
}


// returns 0 if the resource can be freed savely
triS32 triRefcountRelease( const triVoid* data )
{
	if (data==0) return -1;
	triRefcount* r = triRefcountList;
	triRefcount* l = 0;
	
	while (r!=0)
	{
		if (data == r->data)
		{
			r->refs--;
			if (r->refs==0)
			{
				if (l==0)
					triRefcountList = r->next;
				else
					l->next = r->next;
				triFree(r->id);
				triFree(r);
				return(0);
			}
			return(r->refs);
		}
		l = r;
		r = r->next;
	}
	return(0);
}

