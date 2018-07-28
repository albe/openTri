/*
 * triCamera.c: Code for Camera
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
 
#include <pspgum.h>
#include "triMemory.h"
#include "triCamera.h"
#include "triVMath_vfpu.h"


triVec4f	triOrigin = { 0.f, 0.f, 0.f, 0.f };

// Create a default camera looking into positive z direction at position (x,y,z)
triCamera* triCameraCreate( triFloat x, triFloat y, triFloat z )
{
	triCamera* cam = triMalloc(sizeof(triCamera));
	if (cam==0) return(0);
	
	cam->dir.x = 0;
	cam->dir.y = 0;
	cam->dir.z = 1;
	cam->dir.w = 0;
	cam->up.x = 0;
	cam->up.y = 1;
	cam->up.z = 0;
	cam->up.w = 0;
	cam->right.x = 1;
	cam->right.y = 0;
	cam->right.z = 0;
	cam->right.w = 0;
	cam->pos.x = x;
	cam->pos.y = y;
	cam->pos.z = z;
	cam->pos.w = 0;

	cam->dirty = 1;
	return cam;
}



void triCameraMove( triCamera* cam, triFloat x, triFloat y, triFloat z )
{
	cam->pos.x += x;
	cam->pos.y += y;
	cam->pos.z += z;
	cam->dirty = 1;
}


static inline void triCameraDoRotation( triCamera* cam, int pos )
{
	triQuatApply( &cam->dir, &cam->rot, &cam->dir );
	triQuatApply( &cam->up, &cam->rot, &cam->up );
	triQuatApply( &cam->right, &cam->rot, &cam->right );
	if (pos)
		triQuatApply( &cam->pos, &cam->rot, &cam->pos );
	#ifdef LDEBUG
	triLogPrint("Camera>\n");
	triLogPrint("dir: <%.2f, %.2f, %.2f>\n", cam->dir.x, cam->dir.y, cam->dir.z );
	triLogPrint("right: <%.2f, %.2f, %.2f>\n", cam->right.x, cam->right.y, cam->right.z );
	triLogPrint("up: <%.2f, %.2f, %.2f>\n", cam->up.x, cam->up.y, cam->up.z );
	triLogPrint("pos: <%.2f, %.2f, %.2f>\n", cam->pos.x, cam->pos.y, cam->pos.z );
	#endif
	cam->dirty = 1;
}


// rotate camera angle degrees around the axis (x,y,z)
void triCameraRotate( triCamera* cam, triFloat angle, triVec4f* axis )
{
	triQuatFromRotate( &cam->rot, -angle, axis );
	#ifdef LDEBUG
	triLogPrint("Camera>\n");
	triLogPrint("rot: <%.2f, %.2f, %.2f, %.2f>\n", cam->rot.x, cam->rot.y, cam->rot.z, cam->rot.w );
	#endif
	triCameraDoRotation( cam, 0 );
}


// rotate camera around center 
void triCameraRotateAbout( triCamera* cam, triFloat angle, triVec4f* axis, triVec4f* center )
{
	//triQuatFromRotate( &cam->rot, angle, axis );
	triVec4Sub3( &cam->pos, &cam->pos, center );
	triQuatFromRotate( &cam->rot, -angle, axis );
	triCameraDoRotation( cam, 1 );
	//triQuatApply( &cam->pos, &cam->rot, &cam->pos );
	triVec4Add3( &cam->pos, &cam->pos, center );
	//triCameraDoRotation( cam );
}


void triCameraSetDestination( triCamera* cam, triFloat angle, triVec4f* axis, triFloat px, triFloat py, triFloat pz, triFloat t )
{
	triQuatFromRotate( &cam->destRot, angle, axis );
	cam->destPos.x = px;
	cam->destPos.y = py;
	cam->destPos.z = pz;
	cam->t = t;
}


void triCameraInterpolate( triCamera* cam, triFloat dt )
{
	if (cam->t<=0) return;	// we already reached our destination point!
	triQuat one;
	triQuatUnit( &one );
	if (dt>cam->t)
	{
		// avoid interpolating over the destination!
		cam->rot = cam->destRot;
		cam->pos = cam->destPos;
		cam->t = 0.0f;
	}
	else
	{
		triQuatNLerp( &cam->rot, &one, &cam->destRot, dt/cam->t );
		triVec4Lerp( &cam->pos, &cam->pos, &cam->destPos, dt/cam->t );
		cam->t -= dt;
	}
	triCameraDoRotation( cam, 0 );
}


void triCameraUpdateMatrix( triCamera* cam )
{
	if (cam->dirty==0) return;
	cam->dirty = 0;
	sceGumMatrixMode( GU_VIEW );
	
	ScePspFMatrix4 t;
	t.x.x = cam->right.x;
	t.y.x = cam->right.y;
	t.z.x = cam->right.z;
	t.w.x = 0.f;

	t.x.y = cam->up.x;
	t.y.y = cam->up.y;
	t.z.y = cam->up.z;
	t.w.y = 0.f;

	t.x.z = -cam->dir.x;
	t.y.z = -cam->dir.y;
	t.z.z = -cam->dir.z;
	t.w.z = 0.f;

	t.x.w = 0.0f;
	t.y.w = 0.0f;
	t.z.w = 0.0f;
	t.w.w = 1.0f;

	ScePspFVector3 p;
	p.x = -cam->pos.x;
	p.y = -cam->pos.y;
	p.z = -cam->pos.z;
	gumTranslate(&t,&p);

	sceGumLoadMatrix(&t);
	sceGumMatrixMode( GU_MODEL );
}

void triCameraProject( triVec4* vout, triCamera* cam, triVec4* vin )
{
	ScePspFMatrix4 p, v;
	sceGumMatrixMode( GU_PROJECTION );
	sceGumStoreMatrix(&p);
	triCameraUpdateMatrix( cam );
	sceGumMatrixMode( GU_VIEW );
	sceGumStoreMatrix(&v);
	sceGumMatrixMode( GU_MODEL );
	
	gumMultMatrix( &p, &p, &v );
	__asm__ (
		".set			push\n"					// save assembler option
		".set			noreorder\n"			// suppress reordering
		"ulv.q			c100,  0 + %1\n"		// c100 = 
		"ulv.q			c110, 16 + %1\n"		// c110 = 
		"ulv.q			c120, 32 + %1\n"		// c120 = 
		"ulv.q			c130, 48 + %1\n"		// c130 = 
		"lv.q			c200, %2\n"				// c200 = *vin
		"vone.s			s203\n"
		"vtfm4.q		c000, e100, c200\n"		// c000 = e100 * c200
		"vfim.s			s200, 0.5\n"
		"vrcp.s			s003, s003\n"			// w = 1/w
		"vscl.t			c000, c000, s003\n"		// x|y|z *= w
		"vscl.p			c000, c000, s200\n"
		"vadd.p			c000, c000, c200[x,x]\n"
		"vocp.s			s001, s001\n"
		"sv.q			c000, %0\n"				// *vout = c000
		".set			pop\n"					// restore assembler option
		: "=m"(*vout)
		: "o"(p), "m"(*vin)
	);
}

void triCameraUnproject( triVec4* vout, triCamera* cam, triVec4* vin )
{
	ScePspFMatrix4 p, v;
	sceGumMatrixMode( GU_PROJECTION );
	sceGumStoreMatrix(&p);
	triCameraUpdateMatrix( cam );
	sceGumMatrixMode( GU_VIEW );
	sceGumStoreMatrix(&v);
	sceGumMatrixMode( GU_MODEL );

	gumMultMatrix( &p, &p, &v );
	gumFullInverse( &p, &p );
	__asm__ (
		".set			push\n"					// save assembler option
		".set			noreorder\n"			// suppress reordering
		"ulv.q			c100,  0 + %1\n"		// c100 = 
		"ulv.q			c110, 16 + %1\n"		// c110 = 
		"ulv.q			c120, 32 + %1\n"		// c120 = 
		"ulv.q			c130, 48 + %1\n"		// c130 = 
		"lv.q			c200, %2\n"				// c200 = *vin
		"vone.s			s203\n"
		"vfim.s			s300, 480.0\n"
		"vfim.s			s301, 272.0\n"
		"vfim.s			s302, 65535.0\n"
		"vrcp.t			c300, c300\n"
		"vfim.s			s303, 2.0\n"
		"vscl.p			c200, c200, s303\n"
		"vmul.t			c200, c200, c300\n"
		"vadd.t			c200, c200, c300[1,1,1]\n"
		
		"vtfm4.q		c000, e100, c200\n"		// c000 = e100 * c200
		"vrcp.s			s003, s003\n"			// w = 1/w
		"vscl.t			c000, c000, s003\n"		// x|y|z *= w
		"sv.q			c000, %0\n"				// *vout = c000
		".set			pop\n"					// restore assembler option
		: "=m"(*vout)
		: "o"(p), "m"(*vin)
	);
}
