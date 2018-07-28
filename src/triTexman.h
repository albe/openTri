/*
 * triTexman.h: Header for Texture manager
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

#ifndef __TRITEXMAN_H__
#define __TRITEXMAN_H__

#include "triTypes.h"
#include "triImage.h"


/** @defgroup triTexman Texture manager
 *  @{
 */

#define TRI_TEXMAN_PRIORITIES 8 /**< How many different priorities the texture manager should handle */

// Texture manager, keeping textures in VRAM on MFU (most frequently used) base

typedef struct triTexture
{
	triS32		id;

	triVoid		*vram;					// pointer to vram if uploaded
	triVoid		*data;					// pointer to sysram copy

	triVoid		*paldata;
	triS32		palformat;

	triS32		size;
	triS32		format;
	triS32		width;
	triS32		height;
	triS32		bytesperline;
	triS32		swizzle;
	triS32		level;
	triS32		allocated;				// texture data was allocated by texture manager (mipmaps only)
	
	triS32		refcount;				// counter for number of allocations
	triS32		priority;				// priority to stay in VRAM
	
	triS32		mipmaps;				// number of mipmaps below this level
	struct triTexture* next;			// mipmap linkage
} triTexture;


triS32 triTextureGen( triS32 n, triS32 *id );
triS32 triTexturePrioritize( triS32 n, triS32 *id, triS32 *priorities );
triS32 triTextureSize( triS32 width, triS32 height, triS32 format );
triS32 triTextureUpload( triS32 id, triS32 priority );	// force upload and set priority
triS32 triTextureImage( triS32 id, triS32 level, triS32 width, triS32 height, triS32 format, triS32 swizzled, triVoid* data, triS32 palformat, triVoid* paldata );
triS32 triTextureImage2( triS32 id, triS32 level, triImage* img );
triS32 triTextureIslocal( triS32 id );
triS32 triTextureBind( triS32 id );
triS32 triTextureRel( triS32 n, triS32 *id );
triS32 triTextureBuildMipmaps( triS32 id, triS32 levels );

triS32 triTextureLoad( triChar* filename );
triS32 triTextureLoadSream( stream* s );
triS32 triTextureUnload( triS32 id );
triImage* triTextureGet( triS32 id );
/** @} */

#endif  // __TRITEXMAN_H__
