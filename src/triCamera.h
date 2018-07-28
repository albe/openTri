/*
 * triCamera.h: Header for Camera
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
 
#ifndef __TRICAMERA_H__
#define __TRICAMERA_H__

#include "triTypes.h"


/** @defgroup triCamera Camera control
 *  @{
 */
 
 
extern triVec4f	triOrigin;

typedef struct
{
	triVec4		pos;		// the camera's position (triVec4 for vfpu accesses)
	triVec4		dir;		// the camera's looking direction
	triVec4		up;			// the camera's up vector
	triVec4		right;		// the camera's right vector

	triQuat		rot;		// the camera's current rotation in triQuaternion form

	triVec4		destPos;	// the camera's destination position
	triQuat		destRot;	// the camera's destination rotation
	triFloat	t;			// the time to take the camera to destination

	triS32		dirty;		// camera changed, needs view matrix update
} triCamera ALIGN16;




// Create a default camera looking into positive z direction at position (x,y,z)
triCamera* triCameraCreate( triFloat x, triFloat y, triFloat z );

// Static camera movement control:
// move camera position by (x,y,z)
void triCameraMove( triCamera* cam, triFloat x, triFloat y, triFloat z );

// rotate camera angle degrees around the axis (x,y,z) around itself
void triCameraRotate( triCamera* cam, triFloat angle, triVec4f* axis );

// rotate camera angle degrees around the axis (x,y,z) around the point center
void triCameraRotateAbout( triCamera* cam, triFloat angle, triVec4f* axis, triVec4f* center );


// Camera movement based on keyframe positions:
// set camera destination point based on rotation (angle,x,y,z) and position (px,py,pz) and a time frame to reach the position
void triCameraSetDestination( triCamera* cam, triFloat angle, triVec4f* axis, triFloat px, triFloat py, triFloat pz, triFloat t );

// do an interpolation step towards the camera's destination
void triCameraInterpolate( triCamera* cam, triFloat dt );

// update the VIEW matrix to resemble the camera
void triCameraUpdateMatrix( triCamera* cam );


// project world space coordinates into normalized device space coordinates
void triCameraProject( triVec4* vout, triCamera* cam, triVec4* vin );
// unproject screen space coordinates into world space coordinates
void triCameraUnproject( triVec4* vout, triCamera* cam, triVec4* vin );
/** @} */

#endif // __TRICAMERA_H__
