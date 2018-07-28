/*
 * triParticle.h: Header for particle engine
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


#ifndef __TRIPARTICLE_H__
#define __TRIPARTICLE_H__

#include <pspgu.h>
#include "triTypes.h"


#define ALIGN16 __attribute__((aligned(16)))

/** @defgroup triParticle Particle system
 *  @{
 */
 
typedef struct triParticle
{
	triVec4f		pos;			// (x,y,z,rotation)	- rotation only applicable in GU_TRIANGLES mode
	triVec4f		vel;			// (x,y,z,rotation) - rotation speed in degree/second

	triColor4f		col;
	triFloat		rand;			// randomness factor of this particle (to randomize different emitter parameters)
	triFloat		size;			// size of the particle, only applicable in GU_SPRITES and GU_TRIANGLES mode

	triFloat		age;
	triFloat		lifetime;
	//triFloat		pad16;			// padding to 16byte alignment
} ALIGN16 triParticle;	// struct size: 16*4 bytes = 64 bytes


#define TRI_VORTEX_RANGE (3.0f)
typedef struct triVortex
{
	triVec4f		pos;
	triVec4f		vel;

	triFloat		confinement;	// vorticity confinement value (how far the vortex ranges)
	triFloat		dir;			// vortex rotation direction (+-) and speed

	triFloat		age;
	triFloat		lifetime;
} ALIGN16 triVortex;	// struct size: 16*4 bytes = 48 bytes


enum triParticleEmitterTypes
{
	TRI_EMITTER_MANUAL = 0,
	TRI_EMITTER_FIRE,
	TRI_EMITTER_EXPLOSION,
	TRI_EMITTER_SPRINKLE,
	TRI_EMITTER_WATERFALL,
	TRI_EMITTER_SMOKE,
	TRI_EMITTER_NUM_TYPES
};

/*
 * General usage of values with randomness:
 *   result = value + (-1.0,..,1.0) * valueRand
 *
 * To generate values in range X - Y, set as follows:
 *   valueRand = (Y-X) / 2.0
 *   value = X + valueRand
 */
typedef struct triParticleEmitter
{
	triVec4f		pos;			// position of emitter and initial position of all emitted particles (x,y,z,rotation)
	triVec4f		posRand;		// length in all directions of random position (0 for no random placement) (x,y,z,rotation)
	triVec4f		lastpos;		// internal: last position of emitter (for moving particles with emitter)

	triVec4f		vel;			// initial velocity of all emitted particles (x,y,z,rotation)
	triVec4f		velRand;		// randomness of particle velocity (x,y,z,rotation)
	
	triVec4f		gravity;		// Gravital force on emitter particles

	triVec4f		wind;			// wind direction vector
	triVec4f		windRand;		// wind randomness

	triColor4f		cols[8];		// Color shades during lifetime of particle, max 8 currently
	triS32			numCols;		// Number of color fades

	triFloat		size;			// Mean size of particles
	triFloat		sizeRand;		// Random size (0 means no randomness)
	
	triFloat		burnout;		// Burnout of the emitter with age. 1.0 means the emitters rate gradually turns towards 0 (and particles life towards ~20%) with age, 0 means no burnout

	triFloat		friction;		// air friction to damp particle velocity, 0 = no friction, 1.0 = stop immediately (same as vel = 0)

	triFloat		growth;			// Amount to grow particles (size/second) - Size after end of life = size + rand()*sizeRand + growth*life

	triFloat		glitter;		// Amount of glitter on particles (sinusform brightening) - 0 means no glitter, 1.0 means glitter in full intensity range
	triFloat		glitterSpeed;	// Speed of glitter (number of wavelengths inside particles age)

	triFloat		life;			// Lifetime of particles to be created (lifeRand is 0.2 by default, ie 20%)
	triFloat		lifeRand;		// Lifetime of particles randomness
	
	triFloat		binding;		// binding of particles to emitter, 0 = no binding, 1.0 = particles move with emitter
	triFloat		loosen;			// loosening of particles with age, 0 = no loosening, 1.0 = particles move completely free at end of life

	triS32			hTexFrames;		// number of horizontal texture frames (texture animation)
	triS32			vTexFrames;		// number of vertical texture frames (texture animation)
	triS32			nTexLoops;		// number of loops to do per particle Lifetime (0 means no texture animation at all)
	triS32			fixedTexRate;	// fixed texture animation frame rate in frames/second (0 if rate dependent on life - use nTexLoops*vTexFrames*hTexFrames/life)
	
	triS32			min;			// minimum number of particles at same time
	triS32			max;			// maximum number of particles at same time
	triS32			minVortex;		// minimum number of vortex particles at same time
	triS32			maxVortex;		// maximum number of vortex particles at same time
	
	triFloat		vortexRange;	// The squared range of the vortices influence (+- 20%)
	triFloat		vortexDir;		// Vortex direction (and speed)
	triFloat		vortexDirRand;	// Vortex direction randomness

	triS32			rate;			// emission rate (particles/second) - rate*lifetime is amount of emitted particles in total
	triS32			rateVortex;		// vortex emission rate (vortices/second)
	
	triFloat		lifetime;		// lifetime of emitter in seconds, 0 if eternal (fueled flame)
	triFloat		age;			// internal: age of the emitter, if age > lifetime it will die unless lifetime = 0
	
	triFloat		lastemission;	// internal: time of last emission (for proper emittance rate)
	triS32			emitted;		// internal: particles emitted since last emission
	triS32			emittedVortex;	// internal: vortices emitted since last emission
	
	triFloat		padding[3];
} ALIGN16 triParticleEmitter;



typedef struct triBlendMode
{
	triS32		op;
	triS32		src_op;
	triS32		dst_op;
	triU32		src_fix;
	triU32		dst_fix;
} triBlendMode;


extern triBlendMode TRI_BLEND_MODE_ALPHA /*= { GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 }*/;
extern triBlendMode TRI_BLEND_MODE_ADD /*= { GU_ADD, GU_FIX, GU_FIX, 0xFFFFFFFF, 0xFFFFFFFF }*/;
extern triBlendMode TRI_BLEND_MODE_GLENZ /*= { GU_ADD, GU_FIX, GU_FIX, 0x7F7F7F7F, 0x7F7F7F7F }*/;
extern triBlendMode TRI_BLEND_MODE_ALPHA_ADD /*= { GU_ADD, GU_SRC_ALPHA, GU_FIX, 0, 0xFFFFFFFF }*/;
extern triBlendMode TRI_BLEND_MODE_SUB /* = { GU_SUBTRACT, GU_FIX, GU_FIX, 0xFFFFFFFF, 0xFFFFFFFF }*/;
extern triBlendMode TRI_BLEND_MODE_ALPHA_SUB /* = { GU_SUBTRACT, GU_SRC_ALPHA, GU_FIX, 0, 0xFFFFFFFF }*/;


enum triParticleActions {
	triApplyForce,					// apply external force (gravity or similar)
	triInternalGravity,				// calculate internal gravity and move particles accordingly
	triInternalCollide,				// collide particles against each other
	triEmitterBound,				// bind particles to the emitter - ie move them along with it based on age of particle
	triCollide,						// collide particles against exterior mesh
	triDie,							// let particles die (always applied)
	triNumActions
	};


typedef struct triParticleSystem triParticleSystem;

/** Custom particle render callback
  * @param s Pointer to particle system
  * @param p Pointer to particle structure to render
  */
typedef void (*triParticleRenderer)(triParticleSystem* s, triParticle* p);


struct triParticleSystem
{
	triS32					ID;							// Particle system ID
	triS32					typeID;						// Particle system type ID - see triParticleEmitterTypes

	triParticleEmitter		*emitter;					// The emitter attached to this system

	triS32					textureID;					// Texture to use for this particle system
	triS32					texMode;					// one of GU_TFX_*
	triU32					texColor;					// sceGuTexEnvColor
	triBlendMode			blendMode;
	triS32					renderMode;					// GU_POINTS, GU_LINES, GU_SPRITES, GU_TRIANGLES

	triS32					useBillboards;				// make particles always point towards camera, only applicable in GU_TRIANGLES_MODE

	triU32					actions[triNumActions];		// actions to apply every frame


	triParticle* 			particles;					// particle list for update/movement
	triVortex* 				vortices;					// vortex particle list
	triS32*					particleStack;
	triS32*					vorticesStack;
	triS32					numParticles;
	triS32					numVortices;

	triS32					numVerticesPerParticle;		// numbers to allocate the right amount of memory

	void* 					vertices[2];				// particle vertice list for rendering (created during update)
	triS32					numVertices;
	triS32					vertexFormat;
	triS32					vindex;

	triVec3f				boundingBox[8];
	triS32					updateFlag;
	
	
	triParticleRenderer		render;						// function pointer to custom particle rendering function
	struct triParticleSystem*	next;
};



typedef struct triParticleManager
{
	triFloat				dt;
	triParticleSystem*		systems;
	triS32					numSystems;
	
	triS32					idCounter;
	
	triU32					numParticles;
	triU32					numVertices;
} triParticleManager;



void triParticleSystemConstructor( triParticleSystem* s );
void triParticleSystemFree( triParticleSystem* s );
triS32 triParticleSystemRender( triParticleSystem* s );
void triParticleSystemInitialize( triParticleSystem* s, triParticleEmitter* e );
void triParticleSystemUpdate( triParticleSystem* s, triCamera* cam, triFloat dt );


triS32 triParticleVertexUVCListCreate( triParticleSystem* s, triCamera* cam );
triS32 triParticleVertexCListCreate( triParticleSystem* s, triCamera* cam );


void triParticleEmitterConstructor( triParticleEmitter *e, triS32 emitterType );


void triParticleManagerUpdate( triCamera* cam, triFloat dt );
void triParticleManagerRender();
void triParticleManagerUpdateRender( triCamera* cam, triFloat dt );
void triParticleManagerRemove( triS32 id );
triParticleSystem* triParticleManagerGet( triS32 id );
void triParticleManagerDestroy();
triS32 triParticleManagerAdd( triParticleSystem* p, triParticleEmitter* e );
triS32 triParticleManagerLoadScript( triChar* name );

/** @} */

#endif // __TRIPARTICLE_H__
