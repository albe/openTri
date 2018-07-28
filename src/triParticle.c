/*
 * triParticle.c: Code for particle engine
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

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "triVMath_vfpu.h"
#include "tri3d.h"
#include "triCamera.h"
#include "triTexman.h"
#include "triParticle.h"
#include "triMemory.h"
#include "triLog.h"
#include "triTimer.h"



triParticleManager TRI_PARTMAN = { 0, 0, 0 };

triBlendMode TRI_BLEND_MODE_ALPHA = { GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 };
triBlendMode TRI_BLEND_MODE_ADD = { GU_ADD, GU_FIX, GU_FIX, 0xFFFFFFFF, 0xFFFFFFFF };
triBlendMode TRI_BLEND_MODE_GLENZ = { GU_ADD, GU_FIX, GU_FIX, 0x7F7F7F7F, 0x7F7F7F7F };
triBlendMode TRI_BLEND_MODE_ALPHA_ADD = { GU_ADD, GU_SRC_ALPHA, GU_FIX, 0, 0xFFFFFFFF };
triBlendMode TRI_BLEND_MODE_SUB = { GU_REVERSE_SUBTRACT, GU_FIX, GU_FIX, 0xFFFFFFFF, 0xFFFFFFFF };
triBlendMode TRI_BLEND_MODE_ALPHA_SUB = { GU_REVERSE_SUBTRACT, GU_SRC_ALPHA, GU_FIX, 0, 0xFFFFFFFF };


void triParticleSystemConstructor( triParticleSystem* s )
{
	if (s==0) return;

	s->emitter = 0;
	s->particles = 0;
	s->numParticles = 0;
	s->particleStack = 0;
	s->vortices = 0;
	s->vorticesStack = 0;
	s->numVortices = 0;
	
	s->vertices = 0;
	s->numVertices = 0;
	
	s->textureID = -1;
	s->renderMode = GU_SPRITES;
	s->texMode = GU_TFX_MODULATE;
	s->blendMode = TRI_BLEND_MODE_ALPHA_ADD;
	s->useBillboards = 1;
	s->updateFlag = 1;
	memset( s->actions, 0, sizeof(s->actions) );
}


void triParticleSystemFree( triParticleSystem* s )
{
	if (s==0) return;

	triLogPrint("Closing particle system %p...\n", s);
	s->emitter = 0;
	if (s->particles!=0) triFree(s->particles);
	triLogPrint("triFreed particles...\n");
	s->particles = 0;
	s->numParticles = 0;
	if (s->particleStack!=0) triFree(s->particleStack);
	triLogPrint("triFreed particle stack...\n");
	s->particleStack = 0;
	if (s->vortices!=0) triFree(s->vortices);
	triLogPrint("triFreed vortices...\n");
	s->vortices = 0;
	if (s->vorticesStack!=0) triFree(s->vorticesStack);
	triLogPrint("triFreed vortices stack...\n");
	s->vorticesStack = 0;
	s->numVortices = 0;
	
	if (s->vertices!=0) triFree(s->vertices);
	triLogPrint("triFreed vertexbuffer...\n");
	s->vertices = 0;
	s->numVertices = 0;
	
	s->textureID = -1;
	s->renderMode = GU_SPRITES;
	s->texMode = GU_TFX_MODULATE;
	s->blendMode = TRI_BLEND_MODE_ALPHA_ADD;
	s->useBillboards = 0;
	memset( s->actions, 0, sizeof(s->actions) );
}


void triParticleSystemInitialize( triParticleSystem* s, triParticleEmitter* e )
{	
	s->emitter = e;

	s->particles = triMalloc( sizeof(triParticle)*e->max );
	if (s->particles==0) return;
	
	s->particleStack = triMalloc( sizeof(triS32)*e->max );
	if (s->particleStack==0) return;

	if (e->maxVortex>0)
	{
		s->vortices = triMalloc( sizeof(triVortex)*e->maxVortex );
		if (s->vortices==0) return;
		
		s->vorticesStack = triMalloc( sizeof(triS32)*e->maxVortex );
		if (s->vorticesStack==0) return;
	}
	s->numParticles = 0;
	s->numVortices = 0;
	
	triS32 i;
	for (i=0;i<e->max;i++)
		s->particleStack[i] = i;
	for (i=0;i<e->maxVortex;i++)
		s->vorticesStack[i] = i;
	triLogPrint("Initiated particle Stack...\n");

	switch (s->renderMode)
	{
		case GU_POINTS: s->numVerticesPerParticle = 1; break;
		case GU_LINES:
		case GU_SPRITES: s->numVerticesPerParticle = 2; break;
		case GU_TRIANGLES:
		case GU_TRIANGLE_FAN:
		case GU_TRIANGLE_STRIP: s->numVerticesPerParticle = 6; break;
	}

	if (s->render==0)
	{
		if (s->textureID!=-1)
		{
			s->vertices = triMalloc( sizeof(triVertFastUVCf)*e->max*s->numVerticesPerParticle );
			s->vertexFormat = TRI_VERTFASTUVCF_FORMAT;
		}
		else
		{
			s->vertices = triMalloc( sizeof(triVertC)*e->max*s->numVerticesPerParticle );
			s->vertexFormat = TRI_VERTC_FORMAT;
		}
		if (s->vertices==0)
			triLogError("ERROR allocating vertex memory!\n");
	}
}


triS32 triParticleSystemRender( triParticleSystem* s )
{
	if (s==0) return(0);
	if (s->numVertices==0) return(0);
	
	if (s->render!=0)
	{
		int i = 0;
		for (;i<s->numParticles;i++)
		{
			s->render(s, &s->particles[s->particleStack[i]]);
		}
		return(1);
	}
	
	sceGumUpdateMatrix();
	sceGuBeginObject( GU_VERTEX_32BITF|GU_TRANSFORM_3D, 8, 0, s->boundingBox );
	if (s->textureID>=0 && (s->vertexFormat&(GU_TEXTURE_16BIT | GU_TEXTURE_32BITF))!=0)
	{
		sceGuEnable(GU_TEXTURE_2D);
		triTextureBind( s->textureID );
		sceGuTexFunc(s->texMode, GU_TCC_RGBA);
		sceGuTexEnvColor(s->texColor);
	}
	else
		sceGuDisable(GU_TEXTURE_2D);
	
	sceGuDisable(GU_CULL_FACE);
	sceGuEnable(GU_BLEND);
	sceGuDepthMask(GU_TRUE); // No depth writes
	sceGuBlendFunc(s->blendMode.op,s->blendMode.src_op,s->blendMode.dst_op,s->blendMode.src_fix,s->blendMode.dst_fix);
	sceGumDrawArray(s->renderMode,s->vertexFormat|GU_TRANSFORM_3D,s->numVertices,0,s->vertices);
	sceGuEnable(GU_CULL_FACE);
	sceGuDisable(GU_BLEND);
	sceGuDepthMask(GU_FALSE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEndObject();
	return(1);
}



static inline void triParticleRemove( triParticleSystem* s, triS32 idx )
{
	if (s->numParticles<=0 || idx<0) return;
	s->numParticles--;
	if (s->numParticles==idx) return;
	triS32 temp = s->particleStack[s->numParticles];
	s->particleStack[s->numParticles] = s->particleStack[idx];
	s->particleStack[idx] = temp;
	
}


static inline triS32 triParticleAdd( triParticleSystem* s )
{
	if (s->numParticles>=s->emitter->max) return (-1);
	triS32 idx = s->numParticles++;
	return idx;
}


static inline void triVortexRemove( triParticleSystem* s, triS32 idx )
{
	if (s->numVortices<=0 || idx<0) return;
	s->numVortices--;
	if (s->numVortices==idx) return;
	triS32 temp = s->vorticesStack[s->numVortices];
	s->vorticesStack[s->numVortices] = s->vorticesStack[idx];
	s->vorticesStack[idx] = temp;
	
}

static inline triS32 triVortexAdd( triParticleSystem* s )
{
	if (s->numVortices>=s->emitter->maxVortex) return (-1);
	triS32 idx = s->numVortices++;
	return idx;
}


static inline triFloat vcosf( triFloat s )
{
	register triFloat ret;
	__asm__(
		"mtv	%1, s000\n"
		"vcst.s  s001, VFPU_2_PI\n"
        "vmul.s  s000, s000, s001\n"
		"vcos.s	s000, s000\n"
		"mfv	%0, s000\n"
	:"=r"(ret)
	:"r"(s)
	);
	return ret;
}

static inline triS32 triParticleUpdateLoadEmitter( triParticleEmitter* e, triFloat dt )
{
	__asm__(
	"lv.q	c320, 32 +%0\n"			// e->lastPos
	"lv.s	s110, 256+%0\n"			// e->numCols
	"lv.q	c100, 272+%0\n"			// e->friction, e->growth, e->glitter, e->glitterSpeed
	
	"lv.s	s310, 296+%0\n"			// e->binding
	"lv.s	s311, 300+%0\n"			// e->loosen
	
	"vmov.p	c120, c102\n"			// c120 = e->glitter, e->glitterSpeed
	"mtv	%1, s102\n"				// dt
	
	"vmul.s s120, s120, s120[1/2]\n"// e->glitter*0.5
	"vmul.s s121, s121, s121[2]\n"	// e->glitterSpeed*2
	
	"vi2f.s s110, s110, 0\n"		// e->numCols
	"vsub.s	s110, s110, s110[1]\n"	// e->numCols-1
	
	"vscl.t	c100, c100[Y,1,X], s102\n"		// e->growth*dt, dt, e->friction*dt
	"vocp.s s102, s102\n"			// 1.0 - e->friction*dt
	::"m"(*e),"r"(dt)
	);
}

triU32 min = 99999, max = 0;

triVec4f temp_vel, temp_force;
static inline triS32 triParticleUpdate( triParticleEmitter* e, triParticle* p, triVec4f* f, triFloat dt )
{
	if (p->age>=p->lifetime || p->size<=0.0f)
		return 0;
#ifdef BENCH_CODE
	triS32 start = 0, end;
	sceKernelIcacheInvalidateAll();	// We don't want to bench the I-Cache
	__asm__(
		".set push\n"
		".set noreorder\n"
		"vsync\n"
		"sync\n"
		"mfc0    %0, $9\n"
        :"=r"(start));
#endif

#if 1
	// Faster at around 10-20 times without I-Cache and even more with I-Cache
	__asm__(
	// M000 = particle
	"lv.q	c000, 0+%1\n"
	"lv.q	c010, 16+%1\n"
	"lv.q	c030, 48+%1\n"			// p->size, p->age

	//"lv.q	c320, 32 +%2\n"			// e->lastPos
	"lv.q	c020, 128+%2\n"			// c020 = e->cols[0]
	//"lv.s	s110, 256+%2\n"			// e->numCols
	//"lv.q	c100, 272+%2\n"			// e->friction, e->growth, e->glitter, e->glitterSpeed

	"lv.q	c200, %3\n"				// force

	//"vmov.p	c120, c102\n"			// c120 = e->glitter, e->glitterSpeed

	//"mtv	%4, s102\n"				// dt

	"vrcp.s	s303, s033\n"			// 1.0 / p->lifetime

	//"vi2f.s s110, s110, 0\n"		// e->numCols
	//"vsub.s	s110, s110, s110[1]\n"	// e->numCols-1
	"vcmp.s	EZ, s110\n"
	
	//"vscl.t	c100, c100[Y,1,X], s102\n"		// e->growth*dt, dt, e->friction*dt
	//"vocp.s s102, s102\n"			// 1.0 - e->friction*dt
	"vadd.t c030, c030, c100[0,X,Y]\n"		// p->size += e->growth*dt, p->age += dt
	"vscl.q c200, c200, s101\n"		// force*e->firction
	"vmul.s s302, s032, s303\n"		// p->age / p->lifetime

	// Interpolate color
	"bvt	4,	0f\n"
	"vmul.s	s410, s110, s302\n"		// s410 = age * (e->numCols-1)
	"vf2iz.s	s411, s410,	0\n"	// s411 = trunc(s410)
	"la		$9, %2\n"
	"mfv	$8, s411\n"
	"vi2f.s	s411, s411, 0\n"
	"sll	$8, $8, 4\n"			// $8 * 16
	"vsub.s s410, s410, s411\n"		// s410 = s410 - trunc(s410) = frac(s410)
	"addu	$8, $9, $8\n"
	"addiu  $8, $8, 128\n"

	"lv.q	c020,  0($8)\n"
	"lv.q	c130, 16($8)\n"

	"vocp.s	s411, s410\n"
	"vscl.q	c020, c020, s411\n"
	"vscl.q c130, c130, s410\n"
	"vadd.q c020, c130, c020\n"
	"0:\n"
	
	"vcmp.s EZ, s120\n"		// e->glitter = 0?
	//"lv.s	s310, 296+%2\n"			// e->binding
	//"lv.s	s311, 300+%2\n"			// e->loosen


	"vscl.q			c010, c010, s102\n"		// c010 = p->vel * friction
	
	// if (e->binding > 0.0f)
	//	triVec4Add3( &p->pos, &p->pos, triVec4Scale3( &temp_force, &e->lastpos, e->binding*(1.0f - (age * e->loosen)) ) );
	"vmul.s	s411, s311, s302\n"		// e->loosen*age
	"vscl.q			c210, c010, s101\n"
	"vocp.s s411, s411\n"
	"vadd.t			c010, c010, c200\n"
	"vmul.s s411, s411, s310\n"		// e->binding*(1.0f - (age * e->loosen))
	"vadd.t			c000, c000, c210\n"
	"vscl.t	c320, c320, s411\n"
	"vadd.t c000, c000, c320\n"

	// Glitter
	"bvtl	4,	1f\n"
	"vfim.s s123, 0.1\n"	// NOTE: precision problem?
	// p->col.a += e->glitter*0.5f*vcosf( (p->rand*0.1f+1.0f)*e->glitterSpeed*GU_PI*age );
	"vmul.s s122, s020, s123\n"	// p->rand*0.1
	"vmul.s s421, s121, s302\n"	// e->glitterSpeed * age
	//"vmul.s s121, s121, s123[2]\n" // e->glitterSpeed * age * 2
	"vadd.s s122, s122, s122[1]\n"	// p->rand*0.1+1.0
	
	"vmul.s s421, s421, s122\n"
	"vcos.s s421, s421\n"
	"vmul.s s420, s120, s421\n"	// e->glitter * 0.5 * vcos( ... )
	"vadd.s s023, s023, s420\n"
	"1:\n"

	"sv.q	c000, 0+ %0\n"
	"sv.q	c010, 16+%0\n"
	"sv.q	c020, 32+%0\n"
	"sv.q	c030, 48+%0\n"
	:"=m"(*p):"m"(*p),"m"(*e),"m"(*f)
	);

#else
	triFloat age = p->age / p->lifetime;
	if (e->numCols > 1)
	{
		triFloat t = age * (e->numCols - 1);
		triS32 i = (triS32)t;
		t -= i;
		triVec4Lerp( (triVec4f*)&p->col, (triVec4f*)&e->cols[i], (triVec4f*)&e->cols[i+1], t );
	}
	else
	{
		p->col = e->cols[0];
	}

	triFloat friction = 1.0f - (e->friction * dt);
	if (e->glitter > 0.0f)
		p->col.a += e->glitter*0.5f*vcosf( (p->rand*0.1f+1.0f)*e->glitterSpeed*GU_PI*age );

	triVec4Scale( &p->vel, &p->vel, friction );
	triVec4Scale( &temp_vel, &p->vel, dt );
	triVec4Scale3( &temp_force, f, dt );
	triVec4Add3( &p->vel, &p->vel, &temp_force );
	triVec4Add( &p->pos, &p->pos, &temp_vel );

	//if (p->pos.w > 360.0f) p->pos.w -= 360.0f;

	// FIXME: Use a better formula to move particles with the emitter
	if (e->binding > 0.0f)
		triVec4Add3( &p->pos, &p->pos, triVec4Scale3( &temp_force, &e->lastpos, e->binding*(1.0f - (age * e->loosen)) ) );
	
	p->size += e->growth*dt;
	p->age += dt;
#endif
#ifdef BENCH_CODE
	__asm__(
	"vsync\n"	// Make sure all VFPU mem reads/writes finished
	"sync\n"	// Make sure all CPU mem reads/writes finished
	"mfc0                    %0, $9\n"
	".set pop\n"
	:"=r"(end));
	triU32 delta = (end-start);
	//triLogPrint("Update: %u\n", delta);
	if (delta<min) min = delta;
	if (delta>max) max = delta;
#endif
	return 1;
}

static inline triS32 triVortexUpdate( triParticleEmitter* e, triVortex* v, triVec4f* f, triFloat dt )
{
	if (v->age>=v->lifetime)
		return 0;

	triFloat friction = 1.0f - (e->friction * dt);
	triVec4Scale3( &v->vel, &v->vel, friction );
	triVec4Scale3( &temp_vel, &v->vel, dt );
	triVec4Scale3( &temp_force, f, dt );
	triVec4Add3( &v->vel, &v->vel, &temp_force );
	triVec4Add3( &v->pos, &v->pos, &temp_vel );

	//if (e->binding > 0.0)
	//	triVec4Add3( &v->pos, &v->pos, triVec4Scale3( &temp_force, &e->lastpos, e->binding*(1.0f - (age * e->loosen)) ) );

	v->age += dt;

	return 1;
}


//#define randf() ((float)(rand()-RAND_MAX/2)/(RAND_MAX/2))

// FIXME: Put the whole function in VFPU code
triVec4f randVec;
static inline void triParticleCreate( triParticleEmitter* e, triParticle* p )
{
	p->col = e->cols[0];
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c030, 0 + %1\n"			// c030 = e->pos
		"lv.q			c020, 16+ %1\n"			// c020 = e->posRand
		"lv.q			c130, 48+ %1\n"			// c130 = e->vel
		"lv.q			c120, 64+ %1\n"			// c020 = e->velRand
		"vrndf2.q		c000\n"					// c000 = rnd(2.0, 4.0)
		"vrndf2.q		c100\n"					// c100 = rnd(2.0, 4.0)
		"vsub.q			c000, c000, c000[3,3,3,3]\n"// c000 = | -1.0 ... 1.0 |
		"vsub.q			c100, c100, c100[3,3,3,3]\n"// c100 = | -1.0 ... 1.0 |
		"vdot.t			s010, c000, c000\n"		// s010 = x*x + y*y + z*z
		"vdot.t			s110, c100, c100\n"		// s110 = x*x + y*y + z*z
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / s010
		"vrsq.s			s110, s110\n"			// s110 = 1.0 / s110
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s010\n"		// c000 = c000 / s010
		"vscl.t			c100[-1:1,-1:1,-1:1], c100, s110\n"		// c100 = c100 / s110
		"vmul.q			c000, c000, c020\n"
		"vmul.q			c100, c100, c120\n"
		"vadd.q			c030, c030, c000\n"
		"vadd.q			c130, c130, c100\n"
		"sv.q			c030, 0 + %0\n"
		"sv.q			c130, 16+ %0\n"
		
		/*
		"vrndf2.t		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.t			c000, c000, c000[3,3,3]\n"	// c000 = | -1.0 ... 1.0 |
		
		"vadd.q			c010, c010, c000\n"
		"sv.q			c010, 32+ %0\n"*/		// (rand, size, age, lifetime)
		".set			pop\n"					// restore assember option
		: "=m"(*p)
		: "m"(*e)
	);
	//p->pos = e->pos;
	//p->vel = e->vel;
	// All four component since last component includes the rotation parameter
	//triVec4Add( &p->pos, &p->pos, triVec4Mul( &randVec, triVec4Rndn3( &randVec ), &e->posRand ) );
	//triVec4Add( &p->vel, &p->vel, triVec4Mul( &randVec, triVec4Rndn3( &randVec ), &e->velRand ) );
	
	triVec3Rnd2( (triVec3*)&randVec );
	p->size = e->size + randVec.x*e->sizeRand;
	
	p->lifetime = e->life + randVec.y*e->lifeRand;	// lifetime
	p->age = 0.0f;
	p->rand = randVec.z;
	
	if (e->lifetime>0 && e->burnout>0)
	{
		// Have shorter particle lifes depending on emitter age
		float ef = e->age / e->lifetime;
		if (ef>1.0f) ef = 1.0f;

		p->lifetime *= (1.0f - e->burnout*ef*0.8f);		// lifetime * [0.2, 1.0]
	}
	
	#ifdef DEBUG2
	triLogPrint("Created particle: pos: <%.2f, %.2f, %.2f>\nvel: <%.2f, %.2f, %.2f>\nsize: %.2f\nage: %.2f\nlifetime: %.2f\n", p->pos.x, p->pos.y, p->pos.z, p->vel.x, p->vel.y, p->vel.z, p->size, p->age, p->lifetime );
	#endif
}

static inline void triVortexCreate( triParticleEmitter* e, triVortex* v )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c030, 0 + %1\n"			// c030 = e->pos
		"lv.q			c020, 16+ %1\n"			// c020 = e->posRand
		"lv.q			c130, 48+ %1\n"			// c130 = e->vel
		"lv.q			c120, 64+ %1\n"			// c020 = e->velRand
		"vrndf2.t		c000\n"					// c000 = rnd(2.0, 4.0)
		"vrndf2.t		c100\n"					// c100 = rnd(2.0, 4.0)
		"vsub.t			c000, c000, c000[3,3,3]\n"// c000 = | -1.0 ... 1.0 |
		"vsub.t			c100, c100, c100[3,3,3]\n"// c100 = | -1.0 ... 1.0 |
		"vdot.t			s010, c000, c000\n"		// s010 = x*x + y*y + z*z
		"vdot.t			s110, c100, c100\n"		// s110 = x*x + y*y + z*z
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / s010
		"vrsq.s			s110, s110\n"			// s110 = 1.0 / s110
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s010\n"		// c000 = c000 / s010
		"vscl.t			c100[-1:1,-1:1,-1:1], c100, s110\n"		// c100 = c100 / s110
		"vmul.t			c000, c000, c020\n"
		"vmul.t			c100, c100, c120\n"
		"vadd.t			c030, c030, c000\n"
		"vadd.t			c130, c130, c100\n"
		"sv.q			c030, 0 + %0\n"
		"sv.q			c130, 16+ %0\n"

		".set			pop\n"					// restore assember option
		: "=m"(*v)
		: "m"(*e)
	);
	//v->pos = e->pos;
	//v->vel = e->vel;
	//triVec4Add3( &v->pos, &v->pos, triVec4Mul3( &randVec, triVec4Rndn3( &randVec ), &e->posRand ) );
	//triVec4Add3( &v->vel, &v->vel, triVec4Mul3( &randVec, triVec4Rndn3( &randVec ), &e->velRand ) );
	
	triVec2Rnd2( (triVec2*)&randVec );
	v->lifetime = e->life*1.5 + randVec.x*e->lifeRand*0.2f;	// lifetime (a tad bit longer than the particles and less random)
	v->age = 0.0f;
	
	v->confinement = e->vortexRange + randVec.y*e->vortexRange*0.2f;
	v->dir = e->vortexDir + randVec.y*e->vortexDirRand; // (randVec.y<0?(-0.6f + randVec.y*0.4f):(0.6f + randVec.y*0.4f));

	if (e->lifetime>0)
	{
		// Have shorter particle lifes depending on emitter age
		float ef = e->age / e->lifetime;
		if (ef>1.0f) ef = 1.0f;

		v->lifetime *= (1.0f - ef*0.8f);
	}
}


// Undefine this to disable Vortex influence on particles
#define VORTICES

triVec4f force, localforce, tempvec, wind, diff, view, cross;
triQuat vortRot;

void triParticleSystemUpdate( triParticleSystem* s, triCamera* cam, triFloat dt )
{
	if (s->emitter->lifetime>0 && s->emitter->age>=s->emitter->lifetime && s->numParticles==0)
		return;

	triVec4Sub3( &s->emitter->lastpos, &s->emitter->lastpos, &s->emitter->pos );
	
	force = s->emitter->gravity;
	// Update force according to global actions
	triVec4Add3( &s->emitter->wind, &s->emitter->wind, triVec4Mul3( &wind, triVec4Rnd( &randVec ), &s->emitter->windRand ) );
	wind = s->emitter->wind;
	triVec4Add3( &force, &force, &wind );

	#ifdef DEBUG2
	triLogPrint("Update Particles:\nnumParticles: %i\nnumVortices: %i\n", s->numParticles, s->numVortices);
	#endif
	triS32 i, j;
	
	triParticleUpdateLoadEmitter( s->emitter, dt );
	for (i=0;i<s->numParticles;i++)
	{
		localforce = force;
		// Update localforce according to local actions
		
		if (triParticleUpdate( s->emitter, &s->particles[s->particleStack[i]], &localforce, dt )==0)
			triParticleRemove( s, i-- );
	}
#ifdef BENCH_CODE
	triLogPrint("Update: %i, %i\n", min, max);
	min = 99999;
	max = 0;
#endif
	
#ifdef VORTICES
	triVortex* v;
	for (j=0;j<s->numVortices;j++)
	{
		v = &s->vortices[s->vorticesStack[j]];
		//triVec4Sub(&view, &v->pos, &cam->pos);

		//v->confinement += v->dir*dt;
		triQuatFromRotate( &vortRot, v->dir*dt, &cam->dir );
		
		triParticle* p;
		for (i=0;i<s->numParticles;i++)
		{
			p = &s->particles[s->particleStack[i]];
			if (triVec4SquareLength3(triVec4Sub3(&diff, &p->pos, &v->pos)) > v->confinement)
				continue;
			
			triQuatApply( &diff, &vortRot, &diff );
			triFloat w = p->pos.w;
			triVec4Add3( &p->pos, &v->pos, &diff );
			p->pos.w = w;	// restore original rotation of particle
			
			//triVec4Scale3( &cross, triVec4Cross(&cross, &diff, &view), v->dir );
			//triVec4Add3(&cross, &cross, triVec4Scale3(&diff, &diff, -v->confinement));
			//triVec4Add3(&p->pos, &p->pos, triVec4Scale3(&cross, &cross, dt));
		}

		localforce = force;
		// Update localforce according to local actions
		if (triVortexUpdate( s->emitter, v, &localforce, dt )==0)
			triVortexRemove( s, j-- );
	}
#endif

	triFloat rate = s->emitter->rate;
	triFloat rateVortex = s->emitter->rateVortex;
	// Slow emitter rate based on emitter age
	if (s->emitter->lifetime>0)
	{
		rate *= (1.0f - s->emitter->burnout*s->emitter->age/s->emitter->lifetime);
		if (s->emitter->age >= s->emitter->lifetime)
		{
			rate = 0.0;
			rateVortex = 0.0;
		}
	}
	
	// Calculate correct emitter rate - needed for low rates at high frame rates -> rate*dt might be zero every frame!	
	if (s->emitter->lastemission>0)
	{
		triFloat t = s->emitter->age + dt - s->emitter->lastemission;
		if (t>=1.0f)
		{
			s->emitter->emitted = 0;
			s->emitter->emittedVortex = 0;
			s->emitter->lastemission = s->emitter->age;
			t -= 1.0f;
		}
		rate = rate*t - s->emitter->emitted;
		rateVortex = rateVortex*t - s->emitter->emittedVortex;
	}
	else
	{
		s->emitter->emitted = 0;
		s->emitter->emittedVortex = 0;
		s->emitter->lastemission = s->emitter->age;
		rate = rate*dt;
		rateVortex = rateVortex*dt;
	}
	rate += 0.5f;
	rateVortex += 0.5f;

	for (i=0;i<(triS32)(rate);i++)
	{
		triS32 idx = triParticleAdd( s );
		if (idx<0) break;
		
		triParticleCreate( s->emitter, &s->particles[s->particleStack[idx]] );
		s->emitter->emitted++;
	}

#ifdef VORTICES
	for (i=0;i<(triS32)(rateVortex);i++)
	{
		triS32 idx = triVortexAdd( s );
		if (idx<0) break;
		
		triVortexCreate( s->emitter, &s->vortices[s->vorticesStack[idx]] );
		s->emitter->emittedVortex++;
	}
#endif

	if (s->render==0)
	{
		if (s->textureID!=-1)
			triParticleVertexUVCListCreate( s, cam );
		else
			triParticleVertexCListCreate( s, cam );
	}
	
	s->emitter->lastpos = s->emitter->pos;
	s->emitter->age += dt;
	/*if (s->emitter->lifetime>0 && s->emitter->age>=s->emitter->lifetime)
		triParticleManagerRemove( s );*/
}



triVec4f tleft, tright, templeft, tempright, tempvec;
triQuat rot;


triS32 triParticleVertexUVCListCreate( triParticleSystem* s, triCamera* cam )
{
	triVertFastUVCf* v = (triVertFastUVCf*)s->vertices;
	triParticle* p = s->particles;
	s->vertexFormat = TRI_VERTFASTUVCF_FORMAT;
	s->numVertices = 0;

	triS32 nFrames = s->emitter->vTexFrames*s->emitter->hTexFrames;
	if (nFrames<1) nFrames = 1;
	
	triFloat iu = 127.f / s->emitter->hTexFrames;
	triFloat iv = 127.f / s->emitter->vTexFrames;
	
	triS32 i = 0;

	triVec3f min = { 1e30f, 1e30f, 1e30f }, max = { -1e30f, -1e30f, -1e30f };
	
	// TODO: Add 16bit vertex positions by using last frames bounding box as approx. scale factor
	//       Needs an additional scale vector for the particle system so it can set scale matrix appropriately
	switch (s->renderMode)
	{
		case GU_POINTS:
			s->numVertices = s->numParticles;
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				v->u = 0;
				v->v = 0;
				v->color = triColor4f2RGBA8888( &p->col );
				v->x = p->pos.x;
				v->y = p->pos.y;
				v->z = p->pos.z;
				triVec3Max( &max, &max, (triVec3*)&p->pos );
				triVec3Min( &min, &min, (triVec3*)&p->pos );
				v++;
			}
			break;
		case GU_LINES:
			s->numVertices = s->numParticles*2;
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				triU32 color = triColor4f2RGBA8888( &p->col );
				color = ((color & 0xF0) >> 4) | ((color &0xF000) >> 8) | ((color&0xF00000) >> 12) | ((color&0xF0000000) >> 16);
				v->u = 0;
				v->v = 0;
				v->color = (color & 0x0FFF) | ((color>>1)&0xF000);		// decrease this alpha to create a fade effect
				triVec3Max( &max, &max, (triVec3*)triVec4Sub3( &tempvec, &p->pos, &p->vel ) );
				triVec3Min( &min, &min, (triVec3*)&tempvec );
				v->x = p->pos.x - p->vel.x;
				v->y = p->pos.y - p->vel.y;
				v->z = p->pos.z - p->vel.z;
				v++;
				v->u = 0;
				v->v = 0;
				v->color = color;
				v->x = p->pos.x;
				v->y = p->pos.y;
				v->z = p->pos.z;
				triVec3Max( &max, &max, (triVec3*)&p->pos );
				triVec3Min( &min, &min, (triVec3*)&p->pos );
				v++;
			}
			break;
		case GU_SPRITES:
			s->numVertices = s->numParticles*2;
			triVec4Sub3( &tleft, &cam->up, &cam->right );
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				triS32 frame = 1;
				triU8 u0 = 0, v0 = 0, u1 = 127, v1 = 127;
				// If there are more than 1 frame
				if (nFrames>1)
				{
					triS32 hFrame = 0, vFrame = 0;
					// Animate the texture?
					if (s->emitter->nTexLoops>0)
					{
						if (s->emitter->fixedTexRate>0)
							frame = (triS32)(s->emitter->fixedTexRate*p->age) % nFrames;
						else
							frame = (triS32)(s->emitter->nTexLoops*nFrames*(p->age/p->lifetime)) % nFrames;
						
						hFrame = frame % s->emitter->hTexFrames;
						vFrame = frame / s->emitter->hTexFrames;
					}
					u0 = (triU8)(hFrame * iu);
					v0 = (triU8)(vFrame * iv);
					u1 = (triU8)((hFrame+1) * iu);
					v1 = (triU8)((vFrame+1) * iv);
				}
				triFloat sz = p->size * 0.5f;
				triU32 color = triColor4f2RGBA8888( &p->col );
				color = ((color & 0xF0) >> 4) | ((color &0xF000) >> 8) | ((color&0xF00000) >> 12) | ((color&0xF0000000) >> 16);
				v->u = u0;
				v->v = v0;
				v->color = color;
				triVec4Scale3( &tempvec, &tleft, sz );
				triVec3Max( &max, &max, (triVec3*)triVec4Add3( &templeft, &p->pos, &tempvec ) );
				triVec3Min( &min, &min, (triVec3*)&templeft );
				v->x = templeft.x;
				v->y = templeft.y;
				v->z = templeft.z;
				v++;
				v->u = u1;
				v->v = v1;
				v->color = color;
				triVec3Min( &min, &min, (triVec3*)triVec4Sub3( &tempright, &p->pos, &tempvec ) );
				triVec3Max( &max, &max, (triVec3*)&tempright );
				v->x = tempright.x;
				v->y = tempright.y;
				v->z = tempright.z;
				v++;
			}
			break;
		case GU_TRIANGLES:
		case GU_TRIANGLE_FAN:
		case GU_TRIANGLE_STRIP:
			s->renderMode = GU_TRIANGLES;
			s->numVertices = s->numParticles*6;
			triVec4Sub3( &tleft, &cam->up, &cam->right );
			triVec4Add3( &tright, &cam->up,&cam->right );
			// FIXME: Add quaternion based rotation for non-billboard particles
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				triFloat sz = p->size * 0.5f;
				triU32 color = triColor4f2RGBA8888( &p->col );
				color = ((color & 0xF0) >> 4) | ((color &0xF000) >> 8) | ((color&0xF00000) >> 12) | ((color&0xF0000000) >> 16);
				
				triS32 frame = 1;
				triU8 u0 = 0, v0 = 0, u1 = 127, v1 = 127;
				// If there are more than 1 frame
				if (nFrames>1)
				{
					triS32 hFrame = 0, vFrame = 0;
					// Animate the texture?
					if (s->emitter->nTexLoops>0)
					{
						if (s->emitter->fixedTexRate>0)
							frame = (triS32)(s->emitter->fixedTexRate*p->age) % nFrames;
						else
							frame = (triS32)(s->emitter->nTexLoops*nFrames*(p->age/p->lifetime)) % nFrames;
						
						hFrame = frame % s->emitter->hTexFrames;
						vFrame = frame / s->emitter->hTexFrames;
					}
					u0 = (triU8)(hFrame * iu);
					v0 = (triU8)(vFrame * iv);
					u1 = (triU8)((hFrame+1) * iu);
					v1 = (triU8)((vFrame+1) * iv);
				}
				
				if (p->pos.w!=0.0)
				{
					triQuatFromRotate( &rot, p->pos.w, &cam->dir );
					triQuatApply( &templeft, &rot, triVec4Scale3( &templeft, &tleft, sz ) );
					triQuatApply( &tempright, &rot, triVec4Scale3( &tempright, &tright, sz ) );
					tempvec = templeft;
					v->u = u0;
					v->v = v0;
					v->color = color;
					v->x = p->pos.x + templeft.x;
					v->y = p->pos.y + templeft.y;
					v->z = p->pos.z + templeft.z;
					v++;
					v->u = u1;
					v->v = v0;
					v->color = color;
					v->x = p->pos.x + tempright.x;
					v->y = p->pos.y + tempright.y;
					v->z = p->pos.z + tempright.z;
					v++;
					triVec3Max( &max, &max, (triVec3*)triVec4Add3( &templeft, &p->pos, &templeft ) );
					triVec3Min( &min, &min, (triVec3*)&templeft );
					triVec3Max( &max, &max, (triVec3*)triVec4Add3( &tempright, &p->pos, &tempright ) );
					triVec3Min( &min, &min, (triVec3*)&tempright );
					v->u = u1;
					v->v = v1;
					v->color = color;
					triQuatApply( &templeft, &rot, triVec4Scale3( &templeft, &tleft, -sz ) );
					triQuatApply( &tempright, &rot, triVec4Scale3( &tempright, &tright, -sz ) );
					v->x = p->pos.x + templeft.x;
					v->y = p->pos.y + templeft.y;
					v->z = p->pos.z + templeft.z;
					v++;
					
					
					v->u = u1;
					v->v = v1;
					v->color = color;
					v->x = p->pos.x + templeft.x;
					v->y = p->pos.y + templeft.y;
					v->z = p->pos.z + templeft.z;
					v++;
					v->u = u0;
					v->v = v1;
					v->color = color;
					v->x = p->pos.x + tempright.x;
					v->y = p->pos.y + tempright.y;
					v->z = p->pos.z + tempright.z;
					v++;
					v->u = u0;
					v->v = v0;
					v->color = color;
					v->x = p->pos.x + tempvec.x;
					v->y = p->pos.y + tempvec.y;
					v->z = p->pos.z + tempvec.z;
					v++;
					
					triVec3Max( &max, &max, (triVec3*)triVec4Add3( &templeft, &p->pos, &templeft ) );
					triVec3Min( &min, &min, (triVec3*)&templeft );
					triVec3Max( &max, &max, (triVec3*)triVec4Add3( &tempright, &p->pos, &tempright ) );
					triVec3Min( &min, &min, (triVec3*)&tempright );
				}
				else
				{
					v->u = u0;
					v->v = v0;
					v->color = color;
					v->x = p->pos.x + tleft.x*sz;
					v->y = p->pos.y + tleft.y*sz;
					v->z = p->pos.z + tleft.z*sz;
					triVec3Max( &max, &max, (triVec3*)&v->x );	// Hacky way of getting the vector - won't work with 16bit vertex positions any more
					triVec3Min( &min, &min, (triVec3*)&v->x );
					v++;
					v->u = u1;
					v->v = v0;
					v->color = color;
					v->x = p->pos.x + tright.x*sz;
					v->y = p->pos.y + tright.y*sz;
					v->z = p->pos.z + tright.z*sz;
					triVec3Max( &max, &max, (triVec3*)&v->x );
					triVec3Min( &min, &min, (triVec3*)&v->x );
					v++;
					v->u = u1;
					v->v = v1;
					v->color = color;
					v->x = p->pos.x - tleft.x*sz;
					v->y = p->pos.y - tleft.y*sz;
					v->z = p->pos.z - tleft.z*sz;
					triVec3Max( &max, &max, (triVec3*)&v->x );
					triVec3Min( &min, &min, (triVec3*)&v->x );
					v++;
	
					v->u = u1;
					v->v = v1;
					v->color = color;
					v->x = p->pos.x - tleft.x*sz;
					v->y = p->pos.y - tleft.y*sz;
					v->z = p->pos.z - tleft.z*sz;
					v++;
					v->u = u0;
					v->v = v1;
					v->color = color;
					v->x = p->pos.x - tright.x*sz;
					v->y = p->pos.y - tright.y*sz;
					v->z = p->pos.z - tright.z*sz;
					triVec3Max( &max, &max, (triVec3*)&v->x );
					triVec3Min( &min, &min, (triVec3*)&v->x );
					v++;
					v->u = u0;
					v->v = v0;
					v->color = color;
					v->x = p->pos.x + tleft.x*sz;
					v->y = p->pos.y + tleft.y*sz;
					v->z = p->pos.z + tleft.z*sz;
					v++;
				}
			}
			break;
	}
	s->boundingBox[0].x = min.x;
	s->boundingBox[0].y = min.y;
	s->boundingBox[0].z = min.z;

	s->boundingBox[1].x = max.x;
	s->boundingBox[1].y = min.y;
	s->boundingBox[1].z = min.z;

	s->boundingBox[2].x = max.x;
	s->boundingBox[2].y = min.y;
	s->boundingBox[2].z = max.z;

	s->boundingBox[3].x = min.x;
	s->boundingBox[3].y = min.y;
	s->boundingBox[3].z = max.z;


	s->boundingBox[4].x = min.x;
	s->boundingBox[4].y = max.y;
	s->boundingBox[4].z = min.z;

	s->boundingBox[5].x = max.x;
	s->boundingBox[5].y = max.y;
	s->boundingBox[5].z = min.z;

	s->boundingBox[6].x = max.x;
	s->boundingBox[6].y = max.y;
	s->boundingBox[6].z = max.z;

	s->boundingBox[7].x = min.x;
	s->boundingBox[7].y = max.y;
	s->boundingBox[7].z = max.z;
	
	sceKernelDcacheWritebackAll();
	return 1;
}


triS32 triParticleVertexCListCreate( triParticleSystem* s, triCamera* cam )
{
	triVertC* v = (triVertC*)s->vertices;
	triParticle* p = s->particles;
	s->vertexFormat = TRI_VERTC_FORMAT;
	s->numVertices = 0;
	
	triS32 i = 0;

	switch (s->renderMode)
	{
		case GU_POINTS:
			s->numVertices = s->numParticles;
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				v->color = triColor4f2RGBA8888( &p->col );
				v->x = p->pos.x;
				v->y = p->pos.y;
				v->z = p->pos.z;
				v++;
			}
			break;
		case GU_LINES:
			s->numVertices = s->numParticles*2;
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				triU32 color = triColor4f2RGBA8888( &p->col );
				v->color = (color & 0x00FFFFFF) | ((color>>2)&0xFF000000);		// decrease this alpha to create a fade effect
				v->x = p->pos.x - p->vel.x;
				v->y = p->pos.y - p->vel.y;
				v->z = p->pos.z - p->vel.z;
				v++;
				v->color = color;
				v->x = p->pos.x;
				v->y = p->pos.y;
				v->z = p->pos.z;
				v++;
			}
			break;
		case GU_SPRITES:
			s->numVertices = s->numParticles*2;
			triVec4Sub3( &tleft, &cam->up, &cam->right );
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				triFloat sz = p->size * 0.5f;
				triU32 color = triColor4f2RGBA8888( &p->col );
				v->color = color;
				v->x = p->pos.x + tleft.x*sz;
				v->y = p->pos.y + tleft.y*sz;
				v->z = p->pos.z + tleft.z*sz;
				v++;
				v->color = color;
				v->x = p->pos.x - tleft.x*sz;
				v->y = p->pos.y - tleft.y*sz;
				v->z = p->pos.z - tleft.z*sz;
				v++;
			}
			break;
		case GU_TRIANGLES:
		case GU_TRIANGLE_FAN:
		case GU_TRIANGLE_STRIP:
			s->renderMode = GU_TRIANGLES;
			s->numVertices = s->numParticles*6;
			
			triVec4Sub3( &tleft, &cam->up, &cam->right );
			triVec4Add3( &tright, &cam->up,&cam->right );
			for (;i<s->numParticles;i++)
			{
				p = &s->particles[s->particleStack[i]];
				triFloat sz = p->size * 0.5f;
				triU32 color = triColor4f2RGBA8888( &p->col );
				
				v->color = color;
				v->x = p->pos.x + tleft.x*sz;
				v->y = p->pos.y + tleft.y*sz;
				v->z = p->pos.z + tleft.z*sz;
				v++;
				v->color = color;
				v->x = p->pos.x + tright.x*sz;
				v->y = p->pos.y + tright.y*sz;
				v->z = p->pos.z + tright.z*sz;
				v++;
				v->color = color;
				v->x = p->pos.x - tleft.x*sz;
				v->y = p->pos.y - tleft.y*sz;
				v->z = p->pos.z - tleft.z*sz;
				v++;
				
				v->color = color;
				v->x = p->pos.x - tleft.x*sz;
				v->y = p->pos.y - tleft.y*sz;
				v->z = p->pos.z - tleft.z*sz;
				v++;
				v->color = color;
				v->x = p->pos.x - tright.x*sz;
				v->y = p->pos.y - tright.y*sz;
				v->z = p->pos.z - tright.z*sz;
				v++;
				v->color = color;
				v->x = p->pos.x + tleft.x*sz;
				v->y = p->pos.y + tleft.y*sz;
				v->z = p->pos.z + tleft.z*sz;
				v++;
			}
			break;
	}
	return 1;
}


void triParticleEmitterConstructor( triParticleEmitter *e, triS32 emitterType )
{
	triLogPrint("Initializing hardcoded emitter type %i <%x>\n", emitterType, emitterType);
	// Init to default values
	triVec4Set( &e->pos, 0.0f, 0.0f, 0.0f, 0.0f );
	triVec4Set( &e->posRand, 0.0f, 0.0f, 0.0f, 0.0f );
	triVec4Set( &e->vel, 0.0f, 0.0f, 0.0f, 0.0f );
	triVec4Set( &e->velRand, 0.1f, 0.1f, 0.1f, 1.0f );
	triColor4Set( &e->cols[0], 1.0f, 1.0f, 1.0f, 1.0f );
	e->numCols = 1;
	e->size = 1.0f;
	e->life = 5.0f;
	e->lifeRand = 0.2f;
	e->vTexFrames = 1;
	e->hTexFrames = 1;
	e->nTexLoops = 0;
	e->fixedTexRate = 0;
	e->min = 0;
	e->max = 256;
	e->rate = e->max/e->life;
	e->lifetime = 0;	// Eternal
	
	// Default no wind and no vortices
	triVec4Set( &e->wind, 0.0f, 0.0f, 0.0f, 0.0f );
	triVec4Set( &e->windRand, 0.0f, 0.0f, 0.0f, 0.0f );
	triVec4Set( &e->gravity, 0.0f, 0.0f, 0.0f, 0.0f );
	e->rateVortex = 0;
	e->minVortex = 0;
	e->maxVortex = 0;
	e->growth = 0.0f;
	e->sizeRand = 0.0f;
	e->burnout = 0.0f;
	e->binding = 0.0f;
	e->loosen = 0.0f;
	e->friction = 0.0f;
	e->vortexRange = 0.66f;
	e->glitter = 0.0f;
	e->glitterSpeed = 3.0f;
	
	switch(emitterType)
	{
		case TRI_EMITTER_FIRE:
			triVec4Set( &e->pos, 0.6f, 0.2f, 0.6f, 0.0f );
			triVec4Set( &e->posRand, 0.4f, 0.2f, 0.4f, 0.0f );

			triVec4Set( &e->vel, 0.0f, 0.4f, 0.0f, 0.0f );
			triVec4Set( &e->velRand, 0.05f, 0.05f, 0.05f, 0.0f );

			triColor4Set( &e->cols[0], 0.0f, 0.1f, 0.8f, 0.3f );	// Start at a dark blue
			triColor4Set( &e->cols[1], 0.6f, 0.1f, 0.0f, 1.0f );	// go to light blue
			triColor4Set( &e->cols[2], 0.8f, 0.1f, 0.0f, 1.0f );	// to red
			triColor4Set( &e->cols[3], 0.8f, 0.1f, 0.0f, 1.0f );	// to red
			triColor4Set( &e->cols[4], 0.8f, 0.1f, 0.0f, 1.0f );	// to red
			triColor4Set( &e->cols[5], 0.8f, 0.8f, 0.0f, 0.8f );	// to yellow
			triColor4Set( &e->cols[6], 0.8f, 0.8f, 0.0f, 0.0f );	// fade out
			e->numCols = 7;
			
			e->growth = -0.7f/6.0f;

			e->size = 0.9f;
			e->sizeRand = 0.1f;
			
			e->life = 6.0f;

			e->friction = 0.01f;

			e->min = 0;
			e->max = 256;
			e->rate = 16;
			e->lifetime = 0;	// Eternal
			break;

		case TRI_EMITTER_EXPLOSION:
			triVec4Set( &e->pos, 0.0f, 0.0f, 0.0f, 0.0f );
			triVec4Set( &e->posRand, 0.0f, 0.0f, 0.0f, 0.0f );

			triVec4Set( &e->vel, 0.0f, 0.0f, 0.0f, 0.0f );
			triVec4Set( &e->velRand, 5.0f, 5.0f, 5.0f, 0.0f );

			triColor4Set( &e->cols[0], 0.86f, 0.57f, 0.39f, 1.0 );
			triColor4Set( &e->cols[1], 0.86f, 0.57f, 0.39f, 1.0 );
			triColor4Set( &e->cols[2], 0.86f, 0.57f, 0.39f, 0.0 );
			e->numCols = 3;

			e->growth = 8.0f;		// size of particle is 9 at end of life

			e->size = 1.0f;
			e->sizeRand = 0.0f;		// no random size

			e->life = 1.0f;			// Particles live for 1s +- 20%

			e->friction = 0.01f;		// slight air friction

			e->min = 0;
			e->max = 128;
			//e->maxVortex = 16;
			
			e->lifetime = 0.25f;		// lifetime of the emitter is 0.25 second - enough to blow out all particles and die before any respawn
			e->rate = e->max / e->lifetime;	// blow out all particles during the given lifetime
			//e->rateVortex = e->maxVortex / e->lifetime;
			break;
		
		case TRI_EMITTER_SPRINKLE:
			triVec4Set( &e->pos, 0.0f, 0.0f, 0.0f, 0.0f );
			triVec4Set( &e->posRand, 0.001f, 0.001f, 0.001f, 0.0f );

			triVec4Set( &e->vel, 0.0f, 2.0f, 0.0f, 0.0f );
			triVec4Set( &e->velRand, 0.2f, 0.25f, 0.2f, 0.0f );

			triVec4Set( &e->gravity, 0.0f, -0.981f, 0.0f, 0.0f );

			triColor4Set( &e->cols[0], 0.9f, 0.9f, 1.0f, 1.0f );
			triColor4Set( &e->cols[1], 0.9f, 0.9f, 1.0f, 1.0f );
			triColor4Set( &e->cols[2], 0.9f, 0.9f, 1.0f, 0.0f );
			e->numCols = 3;

			e->growth = -0.01f;		// size of particle is at end of life

			e->size = 0.6f;
			e->sizeRand = 0.1f;

			e->life = 5.0f;			// Particles live for 5s +- 20%

			e->friction = 0.01f;		// slight air friction

			e->glitter = 0.5f;
			e->glitterSpeed = 24.0f;	// 24/5 ~ 5 glitters per second

			e->min = 0;
			e->max = 512;
			e->rate = 32;
			e->lifetime = 0.0f;
			break;

		case TRI_EMITTER_SMOKE:
			triVec4Set( &e->pos, 0.0f, 0.0f, 0.0f, 0.0f );
			triVec4Set( &e->posRand, 0.5f, 0.5f, 0.5f, 8.0f );

			triVec4Set( &e->vel, 0.0f, 0.4f, 0.0f, 0.0f );
			triVec4Set( &e->velRand, 0.05f, 0.05f, 0.05f, 6.0f );

			triColor4Set( &e->cols[0], 0.3, 0.3, 0.3, 0.2 );	// fade in
			triColor4Set( &e->cols[1], 0.3, 0.3, 0.3, 1.0 );
			triColor4Set( &e->cols[2], 0.3, 0.3, 0.3, 1.0 );
			triColor4Set( &e->cols[3], 0.3, 0.3, 0.3, 0.0 );	// fade out
			e->numCols = 4;
			
			e->growth = -0.5f/6.0f;

			e->size = 1.2f;
			e->sizeRand = 0.2f;
			
			e->burnout = 0.7f;		// Emitter burns out with life
			
			e->life = 6.0f;

			e->friction = 0.01f;

			e->min = 0;
			e->max = 512;
			e->minVortex = 0;
			e->maxVortex = 100;
			e->rate = 32;
			e->rateVortex = 10;
			e->vortexRange = 0.33f;
			e->lifetime = 30.0f;
			break;
	}
	e->lastpos = e->pos;
	e->age = 0;
	e->lastemission = 0;
	e->emitted = 0;
	e->emittedVortex = 0;
}





void triParticleManagerUpdate( triCamera* cam, triFloat dt )
{
	triParticleSystem* s = TRI_PARTMAN.systems;
	while (s!=0)
	{
		if (s->updateFlag^=1)
		{
			triParticleSystemUpdate( s, cam, dt+TRI_PARTMAN.dt );
			TRI_PARTMAN.dt = 0;
		}
		else
			TRI_PARTMAN.dt = dt;
		s = s->next;
	}
}


void triParticleManagerRender()
{
	triParticleSystem* s = TRI_PARTMAN.systems;
	TRI_PARTMAN.numParticles = 0;
	TRI_PARTMAN.numVertices = 0;
	while (s!=0)
	{
		triParticleSystemRender( s );
		TRI_PARTMAN.numParticles += s->numParticles;
		TRI_PARTMAN.numVertices += s->numVertices;
		s = s->next;
	}
}


void triParticleManagerUpdateRender( triCamera* cam, triFloat dt )
{
	triParticleSystem* s = TRI_PARTMAN.systems;
	TRI_PARTMAN.numParticles = 0;
	TRI_PARTMAN.numVertices = 0;
	while (s!=0)
	{
		if (s->updateFlag^=1)
		{
			triParticleSystemUpdate( s, cam, dt+TRI_PARTMAN.dt );
			TRI_PARTMAN.dt = 0;
		}
		else
			TRI_PARTMAN.dt = dt;
		triParticleSystemRender( s );
		TRI_PARTMAN.numParticles += s->numParticles;
		TRI_PARTMAN.numVertices += s->numVertices;
		s = s->next;
	}
}


void triParticleManagerRemove( triS32 id )
{
	triParticleSystem* s = TRI_PARTMAN.systems;
	while (s!=0)
	{
		if (s->ID == id)
		{
			triFree( s->emitter );
			triParticleSystemFree( s );
			triFree( s );
			TRI_PARTMAN.numSystems--;
			return;
		}
		s = s->next;
	}
}


triParticleSystem* triParticleManagerGet( triS32 id )
{
	triParticleSystem* s = TRI_PARTMAN.systems;
	while (s!=0)
	{
		if (s->ID == id)
		{
			return s;
		}
		s = s->next;
	}
	return(0);
}


void triParticleManagerDestroy()
{
	triParticleSystem* s = TRI_PARTMAN.systems;
	triParticleSystem* next = 0;
	while (s!=0)
	{
		next = s->next;
		triFree( s->emitter );
		triTextureUnload( s->textureID );
		triParticleSystemFree( s );
		triFree( s );
		s = next;
	}
	TRI_PARTMAN.numSystems = 0;
	TRI_PARTMAN.systems = 0;
}


triS32 triParticleManagerAdd( triParticleSystem* p, triParticleEmitter* e )
{
	triParticleSystemInitialize( p, e );
	TRI_PARTMAN.numSystems++;
	p->next = 0;
	TRI_PARTMAN.dt = 0;
	if (TRI_PARTMAN.systems==0)
		TRI_PARTMAN.systems = p;
	else
	{
		triParticleSystem* s = TRI_PARTMAN.systems;
		while (s->next!=0)
			s = s->next;
		s->next = p;
	}
	p->ID = ++TRI_PARTMAN.idCounter;
	p->updateFlag &= p->ID;
	triLogPrint( "Added particle system (%p) and emitter (%p) as ID %i: %i systems in total\n", p, e, p->ID, TRI_PARTMAN.numSystems );
	return p->ID;
}


triS32 triParticleManagerLoadScript( triChar* name )
{
	triLogPrint( "Loading particle script from %s\n", name );
	
	FILE* fp = fopen( name, "r" );
	
	if (fp==0) return(0);
	
	triParticleSystem* p = triMalloc(sizeof(triParticleSystem));
	if (p==0) return(0);
	
	triParticleSystemConstructor(p);
	triParticleEmitter* e = triMalloc(sizeof(triParticleEmitter));
	if (e==0) return(0);
	
	triChar buffer[512];
	triChar* tbuffer;
	triChar temp[64];
	triS32 i;
	while (!feof(fp))
	{
		if (fgets( buffer, 512, fp )==0) break;
		if ((i=strlen(buffer))>0)
			while(i>0)
			{
				// Strip whitespaces from end of line
				if ((buffer[i-1]=='\n') || (buffer[i-1]=='\r') || (buffer[i-1]==' ') || (buffer[i-1]=='\t')) buffer[i-1]=0;
				else break;
				i--;
			}
		if (buffer[0]==0) continue; // Was a blank line
		
		if (buffer[0]==';' || (buffer[0]=='/' && buffer[1]=='/')) continue;	// ignore comments
		if (buffer[0]=='[') continue;	// all identifiers are unique, so no blocks need to be treated special

		
		triLogPrint("Parsing line> '%s'\n", buffer);
		if (sscanf( buffer, "%s: ", temp )==1)
		{
			if (stricmp(temp,"pos:")==0)
				sscanf( buffer+strlen(temp)+1, "(%f, %f, %f, %f)", &e->pos.x, &e->pos.y, &e->pos.z, &e->pos.w );
			else
			if (stricmp(temp,"posRand:")==0)
				sscanf( buffer+strlen(temp)+1, "(%f, %f, %f, %f)", &e->posRand.x, &e->posRand.y, &e->posRand.z, &e->posRand.w );
			else
			if (stricmp(temp,"vel:")==0)
				sscanf( buffer+strlen(temp)+1, "(%f, %f, %f, %f)", &e->vel.x, &e->vel.y, &e->vel.z, &e->vel.w );
			else
			if (stricmp(temp,"velRand:")==0)
				sscanf( buffer+strlen(temp)+1, "(%f, %f, %f, %f)", &e->velRand.x, &e->velRand.y, &e->velRand.z, &e->velRand.w );
			else
			if (stricmp(temp,"gravity:")==0)
				sscanf( buffer+strlen(temp)+1, "(%f, %f, %f, %f)", &e->gravity.x, &e->gravity.y, &e->gravity.z, &e->gravity.w );
			else
			if (stricmp(temp,"wind:")==0)
				sscanf( buffer+strlen(temp)+1, "(%f, %f, %f, %f)", &e->wind.x, &e->wind.y, &e->wind.z, &e->wind.w );
			else
			if (stricmp(temp,"windRand:")==0)
				sscanf( buffer+strlen(temp)+1, "(%f, %f, %f, %f)", &e->windRand.x, &e->windRand.y, &e->windRand.z, &e->windRand.w );
			else
			if (stricmp(temp,"numCols:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->numCols );
			else
			if (stricmp(temp,"cols:")==0)
			{
				tbuffer = buffer + strlen(temp) + 2;
				for (i=0;i<e->numCols;i++)
				{
					sscanf( tbuffer, "(%f, %f, %f, %f)", &e->cols[i].r, &e->cols[i].g, &e->cols[i].b, &e->cols[i].a );
					tbuffer = strchr( tbuffer+1, '(' );
				}
			}
			else
			if (stricmp(temp,"size:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->size );
			else
			if (stricmp(temp,"sizeRand:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->sizeRand );
			else
			if (stricmp(temp,"growth:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->growth );
			else
			if (stricmp(temp,"glitter:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->glitter );
			else
			if (stricmp(temp,"glitterSpeed:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->glitterSpeed );
			else
			if (stricmp(temp,"life:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->life );
			else
			if (stricmp(temp,"lifeRand:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->lifeRand );
			else
			if (stricmp(temp,"vTexFrames:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->vTexFrames );
			else
			if (stricmp(temp,"hTexFrames:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->hTexFrames );
			else
			if (stricmp(temp,"nTexLoops:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->nTexLoops );
			else
			if (stricmp(temp,"fixedTexRate:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->fixedTexRate );
			else
			if (stricmp(temp,"burnout:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->burnout );
			else
			if (stricmp(temp,"friction:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->friction );
			else
			if (stricmp(temp,"loosen:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->loosen );
			else
			if (stricmp(temp,"max:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->max );
			else
			if (stricmp(temp,"maxVortex:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->maxVortex );
			else
			if (stricmp(temp,"vortexRange:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->vortexRange );
			else
			if (stricmp(temp,"rate:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->rate );
			else
			if (stricmp(temp,"rateVortex:")==0)
				sscanf( buffer+strlen(temp)+1, "%d", (int*)&e->rateVortex );
			else
			if (stricmp(temp,"lifetime:")==0)
				sscanf( buffer+strlen(temp)+1, "%f", &e->lifetime );
			else
			if (stricmp(temp,"op:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				if (stricmp(temp,"GU_ADD")==0)
					p->blendMode.op = GU_ADD;
				else
				if (stricmp(temp,"GU_SUBTRACT")==0)
					p->blendMode.op = GU_SUBTRACT;
				else
				if (stricmp(temp,"GU_REVERSE_SUBTRACT")==0)
					p->blendMode.op = GU_REVERSE_SUBTRACT;
				else
				if (stricmp(temp,"GU_MIN")==0)
					p->blendMode.op = GU_MIN;
				else
				if (stricmp(temp,"GU_MAX")==0)
					p->blendMode.op = GU_MAX;
				else
				if (stricmp(temp,"GU_ABS")==0)
					p->blendMode.op = GU_ABS;
			}
			else
			if (stricmp(temp,"src_op:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				if (stricmp(temp,"GU_SRC_COLOR")==0)
					p->blendMode.src_op = GU_SRC_COLOR;
				else
				if (stricmp(temp,"GU_ONE_MINUS_SRC_COLOR")==0)
					p->blendMode.src_op = GU_ONE_MINUS_SRC_COLOR;
				else
				if (stricmp(temp,"GU_SRC_ALPHA")==0)
					p->blendMode.src_op = GU_SRC_ALPHA;
				else
				if (stricmp(temp,"GU_ONE_MINUS_SRC_ALPHA")==0)
					p->blendMode.src_op = GU_ONE_MINUS_SRC_ALPHA;
				else
				if (stricmp(temp,"GU_DST_COLOR")==0)
					p->blendMode.src_op = GU_DST_COLOR;
				else
				if (stricmp(temp,"GU_ONE_MINUS_DST_COLOR")==0)
					p->blendMode.src_op = GU_ONE_MINUS_DST_COLOR;
				else
				if (stricmp(temp,"GU_DST_ALPHA")==0)
					p->blendMode.src_op = GU_DST_ALPHA;
				else
				if (stricmp(temp,"GU_ONE_MINUS_DST_ALPHA")==0)
					p->blendMode.src_op = GU_ONE_MINUS_DST_ALPHA;
				else
				if (stricmp(temp,"GU_FIX")==0)
					p->blendMode.src_op = GU_FIX;
			}
			else
			if (stricmp(temp,"dst_op:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				if (stricmp(temp,"GU_SRC_COLOR")==0)
					p->blendMode.dst_op = GU_SRC_COLOR;
				else
				if (stricmp(temp,"GU_ONE_MINUS_SRC_COLOR")==0)
					p->blendMode.dst_op = GU_ONE_MINUS_SRC_COLOR;
				else
				if (stricmp(temp,"GU_SRC_ALPHA")==0)
					p->blendMode.dst_op = GU_SRC_ALPHA;
				else
				if (stricmp(temp,"GU_ONE_MINUS_SRC_ALPHA")==0)
					p->blendMode.dst_op = GU_ONE_MINUS_SRC_ALPHA;
				else
				if (stricmp(temp,"GU_DST_COLOR")==0)
					p->blendMode.dst_op = GU_DST_COLOR;
				else
				if (stricmp(temp,"GU_ONE_MINUS_DST_COLOR")==0)
					p->blendMode.dst_op = GU_ONE_MINUS_DST_COLOR;
				else
				if (stricmp(temp,"GU_DST_ALPHA")==0)
					p->blendMode.dst_op = GU_DST_ALPHA;
				else
				if (stricmp(temp,"GU_ONE_MINUS_DST_ALPHA")==0)
					p->blendMode.dst_op = GU_ONE_MINUS_DST_ALPHA;
				else
				if (stricmp(temp,"GU_FIX")==0)
					p->blendMode.dst_op = GU_FIX;
			}
			else
			if (stricmp(temp,"src_fix:")==0)
				sscanf( buffer+strlen(temp)+1, "%X", (unsigned int*)&p->blendMode.src_fix );
			else
			if (stricmp(temp,"dst_fix:")==0)
				sscanf( buffer+strlen(temp)+1, "%X", (unsigned int*)&p->blendMode.dst_fix );
			else
			if (stricmp(temp,"emitter:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				if (stricmp(temp,"FIRE")==0)
					triParticleEmitterConstructor( e, TRI_EMITTER_FIRE );
				else
				if (stricmp(temp,"EXPLOSION")==0)
					triParticleEmitterConstructor( e, TRI_EMITTER_EXPLOSION );
				else
				if (stricmp(temp,"SPRINKLE")==0)
					triParticleEmitterConstructor( e, TRI_EMITTER_SPRINKLE );
				else
				if (stricmp(temp,"WATERFALL")==0)
					triParticleEmitterConstructor( e, TRI_EMITTER_WATERFALL );
				else
				if (stricmp(temp,"SMOKE")==0)
					triParticleEmitterConstructor( e, TRI_EMITTER_SMOKE );
			}
			else
			if (stricmp(temp,"texture:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				p->textureID = triTextureLoad( temp );
				triLogPrint("Loaded texture '%s' as ID %i\n", temp, p->textureID );
			}
			else
			if (stricmp(temp,"texMode:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				if (stricmp(temp,"GU_TFX_MODULATE")==0)
					p->texMode = GU_TFX_MODULATE;
				else
				if (stricmp(temp,"GU_TFX_DECAL")==0)
					p->texMode = GU_TFX_DECAL;
				else
				if (stricmp(temp,"GU_TFX_BLEND")==0)
					p->texMode = GU_TFX_BLEND;
				else
				if (stricmp(temp,"GU_TFX_REPLACE")==0)
					p->texMode = GU_TFX_REPLACE;
				else
				if (stricmp(temp,"GU_TFX_ADD")==0)
					p->texMode = GU_TFX_ADD;
			}
			else
			if (stricmp(temp,"renderMode:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				if (stricmp(temp,"GU_POINTS")==0)
					p->renderMode = GU_POINTS;
				else
				if (stricmp(temp,"GU_LINES")==0)
					p->renderMode = GU_LINES;
				else
				if (stricmp(temp,"GU_SPRITES")==0)
					p->renderMode = GU_SPRITES;
				else
				if (stricmp(temp,"GU_TRIANGLES")==0)
					p->renderMode = GU_TRIANGLES;
			}
			else
			if (stricmp(temp,"texColor:")==0)
				sscanf( buffer+strlen(temp)+1, "%X", (unsigned int*)&p->texColor );
			else
			if (stricmp(temp,"blendMode:")==0)
			{
				sscanf( buffer+strlen(temp)+1, "%s", temp );
				if (stricmp(temp,"ALPHA")==0)
					p->blendMode = TRI_BLEND_MODE_ALPHA;
				else
				if (stricmp(temp,"ADD")==0)
					p->blendMode = TRI_BLEND_MODE_ADD;
				else
				if (stricmp(temp,"GLENZ")==0)
					p->blendMode = TRI_BLEND_MODE_GLENZ;
				else
				if (stricmp(temp,"ALPHA_ADD")==0)
					p->blendMode = TRI_BLEND_MODE_ALPHA_ADD;
				else
				if (stricmp(temp,"SUB")==0)
					p->blendMode = TRI_BLEND_MODE_SUB;
				else
				if (stricmp(temp,"ALPHA_SUB")==0)
					p->blendMode = TRI_BLEND_MODE_ALPHA_SUB;
			}
			else
			if (temp[0]!=0)
				triLogPrint("Could not parse command '%s'\n", temp);
		}
		else
			triLogPrint("Could not parse line '%s'\n", buffer);
	}
	fclose(fp);
	
	e->lastpos = e->pos;
	e->age = 0;
	e->lastemission = 0;
	e->emitted = 0;
	e->emittedVortex = 0;
	
	return triParticleManagerAdd( p, e );
}
