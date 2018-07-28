/*
 * triTypes.h: Header for common datatypes throughout the triEngine
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 Alexander Berl 'Raphael' <raphael@fx-world.org>
 * Copyright (C) 2007 David Perry 'InsertWittyName' <tias_dp@hotmail.com>
 * Copyright (C) 2007 Tomas Jakobsson 'Tomaz'
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


#ifndef __TRITYPES_H__
#define __TRITYPES_H__

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif // DEBUG
#endif // _DEBUG
#ifdef DEBUG
#ifndef _DEBUG
#define _DEBUG
#endif // _DEBUG
#endif // DEBUG

#include <pspgu.h>

#define TRUE 1
#define FALSE 0

#define ALIGN16 __attribute__((aligned(16)))

typedef void triVoid;
typedef char triChar;
typedef unsigned char triUChar;
typedef signed int triSInt;	// sizeof(int) is platform specific, so we might want to have that for optimal code parts
typedef unsigned int triUInt;

typedef signed long long triS64;
typedef unsigned long long triU64;
typedef signed long triS32;
typedef unsigned long triU32;
typedef signed short triS16;
typedef unsigned short triU16;
typedef signed char triS8;
typedef unsigned char triU8;
typedef unsigned char triBool;
typedef float triFloat;

// For now triDouble isn't quaranteed to be 64bit, but is platform dependant -> PSP has no 64bit FPU, so we avoid that
#ifndef _PSP
typedef double triDouble;
#else
typedef float triDouble;		// FIXME: portability issue -> double should be expected to be 64bit
#endif

typedef float triF32;		// Workaround: Define size guaranted floats
typedef double triF64;


/** @defgroup triVec2 2D Vectors
 *  @{
 */
 
/**
  * 2D float Vector
  */
typedef struct triVec2
{
	triFloat	x, y;
} triVec2, triVec2f; // Untyped vectors are always supposed to be float types


/**
  * 2D signed int Vector
  */
typedef struct triVec2S32
{
	triS32		x, y;	// Sizes should be the same as triVec*f, hence S32 instead of platform specific SInt
} triVec2S32, triVec2i;


/**
  * 2D unsigned int Vector
  */
typedef struct triVec2U32
{
	triU32		x, y;
} triVec2U32;


/**
  * 2D signed short Vector
  */
typedef struct triVec2S16
{
	triS16		x, y;
} triVec2S16;


/**
  * 2D unsigned short Vector
  */
typedef struct triVec2U16
{
	triU16		x, y;
} triVec2U16;



/**
  * 2D signed char Vector
  */
typedef struct triVec2S8
{
	triS8		x, y;
} triVec2S8;


/**
  * 2D unsigned char Vector
  */
typedef struct triVec2U8
{
	triU8		x, y;
} triVec2U8;


/** @} */  // End triVec2


/** @defgroup triVec3 3D Vectors
 *  @{
 */
 
/**
  * 3D float Vector
  */
typedef struct triVec3
{
	triFloat	x, y, z;
} triVec3, triVec3f;


/**
  * 3D signed int Vector
  */
typedef struct triVec3S32
{
	triS32		x, y, z;
} triVec3S32, triVec3i;


/**
  * 3D unsigned int Vector
  */
typedef struct triVec3U32
{
	triU32		x, y, z;
} triVec3U32;


/**
  * 3D signed short Vector
  */
typedef struct triVec3S16
{
	triS16		x, y, z;
} triVec3S16;


/**
  * 3D unsigned short Vector
  */
typedef struct triVec3U16
{
	triU16		x, y, z;
} triVec3U16;



/**
  * 3D signed char Vector
  */
typedef struct triVec3S8
{
	triS8		x, y, z;
} triVec3S8;


/**
  * 3D unsigned char Vector
  */
typedef struct triVec3U8
{
	triU8		x, y, z;
} triVec3U8;


/** @} */  // End triVec3


/** @defgroup triVec4 4D Vectors
 *  @{
 */


/**
  * 4D float Vector (quaternion)
  */
typedef struct triVec4
{
	triFloat	x, y, z, w;
} triVec4 ALIGN16, triVec4f ALIGN16, triQuat ALIGN16;


/**
  * 4D signed int Vector (quaternion)
  */
typedef struct triVec4S32
{
	triS32		x, y, z, w;
} triVec4S32, triVec4i ALIGN16, triQuati ALIGN16;


/**
  * 4D unsigned int Vector
  */
typedef struct triVec4U32
{
	triU32		x, y, z, w;
} triVec4U32;



/**
  * 4D signed short Vector
  */
typedef struct triVec4S16
{
	triS16		x, y, z, w;
} triVec4S16;


/**
  * 4D unsigned short Vector
  */
typedef struct triVec4U16
{
	triU16		x, y, z, w;
} triVec4U16;



/**
  * 4D signed char Vector
  */
typedef struct triVec4S8
{
	triS8		x, y, z, w;
} triVec4S8;


/**
  * 4D unsigned char Vector
  */
typedef struct triVec4U8
{
	triU8		x, y, z, w;
} triVec4U8;



/** @} */  // End triVec4


/** @defgroup triColor Colors
 *  @{
 */

/**
  * RGB float color (96bit)
  */
typedef struct triColor3
{
	triFloat	r, g, b;
} triColor3, triColor3f;


/**
  * RGB int color (96bit)
  */
typedef struct triColor3i
{
	triS32		r, g, b;
} triColor3i;


/**
  * RGBA float color (128bit)
  */
typedef struct triColor4
{
	triFloat	r, g, b, a;
} triColor4 ALIGN16, triColor4f ALIGN16;


/**
  * RGBA int color (128bit)
  */
typedef struct triColor4i
{
	triS32		r, g, b, a;
} triColor4i ALIGN16;



/**
  * RGBA8880 color (24bit)
  */
typedef struct triColor8880
{
	triU8		r, g, b;
} triColor3b, triColor8880;



/**
  * RGBA8888 color (32bit)
  */
typedef union triColor8888
{
	struct
	{
		triU8		r, g, b, a;
	};
	triU32			color;		// For fast r,g,b,a -> color conversion
} triColor4b, triColor8888;



/**
  * RGBA4444 color (16bit)
  */
typedef union triColor4444
{
	struct
	{
		triU16		r:4, g:4, b:4, a:4;
	};
	triU16			color;		// For fast r,g,b,a -> color conversion
} triColor4444;



/**
  * RGBA5551 color (16bit)
  */
typedef union triColor5551
{
	struct
	{
		triU16		r:5, g:5, b:5, a:1;
	};
	triU16			color;		// For fast r,g,b,a -> color conversion
} triColor5551;



/**
  * RGBA5650 color (16bit)
  */
typedef union triColor5650
{
	struct
	{
		triU16		r:5, g:6, b:5;
	};
	triU16			color;		// For fast r,g,b,a -> color conversion
} triColor5650;


/** @} */  // End triColor



/** @defgroup triMat Matrices
 *  @{
 */

/**
  * 3D (3x3) Matrix
  */
typedef union triMat3
{
	struct
	{
		triVec3f	x, y, z;
	};
	triFloat		m[9];
	triFloat		md[3][3];
} triMat3, triMat3f;



/**
  * 4D (4x4) Matrix
  */
typedef union triMat4
{
	struct
	{
		triVec4f	x, y, z, w;
	};
	triFloat		m[16];
	triFloat		md[4][4];
} triMat4 ALIGN16, triMat4f ALIGN16;


/** @} */  // End triMat




/** @defgroup triVert Vertices
 *  @{
 */


#define TRI_VERTC_FORMAT (GU_COLOR_8888|GU_VERTEX_32BITF)
/**
  * Vertex with color
  */
typedef struct triVertC
{
	triU32		color;
	triFloat	x, y, z;
} triVertC, triVertCf;	//	16 bytes


#define TRI_VERTCN_FORMAT (GU_NORMAL_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF)
/**
  * Vertex with color and normale
  */
typedef struct triVertCN
{
	triU32		color;
	triFloat	nx, ny, nz;	
	triFloat	x, y, z;
} triVertCN, triVertCNf;	// 28 bytes


#define TRI_VERTUV_FORMAT (GU_TEXTURE_32BITF|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates
  */
typedef struct triVertUV
{
	triFloat	u, v;
	triFloat	x, y, z;
} triVertUV, triVertUVf;	// 20 bytes


#define TRI_VERTUVC_FORMAT (GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates and color
  */
typedef struct triVertUVC
{
	triFloat	u, v;
	triU32		color;
	triFloat	x, y, z;
} triVertUVC, triVertUVCf;	// 24 bytes



#define TRI_VERTUVN_FORMAT (GU_NORMAL_32BITF|GU_TEXTURE_32BITF|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates and normale
  */
typedef struct triVertUVN
{
	triFloat	u, v;
	triFloat	nx, ny, nz;	
	triFloat	x, y, z;
} triVertUVN, triVertUVNf;	// 32 bytes


#define TRI_VERTUVCN_FORMAT (GU_NORMAL_32BITF|GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates, color and normale
  */
typedef struct triVertUVCN
{
	triFloat	u, v;
	triU32		color;
	triFloat	nx, ny, nz;	
	triFloat	x, y, z;
} triVertUVCN, triVertUVCNf;	// 36 bytes




#define TRI_VERTFASTUV_FORMAT (GU_TEXTURE_8BIT|GU_VERTEX_16BIT)
/**
  * Vertex with texture coordinates and color, optimized
  */
typedef struct triVertFastUV
{
	triU8		u, v;
	triS16		x, y, z;
} triVertFastUV;	// 2 + 6 = 8 bytes


#define TRI_VERTFASTUVF_FORMAT (GU_TEXTURE_8BIT|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates and color, optimized
  */
typedef struct triVertFastUVf
{
	triU16		u, v;
	triFloat	x, y, z;
} triVertFastUVf;	// 4 + 12 = 16 bytes


#define TRI_VERTFASTUVC_FORMAT (GU_TEXTURE_8BIT|GU_COLOR_5650|GU_VERTEX_16BIT)
/**
  * Vertex with texture coordinates and color, optimized
  */
typedef struct triVertFastUVC
{
	triU8		u, v;
	triU16		color;
	triS16		x, y, z;
} triVertFastUVC;	// 2 + 2 + 6 = 10 bytes


#define TRI_VERTFASTUVCF_FORMAT (GU_TEXTURE_8BIT|GU_COLOR_4444|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates and color, optimized
  */
typedef struct triVertFastUVCf
{
	triU8		u, v;
	triU16		color;
	triFloat	x, y, z;
} triVertFastUVCf;	// 2 + 2 + 12 = 16 bytes


#define TRI_VERTFASTUVN_FORMAT (GU_NORMAL_8BIT|GU_TEXTURE_8BIT|GU_VERTEX_16BIT)
/**
  * Vertex with texture coordinates and normale, optimized
  */
typedef struct triVertFastUVN
{
	triU8		u, v;
	triS8		nx, ny, nz;	
	triS16		x, y, z;
} triVertFastUVN;	// 2 + 3 (+ 1) + 6 = 12 bytes


#define TRI_VERTFASTUVNF_FORMAT (GU_NORMAL_16BIT|GU_TEXTURE_8BIT|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates and normale, optimized
  */
typedef struct triVertFastUVNf
{
	triU8		u, v;
	triS16		nx, ny, nz;
	triFloat	x, y, z;
} triVertFastUVNf;	// 2 + 6 + 12 = 20 bytes


#define TRI_VERTFASTUVCN_FORMAT (GU_NORMAL_8BIT|GU_TEXTURE_8BIT|GU_COLOR_5650|GU_VERTEX_16BIT)
/**
  * Vertex with texture coordinates, color and normale, optimized
  */
typedef struct triVertFastUVCN
{
	triU8		u, v;
	triU16		color;
	triS8		nx, ny, nz;
	triS16		x, y, z;
} triVertFastUVCN;	// 2 + 2 + 3 (+ 1) + 6 = 14 bytes


#define TRI_VERTFASTUVCNF_FORMAT (GU_NORMAL_8BIT|GU_TEXTURE_8BIT|GU_COLOR_5650|GU_VERTEX_32BITF)
/**
  * Vertex with texture coordinates, color and normale, optimized
  */
typedef struct triVertFastUVCNf
{
	triU8		u, v;
	triU16		color;
	triS8		nx, ny, nz;
	triFloat	x, y, z;
} triVertFastUVCNf;	// 2 + 2 + 3 (+ 1) + 12 = 20 bytes

/** @} */  // End triVert

#endif // __TRITYPES_H__
