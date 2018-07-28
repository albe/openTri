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


static inline void triCameraDoRotation( triCamera* cam )
{
	triQuatApply( &cam->dir, &cam->rot, &cam->dir );
	triQuatApply( &cam->up, &cam->rot, &cam->up );
	triQuatApply( &cam->right, &cam->rot, &cam->right );
	//triVec4Neg( &cam->pos, triQuatApply( &cam->pos, &cam->rot, triVec4Neg( &cam->pos, &cam->pos ) ) );
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
	triQuatFromRotate( &cam->rot, angle, axis );
	#ifdef LDEBUG
	triLogPrint("Camera>\n");
	triLogPrint("rot: <%.2f, %.2f, %.2f, %.2f>\n", cam->rot.x, cam->rot.y, cam->rot.z, cam->rot.w );
	#endif
	triCameraDoRotation( cam );
}


void triCameraRotateAbout( triCamera* cam, triFloat angle, triVec4f* axis, triVec4f* center )
{
	//triQuatFromRotate( &cam->rot, angle, axis );
	triVec4Sub3( &cam->pos, &cam->pos, center );
	triCameraRotate( cam, angle, axis );
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
	triCameraDoRotation( cam );
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

	t.x.y = cam->up.x;
	t.y.y = cam->up.y;
	t.z.y = cam->up.z;

	t.x.z = cam->dir.x;
	t.y.z = cam->dir.y;
	t.z.z = cam->dir.z;

	t.w.x = cam->pos.x;
	t.w.y = cam->pos.y;
	t.w.z = cam->pos.z;

	t.x.w = 0.0f;
	t.y.w = 0.0f;
	t.z.w = 0.0f;
	t.w.w = 1.0f;

	sceGumLoadMatrix(&t);
	sceGumMatrixMode( GU_MODEL );
}
