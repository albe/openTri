/*
 * triModel.c: Code for model loading/saving
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
 
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <pspgum.h>
#include "triTypes.h"
#include "triMemory.h"
#include "triLog.h"
#include "triModel.h"
#include "triVMath_vfpu.h"


triModelManager	triMM = {0};


void triModelManagerInit()
{
	triMM.lastMesh = 0;
	triMM.meshes = 0;
	triMM.numMeshes = 0;
	
	triMM.lastTex = 0;
	triMM.texs = 0;
	triMM.numTexs = 0;
	
	triMM.lastModel = 0;
	triMM.models = 0;
	triMM.numModels = 0;
}

void triModelManagerFreeAll()
{
	while (triMM.texs!=0)
	{
		triTex* next = triMM.texs->next;
		triImageFree( triMM.texs->tex );
		triTextureRel( 1, &triMM.texs->texID );
		triFree( triMM.texs );
		triMM.texs = next;
	}
	
	while (triMM.meshes!=0)
	{
		triMesh* next = triMM.meshes->next;
		triFree( triMM.meshes->verts );
		triFree( triMM.meshes );
		triMM.meshes = next;
	}
	
	while (triMM.models!=0)
	{
		triModel* next = triMM.models->next;
		triFree( triMM.models->parts );
		triFree( triMM.models );
		triMM.models = next;
	}
	
	triModelManagerInit();
}


void triModelManagerAddMesh( triMesh* mesh )
{
	if (triMM.lastMesh==0)
	{
		triMM.meshes = mesh;
		triMM.lastMesh = mesh;
		triMM.numMeshes = 1;
	}
	else
	{
		triMM.lastMesh->next = mesh;
		triMM.lastMesh = mesh;
		triMM.numMeshes++;
	}
}

void triModelManagerAddTex( triImage* tex, triS32 texID )
{
	triTex*	tt = triMalloc(sizeof(triTex));
	tt->tex = tex;
	tt->texID = texID;
	tt->next = 0;
	if (triMM.lastTex==0)
	{
		triMM.texs = tt;
		triMM.lastTex = tt;
		triMM.numTexs = 1;
	}
	else
	{
		triMM.lastTex->next = tt;
		triMM.lastTex = tt;
		triMM.numTexs++;
	}
}

void triModelManagerAddModel( triModel* model )
{
	if (triMM.lastModel==0)
	{
		triMM.models = model;
		triMM.lastModel = model;
		triMM.numModels = 1;
	}
	else
	{
		triMM.lastModel->next = model;
		triMM.lastModel = model;
		triMM.numModels++;
	}
}




static void _triLoadModel( triModel* model, triMesh* meshes, stream* s )
{
	triModelHeader tmh;
	stream_read( s, &tmh, sizeof(tmh) );
	#ifdef DEBUG
	triLogPrint("triModelsLoadTrim: triModelHeader>\n");
	triLogPrint("->name: %s\n", tmh.name );
	triLogPrint("->numParts: %i\n", tmh.numParts );
	triLogPrint("->flags: %i\n", tmh.flags );
	#endif
	if (stream_eos(s)) return;
	
	model->numParts = tmh.numParts;
	model->pos = tmh.pos;
	model->rot = tmh.rot;
	model->boundingCheck = 0;
	model->parts = triMalloc( tmh.numParts * sizeof(triModelPart) );
	if (model->parts==0)
	{
		triLogError("_triLoadMesh: Error allocating parts memory!\n");
		return;
	}
	
	while (!stream_eos(s) && tmh.numParts>0)
	{
		triPart tmp;
		stream_read( s, &tmp, sizeof(tmp) );
		#ifdef DEBUG
		triLogPrint("triModelsLoadTrim: triPart>\n");
		triLogPrint("->name: %s\n", tmp.name );
		triLogPrint("->meshID: %i\n", tmp.meshID );
		triLogPrint("->texID: %i\n", tmp.texID );
		#endif
		triS32 i = model->numParts-tmh.numParts;
		triModelPart* p = &model->parts[i];
		triMesh* mesh = meshes;
		while (tmp.meshID>0 && mesh!=0)
		{
			mesh = mesh->next;
			tmp.meshID--;
		}
		p->mesh = mesh;
		p->pos = tmp.pos;
		p->rot = tmp.rot;
		p->texID = tmp.texID;
		tmh.numParts--;
	}
	
	triModelManagerAddModel( model );
}


static void _triLoadMesh( triMesh* mesh, triS32* texs, triImage** imgs, stream* s )
{
	triMeshHeader tmh;
	stream_read( s, &tmh, sizeof(tmh) );
	
	#ifdef DEBUG
	triLogPrint("triModelsLoadTrim: triMeshHeader>\n");
	triLogPrint("->name: %s\n", tmh.name );
	triLogPrint("->vertFormat: %i\n", tmh.vertFormat );
	triLogPrint("->numVerts: %i\n", tmh.numVerts );
	triLogPrint("->flags: %i\n", tmh.flags );
	triLogPrint("->vertSize: %i\n", tmh.vertSize );
	triLogPrint("->texID: %i\n", tmh.texID );
	triLogPrint("->dataSize: %i\n", tmh.dataSize );
	#endif
	mesh->numVerts = tmh.numVerts;
	mesh->vertFormat = tmh.vertFormat;
	mesh->renderFormat = (tmh.flags >> 2) + 3;
	mesh->texture = imgs[tmh.texID];
	mesh->texID = texs[tmh.texID];
	mesh->boundingCheck = 0;
	mesh->verts = triMalloc( tmh.vertSize * tmh.numVerts );
	if (mesh->verts==0)
	{
		triLogError("_triLoadMesh: Error allocating vertex memory!\n");
		return;
	}
	
	if (tmh.flags&TRI_MESH_FLAGS_GZIP)
	{
		triU8* tdata = triMalloc( tmh.dataSize );
		stream_read( s, tdata, tmh.dataSize );
		triU32 tsize = tmh.vertSize*tmh.numVerts;
		if (uncompress( mesh->verts, &tsize, tdata, tmh.dataSize )!=Z_OK)
		{
				triLogError("_triLoadMesh: Error uncompressing data!\n");
				triFree( tdata );
				triFree( mesh->verts );
				mesh->verts = 0;
				return;
		}
		triFree( tdata );
	}
	else
		stream_read( s, mesh->verts, tmh.vertSize*tmh.numVerts );
	
	triModelManagerAddMesh( mesh );
}


static void _triLoadTexture( triS32 tex, triImage** img, stream* s )
{
	triTexHeader tth;
	stream_read( s, &tth, sizeof(tth) );
	
	#ifdef DEBUG
	triLogPrint("triModelsLoadTrim: %s\n", tth.filename );
	triLogPrint("texID: %i\n", tex);
	#endif
	*img = triImageLoad( tth.filename, 0 );
	triTextureImage2( tex, 0, *img );
	
	triModelManagerAddTex( *img, tex );
}


static void _triLoadImage( triS32 tex, triImage** img, triU32 size, stream* s )
{
	triTexHeader tth;
	stream_read( s, &tth, sizeof(tth) );
	
	#ifdef DEBUG
	triLogPrint("triModelsLoadTrim: %s\n", tth.filename );
	triLogPrint("imgsize: %i\n", size-sizeof(tth));
	triLogPrint("texID: %i\n", tex);
	#endif
	
	void* data = triMalloc( size-sizeof(tth) );
	stream_read( s, data, size-sizeof(tth) );
	
	stream* m = stream_mopen( data, size );
	
	*img = triImageLoadStream( m, 0 );
	if (*img != 0)
	{
		snprintf( (*img)->filename, 64, tth.filename );
		triTextureImage2( tex, 0, *img );
		triModelManagerAddTex( *img, tex );
	}
	stream_close( m );
	triFree( data );
}


#define FOURCC(a,b,c,d) ((triU32)(((triU32)a) | ((triU32)b << 8) | ((triU32)c << 16) | ((triU32)d << 24)))

triModel* triModelsLoadTrim( triChar* filename, triS32* numModels )
{
	stream* s = stream_bufopen( STREAM_TYPE_FILE, filename, STREAM_RDONLY );
	//FILE* fp = fopen(filename, "rb");
	
	if (s==0) return(0);
	
	triModelFileHeader mfh;
	
	stream_read( s, &mfh, sizeof(mfh) );
	
	#ifdef DEBUG
	triLogPrint("triModelsLoadTrim: triModelFileHeader>\n");
	triLogPrint("->magic: %c%c%c%c%c%c%c%c\n", mfh.magic[0], mfh.magic[1], mfh.magic[2], mfh.magic[3], mfh.magic[4], mfh.magic[5], mfh.magic[6], mfh.magic[7] );
	triLogPrint("->numMeshes: %i\n", mfh.numMeshes );
	triLogPrint("->numModels: %i\n", mfh.numModels );
	triLogPrint("->numTexs: %i\n", mfh.numTexs );
	#endif
	
	*numModels = 0;
	if (strncmp(mfh.magic,"triModel",8)!=0)
	{
		stream_close( s );
		return(0);
	}
	
	triModel*	modellist = 0, *models = 0;
	triMesh*	meshlist = 0, *meshes = 0;

	triS32*		texs = triMalloc( sizeof(triS32)*mfh.numTexs );
	if (texs==0)
	{
		stream_close( s );
		return(0);
	}
	triTextureGen( mfh.numTexs, texs );
	
	triS32 i;
	for (i=0;i<mfh.numTexs;i++)
		triLogPrint("Generated texture ID: %i\n", texs[i]);
	
	triImage**	imgs = triMalloc( sizeof(triImage*)*mfh.numTexs );
	if (imgs==0)
	{
		triFree( texs );
		stream_close( s );
		return(0);
	}
	memset( imgs, 0, sizeof(triImage*)*mfh.numTexs );
	
	
	triS32	texcount = 0;
	
	*numModels = mfh.numModels;
	
	triChunkHeader tch;
	
	triS32 eof = 0;
	while (!eof && !stream_eos(s))
	{
		stream_read( s, &tch, sizeof(tch) );
		#ifdef DEBUG
		triLogPrint("triModelsLoadTrim: triChunkHeader>\n");
		triLogPrint("->magic: %c%c%c%c\n", tch.magic[0], tch.magic[1], tch.magic[2], tch.magic[3] );
		triLogPrint("->chunkSize: %i\n", tch.chunkSize );
		#endif
		switch (*(triU32*)tch.magic)
		{
			case FOURCC('t','M','h','H'):
				{
				triU32 pos = stream_tell( s );
				triMesh* mesh = triMalloc( sizeof(triMesh) );
				_triLoadMesh( mesh, texs, imgs, s );
				mesh->next = 0;
				if (meshlist!=0)
					meshlist->next = mesh;
				else
					meshes = mesh;
				meshlist = mesh;
				stream_seek( s, pos+tch.chunkSize, STREAM_SET );
				break;
				}
			case FOURCC('t','M','H',' '):
				{
				triU32 pos = stream_tell( s );
				triModel* model = triMalloc( sizeof(triModel) );
				_triLoadModel( model, meshes, s );
				model->next = 0;
				if (modellist!=0)
					modellist->next = model;
				else
					models = model;
				modellist = model;
				stream_seek( s, pos+tch.chunkSize, STREAM_SET );
				break;
				}
			case FOURCC('t','T','H',' '):
				{
				triU32 pos = stream_tell( s );
				_triLoadTexture( texs[texcount], &imgs[texcount], s );
				texcount++;
				stream_seek( s, pos+tch.chunkSize, STREAM_SET );
				break;
				}
			case FOURCC('t','I','m','g'):
				{
				triU32 pos = stream_tell( s );
				_triLoadImage( texs[texcount], &imgs[texcount], tch.chunkSize, s );
				texcount++;
				stream_seek( s, pos+tch.chunkSize, STREAM_SET );
				break;
				}
			case FOURCC('t','M','o','H'):
				stream_seek( s, tch.chunkSize, STREAM_CUR );
				break;
			
			case FOURCC('t','B','H',' '):
				stream_seek( s, tch.chunkSize, STREAM_CUR );
				break;
			
			case FOURCC('t','E','O','F'):
				eof = 1;
				break;
			
			default:
				stream_seek( s, tch.chunkSize, STREAM_CUR );
				break;
		}
	}
	
	stream_close( s );
	
	triFree( imgs );
	triFree( texs );
	return(models);
}


void triMeshCalcBoundings( triMesh* mesh )
{

}



#define TRI_PI				3.1415926535897932384626433832795028841971693993751058209749445923f
#define TRI_RAD_TO_DEG(X)	((X) * (180.0f / TRI_PI))
#define TRI_DEG_TO_RAD(X)	((X) * (TRI_PI / 180.0f))

void triModelRender( triModel* model )
{
	triS32 i;
	triVec3 Rot;
	
	sceGumPushMatrix();

	triVec3Set(&Rot, TRI_DEG_TO_RAD (model->rot.x), TRI_DEG_TO_RAD (model->rot.y), TRI_DEG_TO_RAD (model->rot.z));

	sceGumTranslate((ScePspFVector3*)&model->pos);
	sceGumRotateXYZ((ScePspFVector3*)&Rot);

	if (model->boundingCheck==1)
	{
		sceGumUpdateMatrix();
		sceGuBeginObject( GU_VERTEX_32BITF, 8, 0, model->boundingBox );
	}
	for (i=0; i<model->numParts; i++)
	{
		sceGumPushMatrix ();

		triVec3Set(&Rot, TRI_DEG_TO_RAD (model->parts[i].rot.x), TRI_DEG_TO_RAD (model->parts[i].rot.y), TRI_DEG_TO_RAD (model->parts[i].rot.z));

		sceGumTranslate((ScePspFVector3*)&model->parts[i].pos);
		sceGumRotateZYX((ScePspFVector3*)&Rot);

		if (model->parts[i].mesh->boundingCheck==1)
		{
			sceGumUpdateMatrix();
			sceGuBeginObject( GU_VERTEX_32BITF, 8, 0, model->parts[i].mesh->boundingBox );
		}
		
		triTextureBind( model->parts[i].mesh->texID );
		sceGumDrawArray(model->parts[i].mesh->renderFormat, model->parts[i].mesh->vertFormat | GU_TRANSFORM_3D, model->parts[i].mesh->numVerts, 0, model->parts[i].mesh->verts);

		if (model->parts[i].mesh->boundingCheck==1)
			sceGuEndObject();
		
		sceGumPopMatrix();
	}
	if (model->boundingCheck==1)
		sceGuEndObject();
	sceGumPopMatrix();
}



#ifdef TRI_SUPPORT_SAVE_TRIM

struct refcount {
	void*	ptr;
	struct refcount*	next;
};

static triBool _triCounted( struct refcount** Counted, void* p )
{
	if (p==0) return 1;
	
	if (*Counted==0)
	{
		*Counted = triMalloc( sizeof(struct refcount) );
		if (*Counted==0) return 1;
		(*Counted)->ptr = p;
		(*Counted)->next = 0;
		return 0;
	}
	
	struct refcount* refs = *Counted;
	while (refs!=0)
	{
		if (refs->ptr==p)
			return 1;
		if (refs->next==0) break;
		refs = refs->next;
	}
	
	refs->next = triMalloc( sizeof(struct refcount) );
	refs->next->ptr = p;
	refs->next->next = 0;
	return 0;
}


static triS32 _triGetCount( struct refcount** Counted, void* p )
{
	triS32 id = 0;
	struct refcount* refs = *Counted;
	while (refs!=0)
	{
		if (refs->ptr==p) return(id);
		id++;
		refs = refs->next;
	}
	return(-1);
}


static void _triFreeCounts( struct refcount** Counted )
{
	struct refcount* refs = *Counted;
	while (refs!=0)
	{
		struct refcount* next = refs->next;
		triFree( refs );
		refs = next;
	}
	*Counted = 0;
}
#endif

static triS32 _triVertSize( triS32 format )
{
	switch (format)
	{
		case TRI_VERTUVCN_FORMAT:
			return sizeof(triVertUVCN);
		case TRI_VERTUVN_FORMAT:
			return sizeof(triVertUVN);
		case TRI_VERTUVC_FORMAT:
			return sizeof(triVertUVC);
		case TRI_VERTUV_FORMAT:
			return sizeof(triVertUV);
		case TRI_VERTCN_FORMAT:
			return sizeof(triVertCN);
		case TRI_VERTC_FORMAT:
			return sizeof(triVertC);
			
		case TRI_VERTFASTUVCN_FORMAT:
			return sizeof(triVertFastUVCN);
		case TRI_VERTFASTUVN_FORMAT:
			return sizeof(triVertFastUVN);
		case TRI_VERTFASTUVC_FORMAT:
			return sizeof(triVertFastUVC);
		case TRI_VERTFASTUV_FORMAT:
			return sizeof(triVertFastUV);
		
		case TRI_VERTFASTUVCNF_FORMAT:
			return sizeof(triVertFastUVCNf);
		case TRI_VERTFASTUVNF_FORMAT:
			return sizeof(triVertFastUVNf);
		case TRI_VERTFASTUVCF_FORMAT:
			return sizeof(triVertFastUVCf);
		case TRI_VERTFASTUVF_FORMAT:
			return sizeof(triVertFastUVf);
	}
	return(0);
}

#ifdef TRI_SUPPORT_SAVE_TRIM
void triModelsSaveTrim( triChar* filename, triModel* models, triS32 numModels, triS32 flags )
{
	if (models==0) return;
	FILE* fp = fopen(filename, "wb");
	
	if (fp==0) return;
	
	triModelFileHeader mfh;
	memcpy( mfh.magic, "triModel", 8 );
	mfh.numModels = numModels;
	
	
	triS32 meshcount = 0;
	triS32 texcount = 0;
	
	triSInt i = 0, j = 0;
	struct refcount* texCounted = 0;
	struct refcount* meshCounted = 0;
	for (;i<numModels;i++)
	{
		if (models[i].parts==0) continue;
		for (j=0;j<models[i].numParts;j++)
		{
			if (models[i].parts[j].mesh==0) continue;
			if (!_triCounted( &meshCounted, models[i].parts[j].mesh )) meshcount++;
			if (!_triCounted( &texCounted, models[i].parts[j].mesh->texture )) texcount++;
		}
	}
	#ifdef DEBUG
	triLogPrint("triModelsSaveTrim: meshcount = %i, texcount = %i\n", meshcount, texcount);
	#endif
	
	mfh.numMeshes = meshcount;
	mfh.numTexs = texcount;
	mfh.reserved = 0;
	fwrite( &mfh, 1, sizeof(mfh), fp );
	triChunkHeader tch;
	struct refcount* refs = texCounted;
	
	if (flags&TRI_MESH_FLAGS_SAVE_IMAGE)
	{
		for (i=0;i<texcount;i++)
		{
			triTexHeader tth;
			memcpy( tch.magic, "tImg", 4 );
			stream* s = stream_fopen( ((triImage*)refs->ptr)->filename, STREAM_RDONLY );
			if (s==0) continue;
			long size = stream_size( s );
			void* data = triMalloc( size );
			if (data==0) continue;
			
			tch.chunkSize = size + sizeof(triTexHeader);
			fwrite( &tch, 1, sizeof(tch), fp );
			
			memset( tth.filename, 0, 64 );
			snprintf( tth.filename, 64, ((triImage*)refs->ptr)->filename );
			#ifdef DEBUG
			triLogPrint("triModelsSaveTrim: %s\n", tth.filename);
			#endif
			fwrite( &tth, 1, sizeof(tth), fp );
			
			stream_read( s, data, size );
			stream_close( s );
			fwrite( data, 1, size, fp );
			triFree( data );
			refs = refs->next;
		}
	}
	else
	{
		for (i=0;i<texcount;i++)
		{
			triTexHeader tth;
			memcpy( tch.magic, "tTH ", 4 );
			tch.chunkSize = sizeof(triTexHeader);
			fwrite( &tch, 1, sizeof(tch), fp );
			
			memset( tth.filename, 0, 64 );
			snprintf( tth.filename, 64, ((triImage*)refs->ptr)->filename );
			#ifdef DEBUG
			triLogPrint("triModelsSaveTrim: %s\n", tth.filename);
			#endif
			fwrite( &tth, 1, sizeof(tth), fp );
			refs = refs->next;
		}
	}
	
	refs = meshCounted;
	for (i=0;i<meshcount;i++)
	{
		memcpy( tch.magic, "tMhH", 4 );
		tch.chunkSize = sizeof(triMeshHeader);
		
		triMeshHeader tmh;
		triMesh* m = (triMesh*)refs->ptr;
		void* tdata = m->verts;
		tmh.numVerts = m->numVerts;
		tmh.vertFormat = m->vertFormat;
		tmh.flags = flags | ((m->renderFormat-3) << 2);		// map GU_* modes to TRI_MESH_FLAGS_* modes
		tmh.vertSize = _triVertSize( m->vertFormat );
		tmh.texID = _triGetCount( &texCounted, m->texture );
		tmh.dataSize = tmh.vertSize * tmh.numVerts;
		
		if (flags&TRI_MESH_FLAGS_GZIP)
		{
			tdata = triMalloc( tmh.dataSize );
			if (tdata==0 || compress2( tdata, &tmh.dataSize, m->verts, tmh.dataSize, 9 )!=Z_OK)
			{
				tmh.flags = 0;
				tdata = m->verts;
				tmh.dataSize = tmh.vertSize * tmh.numVerts;
			}
		}
		#ifdef DEBUG
		triLogPrint("triModelsSaveTrim: Mesh %i\n  vertSize: %i\n  dataSize: %i\n", i, tmh.vertSize, tmh.dataSize);
		#endif
		tch.chunkSize += tmh.dataSize;
		fwrite( &tch, 1, sizeof(tch), fp );

		memset( tmh.name, 0, 12 );
		snprintf( tmh.name, 12, "Mesh%i", i );
		fwrite( &tmh, 1, sizeof(tmh), fp );
		fwrite( tdata, 1, tmh.dataSize, fp );
		
		if (tmh.flags&TRI_MESH_FLAGS_GZIP)
			triFree( tdata );
		refs = refs->next;
	}
	
	for (i=0;i<numModels;i++)
	{
		memcpy( tch.magic, "tMH ", 4 );
		tch.chunkSize = sizeof(triModelHeader) + models[i].numParts*sizeof(triPart);
		fwrite( &tch, 1, sizeof(tch), fp );
		
		triModelHeader tmh;
		memset( tmh.name, 0, 12 );
		snprintf( tmh.name, 12, "Model%i", i );
		tmh.numParts = models[i].numParts;
		tmh.flags = 0;
		tmh.pos = models[i].pos;
		tmh.rot = models[i].rot;
		fwrite( &tmh, 1, sizeof(tmh), fp );
		
		for (j=0;j<tmh.numParts;j++)
		{
			triPart tp;
			memset( tp.name, 0, 12 );
			snprintf( tp.name, 12, "Part%i", j );
			tp.meshID = _triGetCount( &meshCounted, models[i].parts[j].mesh );
			tp.texID = _triGetCount( &texCounted, models[i].parts[j].mesh->texture );
			tp.pos = models[i].parts[j].pos;
			tp.rot = models[i].parts[j].rot;
			
			fwrite( &tp, 1, sizeof(tp), fp );
		}
	}
	
	memcpy( tch.magic, "tEOF", 4 );
	tch.chunkSize = 0;
	fwrite( &tch, 1, sizeof(tch), fp );
	
	fclose( fp );
	
	_triFreeCounts( &meshCounted );
	_triFreeCounts( &texCounted );
}

#endif


void triModelsFree( triModel* models, triS32 numModels )
{
	if (models==0 || numModels==0) return;
	
	triSInt i, j;
	
	for (j=0;j<numModels;j++)
	{
		for (i=0;i<models->numParts;i++)
		{
			triFree(models->parts[i].mesh->verts);
			triImageFree(models->parts[i].mesh->texture);
			triTextureRel( 1, &models->parts[i].mesh->texID );
			triFree(models->parts[i].mesh);
		}
		triFree(models->parts);
		triModel* next = models->next;
		triFree(models);
		models = next;
	}
}



void triModelOptimize( triModel* model, triS32 format )
{
	if (model==0) return;

	triS32 i;
	for (i=0;i<model->numParts;i++)
	{
		triMeshOptimize( model->parts[i].mesh, format );
	}
}

void triMeshOptimize( triMesh* mesh, triS32 format )
{
	if (mesh==0 || mesh->vertFormat==format || mesh->vertFormat!=TRI_VERTUVCN_FORMAT)
		return;

	triU32 vertSize = _triVertSize( format );

	void* vdata = triMalloc( vertSize * mesh->numVerts );
	if (vdata==0)
		return;

	triS32 texSize = (format&GU_TEXTURE_BITS)!=0?((format&GU_TEXTURE_32BITF)==GU_TEXTURE_32BITF?4:((format&GU_TEXTURE_BITS))):0;
	triS32 colSize = (format&GU_COLOR_BITS)?((format&GU_COLOR_8888)==GU_COLOR_8888?4:2):0;
	triS32 norSize = (format&GU_NORMAL_BITS)?((format&GU_NORMAL_32BITF)==GU_NORMAL_32BITF?4:((format&GU_NORMAL_BITS)>>5)):0;
	triS32 verSize = (format&GU_VERTEX_BITS)?((format&GU_VERTEX_32BITF)==GU_VERTEX_32BITF?4:((format&GU_VERTEX_BITS)>>7)):0;
	
	triS32 texPad, colPad, norPad, verPad;
	switch (format)
	{
		case TRI_VERTFASTUVCNF_FORMAT:
		{
			triVertFastUVCNf v;
			texPad = ((triU32)&v.color - (triU32)&v.v) - sizeof(v.v);
			colPad = ((triU32)&v.nx - (triU32)&v.color) - sizeof(v.color);
			norPad = ((triU32)&v.x - (triU32)&v.nz) - sizeof(v.nz);
			verPad = ((triU32)&v + sizeof(v) - (triU32)&v.z) - sizeof(v.z);
			break;
		}
		case TRI_VERTFASTUVNF_FORMAT:
		{
			triVertFastUVNf v;
			texPad = ((triU32)&v.nx - (triU32)&v.v) - sizeof(v.v);
			colPad = 0;
			norPad = ((triU32)&v.x - (triU32)&v.nz) - sizeof(v.nz);
			verPad = ((triU32)&v + sizeof(v) - (triU32)&v.z) - sizeof(v.z);
			break;
		}
		case TRI_VERTFASTUVCF_FORMAT:
		{
			triVertFastUVCf v;
			texPad = ((triU32)&v.color - (triU32)&v.v) - sizeof(v.v);
			colPad = ((triU32)&v.x - (triU32)&v.color) - sizeof(v.color);
			norPad = 0;
			verPad = ((triU32)&v + sizeof(v) - (triU32)&v.z) - sizeof(v.z);
			break;
		}
		case TRI_VERTFASTUVF_FORMAT:
		{
			triVertFastUVf v;
			texPad = ((triU32)&v.x - (triU32)&v.v) - sizeof(v.v);
			colPad = 0;
			norPad = 0;
			verPad = ((triU32)&v + sizeof(v) - (triU32)&v.z) - sizeof(v.z);
			break;
		}
		case TRI_VERTFASTCF_FORMAT:
		{
			triVertFastUVf v;
			texPad = 0;
			colPad = ((triU32)&v.x - (triU32)&v.color) - sizeof(v.color);
			norPad = 0;
			verPad = ((triU32)&v + sizeof(v) - (triU32)&v.z) - sizeof(v.z);
			break;
		}
		case TRI_VERTFASTCNF_FORMAT:
		{
			triVertFastUVf v;
			texPad = 0;
			colPad = ((triU32)&v.nx - (triU32)&v.color) - sizeof(v.color);
			norPad = ((triU32)&v.x - (triU32)&v.nz) - sizeof(v.nz);
			verPad = ((triU32)&v + sizeof(v) - (triU32)&v.z) - sizeof(v.z);
			break;
		}
	}
	/*
	triS32 sz = texSize*2;
	triS32 texPad = ((sz + (colSize-1)) & ~(colSize-1)) - sz;
	sz += colSize + texPad;
	triS32 colPad = ((sz + (norSize-1)) & ~(norSize-1)) - sz;
	sz += norSize*3 + colPad;
	triS32 norPad = ((sz + (verSize-1)) & ~(verSize-1)) - sz;
	sz += verSize*3 + norPad;
	triS32 verPad = _triVertSize( format ) - sz;*/

	triLogPrint("Optimizing mesh (%x) from %i to %i bytes...\n", mesh, _triVertSize( mesh->vertFormat ) * mesh->numVerts, _triVertSize( format ) * mesh->numVerts );
	triLogPrint("texSize: %i - texPad: %i\ncolSize: %i - colPad: %i\nnorSize: %i - norPad: %i\nverSize: %i - verPad: %i\nvertexSize: %i\n", texSize, texPad, colSize, colPad, norSize, norPad, verSize, verPad, vertSize );
	
	void* v = vdata;
	triVertUVCN* src = (triVertUVCN*)mesh->verts;
	triFloat* fsrc = (triFloat*)mesh->verts;
	triS32 i, j;
	for (j=0;j<mesh->numVerts;j++)
	{
		triU32 start = (triU32)v;
		if (format&GU_TEXTURE_BITS)
		{
			if ((mesh->vertFormat&GU_TEXTURE_BITS)==(format&GU_TEXTURE_BITS))
			{
				v = (void*)(((triU32)v+3) & ~3);
				triFloat* fv = (triFloat*)v;
				*fv++ = src->u;
				*fv++ = src->v;
				v = fv;
			}
			else
			{
				if (format&GU_TEXTURE_8BIT)
				{
					triU8* v8 = (triU8*)v;
					*v8++ = (triU8)(src->u * 127.f);
					*v8++ = (triU8)(src->v * 127.f);
					v = v8;
				}
				else
				{
					v = (void*)(((triU32)v+1) & ~1);
					triU16* v16 = (triU16*)v;
					*v16++ = (triU16)(src->u * 32767.f);
					*v16++ = (triU16)(src->v * 32767.f);
					v = v16;
				}
			}
		}
		
		if (format&GU_COLOR_BITS)
		{
			if ((mesh->vertFormat&GU_COLOR_BITS)==(format&GU_COLOR_BITS))
			{
				v = (void*)(((triU32)v+3) & ~3);
				*((triU32*)v) = src->color;
				v+=4;
			}
			else
			{
				v = (void*)(((triU32)v+1) & ~1);
				triU16* v16 = (triU16*)v;
				triU32 color = src->color;
				triU32 r = ((color >> 24) & 0xFF), g = ((color >> 16) & 0xFF), b = ((color) & 0xFF), a = ((color >> 24) & 0xFF);
				if (format&GU_COLOR_4444)
					*v16++ = (triU16)(((a >> 4)<<12) | ((r >> 4)<<8) | ((g >> 4)<<4) | (b >> 4));
				else
				if (format&GU_COLOR_5551)
					*v16++ = (triU16)(((a >> 7)<<15) | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3));
				else
				if (format&GU_COLOR_5650)
					*v16++ = (triU16)(((r >> 3)<<11) | ((g >> 2)<<5) | (b >> 3));
				v = v16;
			}
		}
		
		if (format&GU_NORMAL_BITS)
		{
			if ((mesh->vertFormat&GU_NORMAL_BITS)==(format&GU_NORMAL_BITS))
			{
				v = (void*)(((triU32)v+3) & ~3);
				triFloat* fv = (triFloat*)v;
				*fv++ = src->nx;
				*fv++ = src->ny;
				*fv++ = src->nz;
				v = fv;
			}
			else
			{
				if (format&GU_NORMAL_8BIT)
				{
					triU8* v8 = (triU8*)v;
					*v8++ = (triS8)(src->nx * 127.f);
					*v8++ = (triS8)(src->ny * 127.f);
					*v8++ = (triS8)(src->nz * 127.f);
					v = v8;
				}
				else
				{
					v = (void*)(((triU32)v+1) & ~1);
					triU16* v16 = (triU16*)v;
					*v16++ = (triS16)(src->nx * 32767.f);
					*v16++ = (triS16)(src->ny * 32767.f);
					*v16++ = (triS16)(src->nz * 32767.f);
					v = v16;
				}
			}
		}
		
		if (format&GU_VERTEX_BITS)
		{
			if ((mesh->vertFormat&GU_VERTEX_BITS)==(format&GU_VERTEX_BITS))
			{
				v = (void*)(((triU32)v+3) & ~3);
				triFloat* fv = (triFloat*)v;
				*fv++ = src->x;
				*fv++ = src->y;
				*fv++ = src->z;
				v = fv;
			}
			else
			{
				if (format&GU_VERTEX_8BIT)
				{
					triU8* v8 = (triU8*)v;
					*v8++ = (triS8)(src->x * 127.f);
					*v8++ = (triS8)(src->y * 127.f);
					*v8++ = (triS8)(src->z * 127.f);
					v = v8;
				}
				else
				{
					v = (void*)(((triU32)v+1) & ~1);
					triU16* v16 = (triU16*)v;
					*v16++ = (triS16)(src->x * 32767.f);
					*v16++ = (triS16)(src->y * 32767.f);
					*v16++ = (triS16)(src->z * 32767.f);
					v = v16;
				}
			}
		}
		if (((triU32)v-start)!=vertSize)
			triLogPrint("Vertex size mismatch!\n");
		src++;
	}
	
	triFree( mesh->verts );
	mesh->verts = vdata;
	mesh->vertFormat = format;
	sceKernelDcacheWritebackAll();
	triLogPrint("Finished optimizing mesh.\n");
}


triMesh* triMeshLoadTrim ( triChar* fileName, triChar* texName )
{
	triMesh*	pMesh;
	FILE*		pFile;

	if ((pFile = fopen (fileName, "rb")))
	{
		pMesh				= triMalloc (sizeof(triMesh));
		memset (pMesh, 0, sizeof (triMesh));

		fread (&pMesh->numVerts, sizeof(triU32), 1, pFile);

		pMesh->renderFormat	= GU_TRIANGLES;
		pMesh->vertFormat	= TRI_VERTUVCN_FORMAT;
		pMesh->verts		= triMalloc (sizeof(triVertUVCN) * pMesh->numVerts);
		memset (pMesh->verts, 0, sizeof(triVertUVCN) * pMesh->numVerts);

		fread (pMesh->verts, sizeof(triVertUVCN) * pMesh->numVerts, 1, pFile);

		fclose (pFile);

		triLogPrint ( "trying to load %s\r\n", texName );

		pMesh->texture	= triImageLoad ( texName, 0 );

		if ( pMesh->texture )
			triImageSwizzle ( pMesh->texture );
	}
	else
	{
		triLogError ( "Failed to open model: %s\r\n", fileName );
		return NULL;
	}

	return pMesh;
}

void triMeshFree ( triMesh* pMesh )
{
	if ( pMesh )
	{
		if ( pMesh->texture )
			triImageFree ( pMesh->texture );

		triFree ( pMesh->verts );

		triFree ( pMesh );
	}
}
