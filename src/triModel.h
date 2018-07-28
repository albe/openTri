/*
 * triModel.h: Header for model loading/saving
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 Tomas Jakobsson 'Tomaz'
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
 
#ifndef __TRIMESH_H__
#define __TRIMESH_H__

#ifdef __PSP__
#include <pspgu.h>
#endif
#include "triTypes.h"
#include "triTexman.h"


/** @defgroup triModel Model manager
 *  @{
 */
 
/* triModel file format:
 *
 * <triModelFileHeader>
 *   n-times:
 *   <triChunkHeader>
 *   <triXHeader>
 *      [data]
 *
 * A chunk header contains the header ID (magic) and the size of the chunk including the following tri*Header and data.
 * This is to make sure all readers can parse the whole file, even if they don't know a specific chunk header.
 *
 * NOTE: The loader currently needs the meshes to be stored at the start of the file.
 *
 * TODO: - Add texture skinning support.
 */
typedef struct triModelFileHeader
{
	triChar		magic[8];		// "triModel"
	triU16		numMeshes;		// number of Meshes contained in File
	triU16		numModels;		// number of Models contained in File
	triU16		numTexs;		// number of Textures contained in File
	triU16		reserved;
} triModelFileHeader;	// 16 bytes


typedef struct triChunkHeader
{
	triChar		magic[4];
	triU32		chunkSize;
} triChunkHeader;	// 8 bytes


/* A Model header: "tMH "
 * followed by numMeshes triPart objects
 */
typedef struct triModelHeader
{
	triChar		name[12];
	triU16		numParts;
	triU16		flags;
	triVec4f	pos;
	triVec4f	rot;
} triModelHeader;	// 48 bytes


typedef struct triPart
{
	triChar		name[12];
	triU16		meshID;		// ID of mesh to connect to (0 = first mesh in file, 1 = second mesh in file, etc.)
	triU16		texID;		// ID of tex to connect to (see above)
	triVec4f	pos;
	triVec4f	rot;
} triPart;	// 48 bytes


#define TRI_MESH_FLAGS_GZIP	1
#define TRI_MESH_FLAGS_SAVE_IMAGE 2	// Only for trim saver. Stores the image data in the .trim
#define TRI_MESH_FLAGS_TRIANGLES 0	// to stay backward compatible
#define TRI_MESH_FLAGS_TRIANGLE_STRIP 4
#define TRI_MESH_FLAGS_TRIANGLE_FAN 5


/* A Mesh header: "tMhH"
 * followed by numVerts vertices, each with size vertSize
 */
typedef struct triMeshHeader
{
	triChar		name[12];
	triU32		vertFormat;
	triU16		numVerts;
	triU16		flags;
	triU16		vertSize;		// Size of one vertex in bytes
	triU16		texID;
	triU32		dataSize;		// Size of following data block in bytes (needed for GZIP compression)
} triMeshHeader;	// 28 bytes


/* A Texture header: "tTH "
 */
typedef struct triTexHeader
{
	triChar		filename[64];	// filename of texture
} triTexHeader;		// 64 bytes


/* An Image header: "tImg"
 * followed by a triTexHeader and a triImage file
 */


/* A Morph weights header: "tMoH"
 * followed by numWeights floats, giving weights for vertex indices 0 - numWeights-1
 */
typedef struct triMorphHeader
{
	triU8		numWeights;
	triU8		reserved1;
	triU16		reserved2;
} triMorphHeader;	// 4 bytes


/* A Skinning bone header: "tBH "
 * followed by numBones 4x4 float matrices, giving skinning matrices for weight indices 0 - numBones-1
 */
typedef struct triBoneHeader
{
	triU8		numBones;
	triU8		reserved1;
	triU16		reserved2;
} triBoneHeader;	// 4 bytes





typedef struct triMesh
{
	void*		verts;
	triImage*	texture;		// Mesh texture (only needed for the trim saver any more)
	triS32		texID;
	triU32		numVerts;
	triU32		vertFormat;		// Vertex format
	triU32		renderFormat;	// Render format (GU_TRIANGLES, GU_TRIANGLE_STRIP, GU_TRIANGLE_FAN)
	
	triS32		boundingCheck;	// flag which bounding checks to do on the mesh (0 = none, 1 = boundingBox, 2 = boundingSphere)
	triVec3f	boundingBox[8];
	triVec4f	boundingSphere;	// x,y,z,radius
	struct triMesh*	next;
} triMesh;

typedef struct triModelPart
{
	triVec4f	pos;			// Vec4fs need be aligned on 16bytes, so keep them at start of struct
	triVec4f	rot;
	triMesh*	mesh;			// mesh

	triS32		texID;			// Allow different parts with same meshes to have different textures
	
	triS32		reserved[2];
} triModelPart;

typedef struct triModel
{
	triVec4f		pos;
	triVec4f		rot;
	triModelPart*	parts;		// parts
	triU32			numParts;
	
	triS32			boundingCheck;	// flag which bounding checks to do on the mesh (0 = none, 1 = boundingBox, 2 = boundingSphere)
	triVec3f		boundingBox[8];
	triVec4f		boundingSphere;	// x,y,z,radius
	
	struct triModel*	next;
} triModel;

typedef struct triTex
{
	triImage*		tex;
	triS32			texID;
	
	struct triTex*	next;
} triTex;


typedef struct triModelManager
{
	triMesh*		lastMesh;
	triMesh*		meshes;
	triS32			numMeshes;
	
	triTex*			lastTex;
	triTex*			texs;
	triS32			numTexs;
	
	triModel*		lastModel;
	triModel*		models;
	triS32			numModels;
} triModelManager;



void triModelManagerInit();
void triModelManagerFreeAll();


triModel* triModelsLoadTrim( triChar* filename, triS32* numModels );
#ifdef TRI_SUPPORT_SAVE_TRIM
void triModelsSaveTrim( triChar* filename, triModel* models, triS32 numModels, triS32 flags );
#endif
void triModelsFree( triModel* models, triS32 numModels );

void triModelRender( triModel* model );
void triMeshCalcBoundings( triMesh* mesh );

void triModelOptimize( triModel* model, triS32 format );
void triMeshOptimize( triMesh* mesh, triS32 format );


extern triMesh*	triMeshLoadTrim	( triChar* fileName, triChar* texName );
extern void		triMeshFree		( triMesh* pMesh );

/** @} */

#endif // __TRIMESH_H__
