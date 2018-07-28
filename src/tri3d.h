/*
 * tri3d.h: Header for 3D Engine
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
 
#ifndef __TRI3D_H__
#define __TRI3D_H__


#include "triTypes.h"
#include "triModel.h"



/** @defgroup tri3d 3D graphics core
 *  @{
 */
 
// depth texture modes
#define TRI3D_DEPTHTEXTURE_5650 0
#define TRI3D_DEPTHTEXTURE_5551 1
#define TRI3D_DEPTHTEXTURE_T16 2


typedef void (*tri3dVertexShaderUVf)(triVertUVf* vout, triVertUVf* vin);



void tri3dInit();
void tri3dClose();

void tri3dClear( triS32 col, triS32 stencil, triS32 depth );
void tri3dEnable( triS32 what );
void tri3dDisable( triS32 what );

void tri3dPerspective( triFloat fov );
void tri3dOrtho();


void tri3dFilter( triS32 filter );

// Enable rendering to the alpha channel of current render target
void tri3dRenderToAlpha(triS32 enable);

// Render buffers (glow texture, feedback buffer, etc)
void tri3dRenderbufferCreate( triS32 n, triS32 psm, triS32 width, triS32 height );
void tri3dRenderbufferClose( triU32 n );
void tri3dRenderbufferSetTexture( triU32 n );
void tri3dRenderbufferSetRendertarget( triU32 n );

void tri3dRenderbufferToScreen( triS32 n, triS32 x, triS32 y );

void tri3dFramebufferSetRendertarget();
// Make the current framebuffer a texture source
void tri3dFramebufferSetTexture();
// Make the depthbuffer a texture source
void tri3dDepthbufferSetTexture( triU32 mode, void* ramp );

// Render source to rendertarget (unscaled) at postion x,y
void tri3dRenderTexture( triFloat x, triFloat y, triFloat u0, triFloat v0, triFloat u1, triFloat v1 );
// Render source fullscren to rendertarget
void tri3dRenderFullscreenTexture( triFloat u0, triFloat v0, triFloat u1, triFloat v1 );

// Draw a xstep x ystep grid onto rendertarget and apply a shader onto each vertex
void tri3dGridShader( triS32 twidth, triS32 theight, triS32 ysteps, triS32 xsteps, tri3dVertexShaderUVf shader );

// Apply a 4x4 percentage closer filter on the source and render it to rendertarget
void tri3d4x4Pcf( triS32 twidth, triS32 theight );
// 4x4 dithered PCF (less displaced)
void tri3d4x4PcfDithered( triS32 twidth, triS32 theight );
void tri3d3x3OverlapFilter( triS32 twidth, triS32 theight );


// do a 5x5/9x9 convolution from rendertarget n1 to rendertarget n2
// source and dest must have same size!
void tri3d5x5_2stepConvolution( triS32 n1, triS32 n2 );
void tri3d9x9_2stepConvolution( triS32 n1, triS32 n2 );

/** @} */

#endif // __TRI3D_H__
