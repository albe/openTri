/*
 * triTexman.c: Code for Texture manager
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

#ifdef __PSP__
#include <pspgu.h>
#include <pspkernel.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "triTypes.h"
#include "triGraphics.h"
#include "triMemory.h"
#include "triVAlloc.h"
#include "triTexman.h"
#include "triImage.h"
#include "triRefcount.h"
#include "triLog.h"
#include "streams/streams.h"

#ifdef DEBUG
#include "triInput.h"
#include <pspctrl.h>
#endif

typedef struct triTextureList
{
	triTexture		*tex;
	triU32			mfuCount;			// counter for number of usages (textureBind calls)
	
	struct triTextureList	*prev_prio;	// priority linkage
	struct triTextureList	*next_prio;	// priority linkage
	struct triTextureList	*next;		// hash linkage
} triTextureList;


#define TRI_TEXMAN_HASHSIZE 32

struct triTexman
{
	triTextureList	*hash[TRI_TEXMAN_HASHSIZE];
	triTextureList	*priority[TRI_TEXMAN_PRIORITIES];
	
	triS32			id_counter;
} triTexman = { {NULL}, {NULL}, 0 };

#define HASH(x) ((x)%TRI_TEXMAN_HASHSIZE)


static void triTextureSafeConstructor( triTexture* tex )
{
	if (tex==0) return;
	tex->id = -1;
	tex->vram = tex->data = 0;
	tex->width = tex->height = 0;
	tex->format = 0;
	tex->level = 0;
	tex->swizzle = 0;
	tex->refcount = 0;
	tex->priority = 0;
	tex->allocated = 0;
	
	tex->mipmaps = 0;
	tex->next = 0;
}


static void triTextureVFree( triTexture* tex )
{
	if (tex==0) return;
	if (tex->vram!=0) triVFree(tex->vram);
	tex->vram = 0;
	// TODO: Download mipmaps to RAM?
}


// Allocate the space for a texture in VRAM, removing least frequently used less priority textures
static void triTextureVAlloc( triTexture* tex )
{
	if (tex==0) return;
	triS32 i = 0;
	while (triVLargestblock()<tex->size && i<=tex->priority && i<TRI_TEXMAN_PRIORITIES)
	{
		triTextureList *list = triTexman.priority[i++];

		// There isn't enough free space, so free up textures until there's enough
		do
		{
			triTextureVFree( list->tex );
			list = list->next_prio;
		} while (triVLargestblock()<tex->size && list!=0);
	}
	
	tex->vram = triVAlloc( tex->size );
}


static void triTextureListInsert( triTexture* tex )
{
	if (tex==0) return;
	
	triTextureList* new_item = triMalloc(sizeof(triTextureList));
	if (new_item==0) return;
	
	triTextureList* list = triTexman.hash[HASH(tex->id)];
	triTextureList* root = triTexman.priority[tex->priority];
	
	new_item->tex = tex;
	// hash insert
	new_item->next = list;
	triTexman.hash[HASH(tex->id)] = new_item;
	// priority insert
	new_item->next_prio = root;
	new_item->prev_prio = 0;
	triTexman.priority[tex->priority] = new_item;
}


static void triTextureListRemove( triS32 id )
{
	triTextureList* list = triTexman.hash[HASH(id)];
	if (list==0) return;
	
	triTextureList* prev = 0;
	while(list!=0)
	{
		if (list->tex->id==id)
		{
			if (prev==0)
				triTexman.hash[HASH(id)] = list->next;
			else
				prev->next = list->next;

			if (list == triTexman.priority[list->tex->priority])
			{
				triTexman.priority[list->tex->priority] = list->next_prio;
				if (list->next_prio!=0)
					list->next_prio->prev_prio = 0;
			}
			else
			{
				if (list->prev_prio!=0)
					list->prev_prio->next_prio = list->next_prio;
				if (list->next_prio!=0)
					list->next_prio->prev_prio = list->prev_prio;
			}

			if (list->tex->vram!=0)
				triVFree(list->tex->vram);
			triFree(list->tex);
			triFree(list);
			return;
		}
		prev = list;
		list = list->next;
	}
}


static void triTextureListPriorityChange( triTexture* tex, triS32 priority )
{
	//if (tex==0 || tex->priority==priority) return;
	triTextureList* list = triTexman.priority[tex->priority];
	
	if (list->tex==tex)
	{
		triTexman.priority[tex->priority] = list->next_prio;
		if (list->next_prio!=0)
			list->next_prio->prev_prio = 0;
	}
	else
	{
		if (list->prev_prio!=0)
			list->prev_prio->next_prio = list->next_prio;
		if (list->next_prio!=0)
			list->next_prio->prev_prio = list->prev_prio;
	}
	
	if (triTexman.priority[priority]==0)
	{
		triTexman.priority[priority] = list;
		list->prev_prio = list->next_prio = 0;
	}
	else
	{
		triTextureList* plist = triTexman.priority[priority];
		
		while (plist->mfuCount<list->mfuCount)
		{
			plist = plist->next_prio;
		}

		list->next_prio = plist;
		list->prev_prio = plist->prev_prio;
		if (plist->prev_prio!=0)
			plist->prev_prio->next_prio = list;
		plist->prev_prio = list;
		
		if (plist==triTexman.priority[priority])
			triTexman.priority[priority] = list;
	}
}


static inline triTexture* triTextureListFind( triS32 id )
{
	triTextureList* list = triTexman.hash[HASH(id)];

	while(list!=0)
	{
		if (list->tex->id==id) return list->tex;
		list = list->next;
	}
	
	return 0;
}


triS32 triTextureGen( triS32 n, triS32 *id )
{
	triS32 i = 0;
	for (;i<n;i++)
	{
		id[i] = -1;
		triTexture* tex = triMalloc(sizeof(triTexture));
		if (tex==0) return(-1);
		triTextureSafeConstructor( tex );
		tex->id = id[i] = triTexman.id_counter++;
		tex->priority = 0;
		tex->refcount = 1;
		triTextureListInsert( tex );
	}
	return(0);
}


triS32 triTexturePrioritize( triS32 n, triS32 *id, triS32 *priorities )
{
	triS32 i = 0;
	for (;i<n;i++)
	{
		triTexture* tex = triTextureListFind( id[i] );
		if (tex==0) continue;
		if (priorities[i]!=tex->priority)
			triTextureListPriorityChange( tex, priorities[i] );
	}
	return(0);
}


// Check if one texture is local (in VRAM)
triS32 triTextureIslocal( triS32 id )
{
	triTexture* tex = triTextureListFind( id );
	if (tex==0) return 0;
	
	return (tex->vram!=0);
}

static triS32 last_bind = -1;

triS32 triTextureBind( triS32 id )
{
	if (id<0) return(-1);
	
	triS32 hashID = HASH(id);
	triTextureList* list = triTexman.hash[hashID];
	triTextureList* prev = 0;

	// Find texture
	while(list!=0)
	{
		if (list->tex->id==id) break;
		prev = list;
		list = list->next;
	}
	
	if (list==0) return(-1);
	list->mfuCount++;
	// Order Hash list based on mfu count to find MFU items faster
	if (prev!=0 && prev->mfuCount<=list->mfuCount)
	{
		prev->next = list->next;
		list->next = prev;
		if (triTexman.hash[hashID]==prev)
		{
			triTexman.hash[hashID] = list;
		}
	}
	
	// Reorder priority list for MFU so that the automatic VRAM handling can
	// throw out unused items.
	if (list->next_prio!=0 && list->next_prio->mfuCount<=list->mfuCount)
	{
		if (list->prev_prio!=0)
			list->prev_prio->next_prio = list->next_prio;
		else
			triTexman.priority[list->tex->priority] = list->next_prio;
		
		triTextureList* temp = list->next_prio->next_prio;
		if (temp!=0)
			temp->prev_prio = list;
		
		list->next_prio->next_prio = list;
		list->next_prio->prev_prio = list->prev_prio;
		list->prev_prio = list->next_prio;
		list->next_prio = temp;
	}
	
	sceGuEnable(GU_TEXTURE_2D);
	//if (last_bind==id) return(0);	// Quick out when we bind a already bound texture
	
	triTexture* tex = list->tex;
	switch (tex->format)
	{
		case GU_PSM_T4:
			sceGuClutMode(tex->palformat, 0, 0xff, 0);
			sceGuClutLoad(2, tex->paldata);
			break;
		case GU_PSM_T8:
			sceGuClutMode(tex->palformat, 0, 0xff, 0);
			sceGuClutLoad(32, tex->paldata);
			break;
	}

	if (tex->vram!=0)
	{
		sceGuTexSync();
		sceGuTexMode(tex->format, tex->mipmaps, 0, 1);		// texture in VRAM always swizzled
		sceGuTexImage(tex->level, tex->width, tex->height, tex->width, tex->vram);
	}
	else
	{
		sceGuTexMode(tex->format, tex->mipmaps, 0, tex->swizzle);
		sceGuTexImage(tex->level, tex->width, tex->height, tex->width, tex->data);
	}

	if (tex->mipmaps>0)
	{
		sceGuTexFilter(GU_LINEAR_MIPMAP_LINEAR,GU_LINEAR_MIPMAP_LINEAR);
		while (tex->next!=0)
		{
			tex = tex->next;
			if (tex->vram!=0)
				sceGuTexImage(tex->level, tex->width, tex->height, tex->width, tex->vram );
			else
				sceGuTexImage(tex->level, tex->width, tex->height, tex->width, tex->data);
		}
	}
	else
	{
		sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	}
	
	last_bind = id;
	return(0);
}


triS32 triTextureRel( triS32 n, triS32 *id )
{
	triS32 i = 0;
	for (;i<n;i++)
	{
		if (last_bind==id[i]) last_bind = -1;
		triTexture* tex = triTextureListFind( id[i] );
		if (tex==0) continue;
		if (--tex->refcount==0)
		{
			if (tex->allocated)
			{
				triFree(tex->data);
				tex->data = 0;
				if (tex->paldata)
					triFree(tex->paldata);
				tex->paldata = 0;
				tex->allocated = 0;
			}
			triTextureVFree( tex );
			
			triTexture* next = tex->next;
			while (next!=0)
			{
				tex = next;
				if (tex->allocated)
				{
					triFree(tex->data);
					tex->data = 0;
					tex->paldata = 0;
					tex->allocated = 0;
				}
				triTextureVFree( tex );
				next = tex->next;
				triFree( tex );
			}
			
			triTextureListRemove( id[i] );
		}
	}
	return(0);
}


#define VRAM_BASE 0x04000000
#define UNCACHED_POINTER 0x40000000

static void swizzle_upload(triU8 *dest, triU8 *source, triS32 width, triS32 height)
{
	triS32 i,j;
	triS32 rowblocks = (width / 16);
	triS32 rowblocks_add = (rowblocks-1)*128;
	triU32 block_address = 0;
	triU32 *src = (triU32*)source;

	// (j-blocky*8)*16 + (i-blockx*16) + (blockx + blocky*rowblocks)*16*8 =
	// j*16 + (blocky*rowblocks-blocky)*16*8 + i + (blockx*8-blockx)*16 =   | blocky8 := j/8*8 = j&~0x7, blockx16 := i/16*16 = i&~0xf
	// j*16 + blocky8*(rowblocks-1)*16 + i + blockx16*8-blockx16 =   | k = 0..15, rowmul := (rowblocks-1)*16
	// j*16 + blocky8*rowmul + k + blockx16 + blockx16*8-blockx16 =

	for (j = 0; j < height; j++,block_address+=16)
	{
		triU32 *block;
		if ((triU32)dest>=VRAM_BASE+0x00200000)
			block = (triU32*)&dest[block_address];
		else
			block = (triU32*)((triU32)&dest[block_address] | UNCACHED_POINTER);
		for (i = 0; i < rowblocks; i++)
		{
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			block += 28;
		}

		if ((j&0x7)==0x7)
			block_address += rowblocks_add;
	}
}


triS32 triTextureSize( triS32 width, triS32 height, triS32 format )
{
	switch(format)
	{
		case GU_PSM_T4:
			return(width*height>>1);
			
		case GU_PSM_T8:
			return(width*height);
		
		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return(width*height<<1);
		
		case GU_PSM_8888:
		case GU_PSM_T32:	
			return(width*height<<2);
			
		case GU_PSM_DXT1:
			return(width*height>>1);
		
		case GU_PSM_DXT3:
		case GU_PSM_DXT5:
			return(width*height);
	}
	return(0);
}


static triS32 _triTextureUpload( triTexture* tex )
{
	if (tex==0) return(-1);

	if (tex->vram!=0)
		return(0);

	triTextureVAlloc(tex);
	// TODO: upload mipmaps
	if (tex->vram==0)
		return(-2);

	if (tex->swizzle==0)
		swizzle_upload( tex->vram, tex->data, tex->bytesperline, tex->height );
	else
		sceGuCopyImage( tex->format, 0, 0, tex->width, tex->height, tex->width, tex->data, 0, 0, tex->width, (triVoid*)((triU32)tex->vram | UNCACHED_POINTER) );

	return(0);
}


triS32 triTextureUpload( triS32 id, triS32 priority )
{
	triTexture* tex = triTextureListFind( id );
	if (tex==0) return(-1);
	
	if (priority>=TRI_TEXMAN_PRIORITIES) priority = TRI_TEXMAN_PRIORITIES-1;
	if (priority>=0)
	{
		if (tex->priority!=priority)
		{
			triTextureListPriorityChange( tex, priority );
			tex->priority = priority;
		}
	}

	return _triTextureUpload( tex );
}


// walk through texture list and (re)upload mfu, highest priority
static triS32 triTextureManageUploads()
{
	triS32 i = TRI_TEXMAN_PRIORITIES-1;
	while (i>0)
	{
		triTextureList* list = triTexman.priority[i--];
		while (list!=0)
		{
			triS32 ret = 0;
			if (list->tex->vram==0)
				ret = _triTextureUpload( list->tex );
			if (ret<0) return(ret);
			
			list = list->next_prio;
		}
	}
	return(0);
}


#define ISPOW2(x) ((x&~(x-1))==x)
triS32 triTextureImage( triS32 id, triS32 level, triS32 width, triS32 height, triS32 format, triS32 swizzled, triVoid* data, triS32 palformat, triVoid* paldata )
{
	if (!ISPOW2(width) || !ISPOW2(height)) return(-1);
	
	triTextureList* list = triTexman.hash[HASH(id)];

	while(list!=0)
	{
		if (list->tex->id==id) break;
		list = list->next;
	}
	
	if (list==0) return(-1);
	
	list->mfuCount = 0;
	triTexture* tex = list->tex;
	triTexture* last = 0;
	while (tex!=0 && tex->level<level)
	{
		tex->mipmaps++;
		last = tex;
		tex = tex->next;
	}
	
	if (tex==0)
	{
		tex = triMalloc( sizeof(triTexture) );
		memset( tex, 0, sizeof(triTexture) );
	}
	if (tex->data==data && tex->width==width && tex->height==height && tex->format==format) return(0);
	
	if (last==0)
		list->tex = tex;
	else
		last->next = tex;
	
	tex->level = level;
	tex->width = width;
	tex->height = height;
	tex->bytesperline = triTextureSize( width, 1, format );
	tex->format = format;
	tex->size = triTextureSize( width, height, format );
	tex->swizzle = swizzled;
	tex->data = data;
	tex->palformat = palformat;
	tex->paldata = paldata;

	if (last_bind==id) last_bind = -1;
	
	// Try to upload texture if there is enough VRAM, don't force (triTextureVAlloc)
	if (tex->vram!=0) triVFree(tex->vram);
	tex->vram = triVAlloc(tex->size);
	if (tex->vram==0) return(0);

	if (tex->swizzle==0)
		swizzle_upload( tex->vram, tex->data, tex->bytesperline, tex->height );
	else
		sceGuCopyImage( tex->format, 0, 0, tex->width, tex->height, tex->width, tex->data, 0, 0, tex->width, (triVoid*)((triU32)tex->vram | UNCACHED_POINTER) );
	return(0);
}


triS32 triTextureImage2( triS32 id, triS32 level, triImage* img )
{
	triU32 i = 1;
	triMipLevel* lev = img->level;
	triU32 ret = triTextureImage( id, level, img->stride, img->tex_height, img->format, img->swizzled, img->data, img->palformat, img->palette );
	while (lev!=0 && level+i<=7)
	{
		ret = (ret << 1) | triTextureImage( id, level+i, lev->stride, lev->tex_height, img->format, img->swizzled, lev->data, img->palformat, img->palette );
		lev = lev->next;
		i++;
	}
	return ret;
}


#define HIGHMASK4444	0x3333
#define LOWMASK4444		0x3333
#define HIGHMASK5551	0x1CE7
#define LOWMASK5551		0x8C63
#define HIGHMASK5650	0x39E7
#define LOWMASK5650		0x1863
#define HIGHMASK8888	0x3F3F3F3F
#define LOWMASK8888		0x03030303

#define bilerp(mask, a,b,c,d) \
		(((a >> 2) & HIGHMASK##mask) + ((b >> 2) & HIGHMASK##mask) + ((c >> 2) & HIGHMASK##mask) + ((d >> 2) & HIGHMASK##mask) +\
		 (((a & LOWMASK##mask) + (b & LOWMASK##mask) + (c & LOWMASK##mask) + (d & LOWMASK##mask)) >> 2))


static inline triFloat distance8888( triU32 c1, triU32 c2 )
{
	float r = ((float)(c1 & 0xFF) - (float)(c2 & 0xFF));
	float g = ((float)((c1 >> 8) & 0xFF) - (float)((c2 >> 8) & 0xFF));
	float b = ((float)((c1 >> 16) & 0xFF) - (float)((c2 >> 16) & 0xFF));
	//float a = ((float)((c1 >> 24) & 0xFF) - (float)((c2 >> 24) & 0xFF));
	return (r*r + g*g + b*b);
}

static inline triFloat distance4444( triU32 c1, triU32 c2 )
{
	float r = ((float)(c1 & 0xF) - (float)(c2 & 0xF));
	float g = ((float)((c1 >> 4) & 0xF) - (float)((c2 >> 4) & 0xF));
	float b = ((float)((c1 >> 8) & 0xF) - (float)((c2 >> 8) & 0xF));
	//float a = ((float)((c1 >> 12) & 0xF) - (float)((c2 >> 12) & 0xF));
	return (r*r + g*g + b*b);
}

static inline triFloat distance5551( triU32 c1, triU32 c2 )
{
	float r = ((float)(c1 & 0x1F) - (float)(c2 & 0x1F));
	float g = ((float)((c1 >> 5) & 0x1F) - (float)((c2 >> 5) & 0x1F));
	float b = ((float)((c1 >> 10) & 0x1F) - (float)((c2 >> 10) & 0x1F));
	//float a = ((float)((c1 >> 15) & 0x1) - (float)((c2 >> 15) & 0x1));
	return (r*r + g*g + b*b);
}

static inline triFloat distance5650( triU32 c1, triU32 c2 )
{
	float r = ((float)(c1 & 0x1F) - (float)(c2 & 0x1F));
	float g = ((float)((c1 >> 5) & 0x3F) - (float)((c2 >> 5) & 0x3F));
	float b = ((float)((c1 >> 11) & 0x1F) - (float)((c2 >> 11) & 0x1F));
	return (r*r + g*g + b*b);
}


typedef struct NearestColorCacheEntry {
	int			color;
	int			index;
	
	struct NearestColorCacheEntry*	next;
	struct NearestColorCacheEntry*	prev;
} NearestColorCacheEntry;

static NearestColorCacheEntry* NearestColorCache;
static NearestColorCacheEntry* NearestColorCacheLast;
static int NearestColorCacheSize = 0;
#define NearestColorCacheMaxSize 128

static NearestColorCacheEntry NearestColorCacheMem[NearestColorCacheMaxSize];


static triU32 triNearestColorIndex( triTexture* src, triU32 color )
{
	if (NearestColorCacheSize>0)
	{
		NearestColorCacheEntry* list = NearestColorCache;
		while (list)
		{
			if (list->color==color)
			{
				if (NearestColorCache!=list)
				{
					if (list->prev)
						list->prev->next = list->next;
					if (list->next)
						list->next->prev = list->prev;
					if (NearestColorCacheLast==list)
						NearestColorCacheLast = list->prev;
					list->next = NearestColorCache;
					list->prev = 0;
					
					if (NearestColorCache)
						NearestColorCache->prev = list;
					NearestColorCache = list;
				}
				return list->index;
			}
			list = list->next;
		}
	}

	triU32 numEntries = 256;
	if (src->format==GU_PSM_T4) numEntries = 16;

	triU32 i;
	triFloat minDist = 255.f * 255.f * 5.f, dist = 255.f * 255.f * 5.f;
	triU32 minIndex = 0;
	triU16* pal16 = (triU16*)((triU32)src->paldata/*|0x40000000*/);
	triU32* pal32 = (triU32*)((triU32)src->paldata/*|0x40000000*/);

	switch (src->palformat)
	{
		case GU_PSM_4444:
			for (i=0;i<numEntries;i++)
			{
				dist = distance4444( color, pal16[i] );
				if (dist < minDist)
				{
					minDist = dist;
					minIndex = i;
				}
			}
			break;
		case GU_PSM_5551:
			for (i=0;i<numEntries;i++)
			{
				dist = distance5551( color, pal16[i] );
				if (dist < minDist)
				{
					minDist = dist;
					minIndex = i;
				}
			}
			break;
		case GU_PSM_5650:
			for (i=0;i<numEntries;i++)
			{
				dist = distance5650( color, pal16[i] );
				if (dist < minDist)
				{
					minDist = dist;
					minIndex = i;
				}
			}
			break;
		case GU_PSM_8888:
			for (i=0;i<numEntries;i++)
			{
				dist = distance8888( color, pal32[i] );
				if (dist < minDist)
				{
					minDist = dist;
					minIndex = i;
				}
			}
			break;
	}
	
	if (NearestColorCacheSize<NearestColorCacheMaxSize)
	{
		NearestColorCacheEntry* list = &NearestColorCacheMem[NearestColorCacheSize++];
		list->color = color;
		list->index = minIndex;
		
		list->next = NearestColorCache;
		list->prev = 0;
		if (NearestColorCache)
			NearestColorCache->prev = list;
		NearestColorCache = list;
		if (NearestColorCacheLast==0)
			NearestColorCacheLast = list;
	}
	else
	{
		NearestColorCacheEntry* list = NearestColorCacheLast;
		NearestColorCacheEntry* newlast = NearestColorCacheLast->prev;
		newlast->next = 0;
		list->color = color;
		list->index = minIndex;
		
		list->next = NearestColorCache;
		list->prev = 0;
		
		if (NearestColorCache)
			NearestColorCache->prev = list;
		NearestColorCache = list;
		NearestColorCacheLast = newlast;
	}
	
	return minIndex;
}


// Create a downsampled version of src and store it into dst
static triS32 triTextureBuildMipmapLevel( triTexture* src, triTexture* dst )
{
	if (src==0 || dst==0) return(-1);
	
	if (src->format>=GU_PSM_DXT1) return(-1);
	
	dst->width = src->width/2;
	dst->height = src->height/2;
	if (dst->width<4 || dst->height<4) return(-1);
	dst->format = src->format;
	dst->bytesperline = src->bytesperline/2;
	dst->level = src->level+1;
	dst->palformat = src->palformat;
	dst->paldata = src->paldata;
	dst->allocated = 1;
	dst->swizzle = 0;
	dst->vram = 0;
	dst->size = src->size/4;
	dst->data = triMalloc( dst->height * dst->bytesperline );
	if (dst->data==0) return(-1);
	
	triU32 psm = dst->format;
	if (src->format==GU_PSM_T4)
	{
		sceGuClutMode(src->palformat, 0, 0xff, 0);
		sceGuClutLoad(2, src->paldata);
		psm = src->palformat;
	}
	else if (src->format==GU_PSM_T8)
	{
		sceGuClutMode(src->palformat, 0, 0xff, 0);
		sceGuClutLoad(32, src->paldata);
		psm = src->palformat;
	}

	// The downsampled texture is 256x256x32bit max., this still fits into the 512x272x16bit min framebuffer
	sceGuDrawBufferList( psm, vrelptr(triFramebuffer), 256 );
	
	sceGuDisable( GU_DITHER );
	sceGuOffset(2048 - (256/2), 2048 - (256/2));
	sceGuViewport(2048, 2048, 256, 256);
	sceGuScissor(0, 0, 256, 256);
	
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_FALSE);

	if (src->vram!=0)
	{
		sceGuTexSync();
		sceGuTexMode(src->format, 0, 0, 1);		// texture in VRAM always swizzled
		sceGuTexImage(0, src->width, src->height, src->width, src->vram);
	}
	else
	{
		sceGuTexMode(src->format, 0, 0, src->swizzle);
		sceGuTexImage(0, src->width, src->height, src->width, src->data);
	}
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	
	triFloat start;
	triFloat cur_u = 0.0f;
	triFloat cur_x = 0.0f;
	triFloat x_end = dst->width;
	triFloat slice = 32.f;
	triFloat ustep = 2.0f * slice;

	// blit maximizing the use of the texture-cache
	for( start=0; start<x_end; start+=slice )
	{
		triVertUV* vertices = (triVertUV*)sceGuGetMemory(2 * sizeof(triVertUV));

		triFloat poly_width = ((cur_x+slice) > x_end) ? (x_end-cur_x) : slice;
		triFloat source_width = ((cur_u+ustep) > src->width) ? (src->width-cur_u) : ustep;

		vertices[0].u = cur_u;
		vertices[0].v = 0.0f;
		vertices[0].x = cur_x; 
		vertices[0].y = 0.0f; 
		vertices[0].z = 0.0f;

		cur_u += source_width;
		cur_x += poly_width;

		vertices[1].u = cur_u;
		vertices[1].v = src->height;
		vertices[1].x = cur_x;
		vertices[1].y = dst->height;
		vertices[1].z = 0.0f;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
	sceGuSync(0,2);
	if (psm == dst->format)
	{
		triLogPrint("Copying downsampled image to texture\n");
		sceGuCopyImage( psm, 0,0,dst->width,dst->height,256,triFramebuffer,0,0,dst->width,dst->data );
		sceGuTexSync();
	}
	else
	{
		triLogPrint("Mapping downsampled image (%ix%i) to palette\n", dst->width, dst->height);
		triU16* src16 = (triU16*)((triU32)triFramebuffer/*|0x40000000*/);
		triU32* src32 = (triU32*)((triU32)triFramebuffer/*|0x40000000*/);
		triS32 j = 0, i = 0;
		triU8* dst8 = dst->data;
		triU32 dstoffs = 0;
		for (j=0;j<dst->height;j++)
		{
			for (i=0;i<dst->width;i++)
			{
				triU32 idx = (psm==GU_PSM_8888)?triNearestColorIndex( dst, *src32++ ):triNearestColorIndex( dst, *src16++ );
				if (dst->format==GU_PSM_T4)
					dst8[dstoffs/2] = (i&1)?(dst8[dstoffs/2] | (idx << 4)):idx;
				else
					dst8[dstoffs] = idx;
				//triLogPrint("Mapped color %x to palette entry %i (%x)\n", *src32 & 0xFFFFFF, idx, ((triU32*)dst->paldata)[idx] & 0xFFFFFF );
				dstoffs++;
			}
			src32 += (256-dst->width);
			src16 += (256-dst->width);
		}
	}
	sceKernelDcacheWritebackAll();
	
/*
	sceGuTexMode(dst->format, 0, 0, 0);
	sceGuTexImage(0, dst->width, dst->height, dst->width, dst->data);

	cur_u = 0.0f;
	cur_x = dst->width;
	x_end = dst->width*2;
	slice = 32.f;
	ustep = 1.0f * slice;
	for( start=0; start<dst->width; start+=slice )
	{
		triVertUV* vertices = (triVertUV*)sceGuGetMemory(2 * sizeof(triVertUV));

		triFloat poly_width = ((cur_x+slice) > x_end) ? (x_end-cur_x) : slice;
		triFloat source_width = ((cur_u+ustep) > dst->width) ? (dst->width-cur_u) : ustep;

		vertices[0].u = cur_u;
		vertices[0].v = 0.0f;
		vertices[0].x = cur_x; 
		vertices[0].y = 0.0f; 
		vertices[0].z = 0.0f;

		cur_u += source_width;
		cur_x += poly_width;

		vertices[1].u = cur_u;
		vertices[1].v = dst->height;
		vertices[1].x = cur_x;
		vertices[1].y = dst->height;
		vertices[1].z = 0.0f;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
	sceGuSync(0,2);
	
	sceDisplaySetFrameBuf( triFramebuffer, 512, psm, 0 );

	while (1)
	{
		triInputUpdate ();
		if (triInputPressed (PSP_CTRL_CROSS))
			break;
	}
*/
	//float slope = 0.4f;
	//sceGuTexLevelMode(0, 1.f); // manual slope setting
	//sceGuTexSlope(slope); // the near from 0 slope is the lower (=best detailed) mipmap it uses
	
	triRendertoscreen();
	return(0);
}


triS32 triTextureBuildMipmaps( triS32 id, triS32 levels )
{
	triTexture* tex = triTextureListFind( id );
	if (tex==0) return(-1);
	triTexture* org = tex;
	
	if (tex->mipmaps!=0) return(0);	// make that mipmaps>=levels?
	
	triU32 states = sceGuGetAllStatus();
	
	// Reset nearest color cache (only necessary here because mipmaps share palette)
	NearestColorCacheSize = 0;
	NearestColorCache = 0;
	NearestColorCacheLast = 0;
	while (levels>0)
	{
		triTexture* mip = triMalloc(sizeof(triTexture));
		if (mip==0) return(-1);
		triTextureSafeConstructor( mip );
		
		if (triTextureBuildMipmapLevel( tex, mip )<0)
		{
			triFree(mip);
			org->mipmaps -= levels;
			sceGuSetAllStatus(states);
			return(-1);
		}
		tex->mipmaps = levels;
		tex->next = mip;
		
		tex = mip;
		levels--;
	}
	
	if (org->swizzle || org->vram)	// if original texture was swizzled
	{
		triS32 vram = (org->vram!=0);
		org = org->next;
		while (org!=0)
		{
			if (org->swizzle==0 && org->allocated==1)
			{
				triU8* temp = 0;
				if (vram)
					temp = triVAlloc( org->size );
				if (temp==0)
					temp = triMalloc( org->size );
				swizzle_upload( temp, org->data, org->bytesperline, org->height );
				triFree( org->data );
				if (vram && (triU32)temp <= 0x04200000)
				{
					org->allocated = 0;
					org->data = 0;
					org->vram = temp;
				}
				else
					org->data = temp;
				org->swizzle = 1;
			}
			org = org->next;
		}
	}
	sceGuSetAllStatus(states);
	return(0);
}


triS32 triTextureLoad( triChar* filename )
{
	triChar tname[512];
	snprintf( tname, 512, "TEX::%s", filename );
	
	triTexture* tex = (triTexture*)triRefcountRetain( tname );

	if (tex==0)
	{
		triImage* img = triImageLoad( filename, 0 );
		if (img==0)
		{
			triLogPrint("ERROR: Could not load image!\n");
			return(-1);
		}
		
		tex = triMalloc(sizeof(triTexture));
		if (tex==0)
		{
			triLogPrint("ERROR: Could not allocate texture struct!\n");
			triImageFree( img );
			return(-1);
		}
		triTextureSafeConstructor( tex );
		tex->id = triTexman.id_counter++;
		tex->priority = 0;
		tex->refcount = 1;
		tex->allocated = 1;
		triTextureListInsert( tex );
		triTextureImage2( tex->id, 0, img );
		
		if (triRefcountRelease( img )==0)
			triFree( img );
		
		triRefcountCreate( tname, tex );
	}

	return tex->id;
}

triS32 triTextureLoadSream( stream* s )
{
	triChar tname[512];
	snprintf( tname, 512, "TEX::stream%i",(int)triTexman.id_counter + 1 );
	
	triTexture* tex = (triTexture*)triRefcountRetain( tname );

	if (tex==0)
	{
		triImage* img = triImageLoadStream( s, 0 );
		if (img==0)
		{
			triLogPrint("ERROR: Could not load image!\n");
			return(-1);
		}
		
		tex = triMalloc(sizeof(triTexture));
		if (tex==0)
		{
			triLogPrint("ERROR: Could not allocate texture struct!\n");
			triImageFree( img );
			return(-1);
		}
		triTextureSafeConstructor( tex );
		tex->id = triTexman.id_counter++;
		tex->priority = 0;
		tex->refcount = 1;
		tex->allocated = 1;
		triTextureListInsert( tex );
		triTextureImage2( tex->id, 0, img );
		
		if (triRefcountRelease( img )==0)
			triFree( img );
		
		triRefcountCreate( tname, tex );
	}

	return tex->id;
}

triS32 triTextureUnload( triS32 id )
{
	triTexture* tex = triTextureListFind( id );
	if (tex==0) return(-1);

	if (triRefcountRelease( tex )==0)
		triTextureRel( 1, &id );

	return(0);
}


triImage* triTextureGet( triS32 id )
{
	triTexture* tex = triTextureListFind( id );
	if (tex==0) return(0);
	
	void* data = triMalloc(tex->size);
	memcpy( data, tex->vram?tex->vram:tex->data, tex->size );
	
	triImage* img = triImageSet( tex->width, tex->height, tex->width, tex->bytesperline/tex->width<<3, tex->format, data );
	if (img==0) return(0);
	
	void* pal = 0;
	if (tex->paldata!=0)
	{
		pal = triMalloc( (tex->format==GU_PSM_T4?16:256)*(tex->palformat==GU_PSM_8888?4:2) );
		memcpy( pal, tex->paldata, (tex->format==GU_PSM_T4?16:256)*(tex->palformat==GU_PSM_8888?4:2) );
	}
	img->palette = pal;
	img->palformat = tex->palformat;
	img->size = tex->size;
	img->levels = tex->mipmaps;
	img->swizzled = tex->swizzle;
	
	triMipLevel* lev = 0;
	triMipLevel* last = 0;
	while (tex->next!=0)
	{
		tex = tex->next;
		lev = triMalloc( sizeof(triMipLevel) );
		if (lev==0) break;
		
		lev->next = 0;
		lev->width = tex->width;
		lev->height = tex->height;
		lev->stride = tex->width;
		lev->size = tex->size;
		lev->tex_height = tex->height;
		
		data = triMalloc(tex->size);
		memcpy( data, tex->vram?tex->vram:tex->data, tex->size );

		lev->data = data;
		if (last==0)
			img->level = lev;
		else
			last->next = lev;
		last = lev;
	}
	
	return(img);
}
