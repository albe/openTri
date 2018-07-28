/*
 * tri3d.c: Code for 3D Engine
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
#include "triTypes.h"
#include "tri3d.h"
#include "triGraphics.h"
#include "triVAlloc.h"

#define MAX_RENDERBUFFERS 4

void*	triRenderbuffer[MAX_RENDERBUFFERS] = { 0 };
static triS32 triRenderbufferWidth[MAX_RENDERBUFFERS],
			triRenderbufferHeight[MAX_RENDERBUFFERS],
			triRenderbufferTwidth[MAX_RENDERBUFFERS],
			triRenderbufferTheight[MAX_RENDERBUFFERS],
			triRenderbufferBpp[MAX_RENDERBUFFERS],
			triRenderbufferPsm[MAX_RENDERBUFFERS];
			
			
static triS32 tri3dRendertargetWidth, tri3dRendertargetHeight;
static triS32 tri3dRendertextureWidth, tri3dRendertextureHeight;


static triS32 tri3dInitialized = 0;



#define ALIGN16 __attribute__((aligned(16)))

#define UNCACHED(x)		(0x40000000 | (x))
#define VRAM_BASE 		(0x04000000)
#define VRAM_SIZE 		(0x00200000)
#define VRAM_UNSWIZZLED (0x04200000)			// VRAM mirror for unswizzled read
#define VRAM_LINEAR 	(0x04600000)			// VRAM mirror for linear read (unswizzled + deinterleave)


void tri3dInit()
{
	if (tri3dInitialized!=0) return;
	tri3dInitialized = 1;
	
	triInit( GU_PSM_8888, 1 );		// if not yet initialized, we init a 32bit render mode
	
	tri3dRendertargetWidth = SCREEN_WIDTH;
	tri3dRendertargetHeight = SCREEN_HEIGHT;
	
	triBegin();
	triDepthbuffer = vrelptr(triVAlloc(FRAME_BUFFER_SIZE*2));
	sceGuDepthBuffer( triDepthbuffer, FRAME_BUFFER_WIDTH );
	sceGuDepthRange(65535,0);
	sceGuDepthFunc(GU_GEQUAL);
	triEnable(TRI_DEPTH_TEST);
	triEnable(TRI_DEPTH_WRITE);
	sceGuClearDepth(0x0);
	
	sceGuFrontFace(GU_CCW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuDisable(GU_BLEND);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexWrap(GU_REPEAT,GU_REPEAT);
	sceGuEnable(GU_CLIP_PLANES);
	
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	tri3dPerspective( 75.0f );
	triEnd();
}

void tri3dClose()
{
	if (!tri3dInitialized) return;
	triVFree(vabsptr(triDepthbuffer));
	triDepthbuffer = 0;
	triS32 i;
	for (i=0;i<MAX_RENDERBUFFERS;i++)
		tri3dRenderbufferClose(i);
	tri3dInitialized = 0;
	triClose();
}


void tri3dFilter( triS32 filter )
{
	sceGuTexFilter(filter,filter);
}


// Return next power of 2
static triS32 nextPow2(triU32 w)
{
	if (w==0) return 0;
	triS32 n = 2;
	while (w>n)
		n <<= 1;
	return (n);
}


void tri3dRenderToAlpha(triS32 enable)
{
	if (enable)
		sceGuSendCommandi(211,((GU_COLOR_BUFFER_BIT|GU_STENCIL_BUFFER_BIT) << 8)|0x1);
	else
		sceGuSendCommandi(211,0);
}

void tri3dRenderbufferCreate( triS32 n, triS32 psm, triS32 width, triS32 height )
{
	if (n>=MAX_RENDERBUFFERS) return;
	
	switch (psm) {
		case GU_PSM_4444:
		case GU_PSM_5650:
		case GU_PSM_5551:
			triRenderbufferPsm[n] = psm;
			triRenderbufferBpp[n] = 2;
			break;
		default:
			triRenderbufferPsm[n] = GU_PSM_8888;
			triRenderbufferBpp[n] = 4;
			break;
		}
	
	if (triRenderbuffer[n]) triVFree(triRenderbuffer[n]);
	triRenderbufferWidth[n] = width;
	triRenderbufferHeight[n] = height;
	triRenderbufferTwidth[n] = nextPow2(width);
	triRenderbufferTheight[n] = nextPow2(height);
	triRenderbuffer[n] = triVAlloc(triRenderbufferTwidth[n]*triRenderbufferHeight[n]*triRenderbufferBpp[n]);
}

void tri3dRenderbufferClose( triU32 n )
{
	if (n>=MAX_RENDERBUFFERS) return;
	if (triRenderbuffer[n]) triVFree(triRenderbuffer[n]);
	triRenderbuffer[n] = 0;
}

void tri3dRenderbufferSetRendertarget( triU32 n )
{
	if (n>=MAX_RENDERBUFFERS) return;
	if (triRenderbuffer[n]==0) return;
	sceGuDrawBufferList( triRenderbufferPsm[n], triRenderbuffer[n], triRenderbufferTwidth[n] );
	
	sceGuOffset(2048 - (triRenderbufferWidth[n]/2), 2048 - (triRenderbufferHeight[n]/2));
	sceGuViewport(2048, 2048, triRenderbufferWidth[n], triRenderbufferHeight[n]);
	sceGuScissor(0, 0, triRenderbufferWidth[n], triRenderbufferHeight[n]);

	tri3dRendertargetWidth = triRenderbufferWidth[n];
	tri3dRendertargetHeight = triRenderbufferHeight[n];
}

void tri3dRenderbufferSetTexture( triU32 n )
{
	if (n>=MAX_RENDERBUFFERS) return;
	if (triRenderbuffer[n]==0) return;
	
	sceGuTexMode(triRenderbufferPsm[n],0,0,0);
	sceGuTexImage(0,triRenderbufferTwidth[n],triRenderbufferTheight[n],triRenderbufferTwidth[n],triRenderbuffer[n]);
	sceGuTexScale( (triFloat)triRenderbufferWidth[n]/triRenderbufferTwidth[n], (triFloat)triRenderbufferHeight[n]/triRenderbufferTheight[n] );
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);
	
	tri3dRendertextureWidth = triRenderbufferWidth[n];
	tri3dRendertextureHeight = triRenderbufferHeight[n];
}

void tri3dFramebufferSetRendertarget()
{
	triRendertoscreen();
	tri3dRenderToAlpha(0);
	tri3dRendertargetWidth = SCREEN_WIDTH;
	tri3dRendertargetHeight = SCREEN_HEIGHT;
}

void tri3dFramebufferSetTexture()
{
	sceGuTexMode(triPsm,0,0,0);
	sceGuTexImage(0,FRAME_BUFFER_WIDTH,FRAME_BUFFER_WIDTH,FRAME_BUFFER_WIDTH,triFramebuffer);
	sceGuTexScale( (triFloat)SCREEN_WIDTH/FRAME_BUFFER_WIDTH, (triFloat)SCREEN_HEIGHT/FRAME_BUFFER_WIDTH );	// scale u/v so 1.0 resolves to screen resolution, not texture resolution (512x512)
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);

	tri3dRendertextureWidth = SCREEN_WIDTH;
	tri3dRendertextureHeight = SCREEN_HEIGHT;
}

void tri3dDepthbufferSetTexture( triU32 mode, void* ramp )
{
	switch (mode) {
		case 0:
			sceGuTexMode(GU_PSM_5650,0,0,0);
			break;
		case 1:
			sceGuTexMode(GU_PSM_5551,0,0,0);
			break;
		case 2:
			sceGuClutLoad( (256/8), ramp );
			sceGuClutMode( GU_PSM_8888, 8, 0xFF, 0 );
			sceGuTexMode(GU_PSM_T16,0,0,0);
			break;
		}
	sceGuTexImage(0,FRAME_BUFFER_WIDTH,FRAME_BUFFER_WIDTH,FRAME_BUFFER_WIDTH,(void*)((u32)triDepthbuffer|VRAM_LINEAR));
	sceGuTexScale( (triFloat)SCREEN_WIDTH/FRAME_BUFFER_WIDTH, (triFloat)SCREEN_HEIGHT/FRAME_BUFFER_WIDTH );
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);
	
	tri3dRendertextureWidth = SCREEN_WIDTH;
	tri3dRendertextureHeight = SCREEN_HEIGHT;
}



void tri3dRenderFullscreenTexture( triFloat u0, triFloat v0, triFloat u1, triFloat v1 )
{
	triFloat cur_x = 0, cur_u = u0;
	triFloat xEnd = (triFloat)tri3dRendertargetWidth;
	triFloat slice = 64.f;
	triFloat ustep = (u1-u0)/(triFloat)tri3dRendertargetWidth * slice;

	sceGuTexWrap(GU_CLAMP,GU_CLAMP);
	triDisable(TRI_DEPTH_TEST);
	triDisable(TRI_DEPTH_MASK);
	for( ;cur_x<xEnd; )
	{
		triVertUVf* vertices = (triVertUVf*)sceGuGetMemory(2 * sizeof(triVertUVf));

		triFloat polyWidth = ((cur_x+slice) > xEnd) ? (xEnd-cur_x) : slice;
		triFloat sourceWidth = ((cur_u+ustep) > u1) ? (u1-cur_u) : ustep;

		vertices[0].u = cur_u;
		vertices[0].v = v0;
		vertices[0].x = cur_x;
		vertices[0].y = 0.f;
		vertices[0].z = 0.f;

		cur_u += sourceWidth;
		cur_x += polyWidth;

		vertices[1].u = cur_u;
		vertices[1].v = v1;
		vertices[1].x = cur_x;
		vertices[1].y = tri3dRendertargetHeight;
		vertices[1].z = 0.f;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
	sceGuTexWrap(GU_REPEAT,GU_REPEAT);
	triEnable(TRI_DEPTH_TEST);
	triEnable(TRI_DEPTH_MASK);
}


void tri3dRenderTexture( triFloat x, triFloat y, triFloat u0, triFloat v0, triFloat u1, triFloat v1 )
{
	triFloat cur_x = x, cur_u = u0;
	triFloat xEnd = x+(u1-u0);
	triFloat slice = 64.f;
	triFloat ustep = slice;

	triDisable(TRI_DEPTH_TEST);
	triDisable(TRI_DEPTH_MASK);
	for( ;cur_x<xEnd; )
	{
		triVertUVf* vertices = (triVertUVf*)sceGuGetMemory(2 * sizeof(triVertUVf));

		triFloat polyWidth = ((cur_x+slice) > xEnd) ? (xEnd-cur_x) : slice;
		triFloat sourceWidth = ((cur_u+ustep) > u1) ? (u1-cur_u) : ustep;

		vertices[0].u = cur_u;
		vertices[0].v = v0;
		vertices[0].x = cur_x;
		vertices[0].y = y;
		vertices[0].z = 0.f;

		cur_u += sourceWidth;
		cur_x += polyWidth;

		vertices[1].u = cur_u;
		vertices[1].v = v1;
		vertices[1].x = cur_x;
		vertices[1].y = y + (v1-v0);
		vertices[1].z = 0.f;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
	triEnable(TRI_DEPTH_TEST);
	triEnable(TRI_DEPTH_MASK);
}


void tri3dRenderbufferToScreen( triS32 n, triS32 x, triS32 y )
{
	if (triRenderbufferPsm[n]!=triPsm) return;
	sceGuCopyImage( triRenderbufferPsm[n],
					0,0, triRenderbufferWidth[n], triRenderbufferHeight[n], triRenderbufferTwidth[n], triRenderbuffer[n],
					0,0, 512, triFramebuffer );
	sceGuTexSync();
}


triVertUVf vin;
void tri3dGridShader( triS32 twidth, triS32 theight, triS32 ysteps, triS32 xsteps, tri3dVertexShaderUVf shader )
{
	if (shader==0) return;
	if (xsteps<=0 || ysteps<=0) return;
	
	triFloat xscale = (triFloat)tri3dRendertargetWidth/(triFloat)xsteps;
	triFloat yscale = (triFloat)tri3dRendertargetHeight/(triFloat)ysteps;
	triFloat uscale = (triFloat)twidth/(triFloat)xsteps;
	triFloat vscale = (triFloat)theight/(triFloat)ysteps;
	
	triVertUVf* vertices = (triVertUVf*)sceGuGetMemory(((ysteps*(xsteps+1) + (ysteps-1))*2) * sizeof(triVertUVf));
	triVertUVf* v = vertices;
	int x, y;
	for (y=0; y<ysteps; y++)
	{
		for (x=0; x<=xsteps; x++)
		{
			vin.u = x*uscale;
			vin.v = y*vscale;
			vin.x = x*xscale;
			vin.y = y*yscale;
			vin.z = 0.f;
			shader( v, &vin );
			v++;
			vin.u = x*uscale;
			vin.v = (y+1)*vscale;
			vin.x = x*xscale;
			vin.y = (y+1)*yscale;
			vin.z = 0.f;
			shader( v, &vin );
			v++;
		}
		if ((y+1)<ysteps)
		{
			vin.u = (xsteps)*uscale;
			vin.v = (y+1)*vscale;
			vin.x = (xsteps)*xscale;
			vin.y = (y+1)*yscale;
			vin.z = 0.f;
			shader( v, &vin );
			v++;
			vin.u = 0.f;
			vin.v = (y+1)*vscale;
			vin.x = 0.f;
			vin.y = (y+1)*yscale;
			vin.z = 0.f;
			shader( v, &vin );
			v++;
		}
	}
	
	triDisable(TRI_CULL_FACE);
	triDisable(TRI_DEPTH_TEST);
	triDisable(TRI_DEPTH_MASK);
	sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	triEnable(TRI_DEPTH_TEST);
	triEnable(TRI_DEPTH_MASK);
	triEnable(TRI_CULL_FACE);
}


// Apply a 4x4 percentage closer filter on the source and render it to rendertarget
void tri3d4x4Pcf( triS32 twidth, triS32 theight )
{
	triFloat v0 = -1.5, v1 = theight - 1.5;
	triFloat u0 = -1.5, u1 = twidth - 1.5;
	
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x3f3f3f3f, 0x000000);
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x40404040, 0xffffffff);
	
	
	u0 = 0.5; u1 = twidth + 0.5;
	v0 = -1.5; v1 = theight - 1.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	
	u0 = -1.5; u1 = twidth - 1.5;
	v0 = 0.5; v1 = theight + 0.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 0.5; u1 = twidth + 0.5;
	v0 = 0.5; v1 = theight + 0.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuDisable(GU_BLEND);
}


// 4x4 dithered PCF (less displaced)
void tri3d4x4PcfDithered( triS32 twidth, triS32 theight )
{
	triFloat v0 = -1.25, v1 = theight - 1.25;
	triFloat u0 = -1.25, u1 = twidth - 1.25;
	
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x3f3f3f3f, 0x000000);
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );


	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x40404040, 0xffffffff);
	u0 = 0.75; u1 = twidth + 0.75;
	v0 = -1.25; v1 = theight - 1.25;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	
	u0 = -1.25; u1 = twidth - 1.25;
	v0 = 0.75; v1 = theight + 0.75;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 0.75; u1 = twidth + 0.75;
	v0 = 0.75; v1 = theight + 0.75;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuDisable(GU_BLEND);
}


// Filter matrix:
// 1.0  1.5  1.0
// 1.5  2.0  1.5
// 1.0  1.5  1.0
void tri3d3x3OverlapFilter( triS32 twidth, triS32 theight )
{
	triFloat v0 = -0.5, v1 = (triFloat)theight - 0.5;
	triFloat u0 = -0.5, u1 = (triFloat)twidth - 0.5;
	
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x3f3f3f3f, 0x000000);
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );


	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x40404040, 0xffffffff);
	u0 = 0.5; u1 = twidth + 0.5;
	v0 = -0.5; v1 = theight - 0.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	
	u0 = -0.5; u1 = twidth - 0.5;
	v0 = 0.5; v1 = theight + 0.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 0.5; u1 = twidth + 0.5;
	v0 = 0.5; v1 = theight + 0.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuDisable(GU_BLEND);
}


// do a 5x5 convolution from rendertarget n1 to rendertarget n2
// source and dest must have same size!
// 
// 
// 
// 
// 
void tri3d5x5_2stepConvolution( triS32 n1, triS32 n2 )
{
	if (triRenderbuffer[n1]==0 || triRenderbuffer[n2]==0) return;
	if (triRenderbufferPsm[n1]!=triRenderbufferPsm[n2] || triRenderbufferWidth[n1]!=triRenderbufferWidth[n2] || triRenderbufferHeight[n1]!=triRenderbufferHeight[n2]) return;
	triS32 twidth = triRenderbufferWidth[n2];
	triS32 theight = triRenderbufferHeight[n2];
	sceGuCopyImage( triRenderbufferPsm[n1],
					0,0, triRenderbufferWidth[n1], triRenderbufferHeight[n1], triRenderbufferTwidth[n1], triRenderbuffer[n1],
					0,0, triRenderbufferTwidth[n2], triRenderbuffer[n2] );
	sceGuTexSync();
	
	tri3dRenderbufferSetTexture( n1 );
	tri3dRenderbufferSetRendertarget( n2 );
	
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0x555555);
	triFloat v0 = 0, v1 = theight;
	triFloat u0 = -1.5, u1 = twidth - 1.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );


	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0xffffff);
	u0 = 1.5; u1 = twidth + 1.5;
	v0 = 0; v1 = theight;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );

	// renderbuffer2 -> renderbuffer1
	sceGuCopyImage( triRenderbufferPsm[n2],
					0,0, triRenderbufferWidth[n2], triRenderbufferHeight[n2], triRenderbufferTwidth[n2], triRenderbuffer[n2],
					0,0, triRenderbufferTwidth[n1], triRenderbuffer[n1] );
	sceGuTexSync();
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0x555555);
	u0 = 0; u1 = twidth;
	v0 = 1.5; v1 = theight + 1.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0xffffff);
	u0 = 0; u1 = twidth;
	v0 = -1.5; v1 = theight - 1.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );

	sceGuDisable(GU_BLEND);
}


void tri3d9x9_2stepConvolution( triS32 n1, triS32 n2 )
{
	if (triRenderbuffer[n1]==0 || triRenderbuffer[n2]==0) return;
	triS32 twidth = tri3dRendertextureWidth = triRenderbufferWidth[n2];
	triS32 theight = tri3dRendertextureHeight = triRenderbufferHeight[n2];
	sceGuCopyImage( triRenderbufferPsm[n1],
					0,0, triRenderbufferWidth[n1], triRenderbufferHeight[n1], triRenderbufferTwidth[n1], triRenderbuffer[n1],
					0,0, triRenderbufferTwidth[n2], triRenderbuffer[n2] );
	sceGuTexSync();
	
	tri3dRenderbufferSetTexture( n1 );
	tri3dRenderbufferSetRendertarget( n2 );
	
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0x333333);
	triFloat u0 = -1.5, u1 = twidth - 1.5;	
	triFloat v0 = 0, v1 = theight;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );

	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0xffffff);
	u0 = -3.5; u1 = twidth - 3.5;
	v0 = 0; v1 = theight;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 1.5; u1 = twidth + 1.5;
	v0 = 0; v1 = theight;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 3.5; u1 = twidth + 3.5;
	v0 = 0; v1 = theight;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );

	// renderbuffer2 -> renderbuffer1
	sceGuCopyImage( triRenderbufferPsm[n2],
					0,0, triRenderbufferWidth[n2], triRenderbufferHeight[n2], triRenderbufferTwidth[n2], triRenderbuffer[n2],
					0,0, triRenderbufferTwidth[n1], triRenderbuffer[n1] );
	sceGuTexSync();
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0x333333);
	u0 = 0; u1 = twidth;
	v0 = 1.5; v1 = theight + 1.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );

	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0xffffff);	
	u0 = 0; u1 = twidth;
	v0 = 3.5; v1 = theight + 3.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 0; u1 = twidth;
	v0 = -1.5; v1 = theight - 1.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 0; u1 = twidth;
	v0 = -3.5; v1 = theight - 3.5;
	tri3dRenderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuDisable(GU_BLEND);
}



void tri3dClear( triS32 col, triS32 stencil, triS32 depth )
{
	triS32 mask = 0;
	if (col) mask |= GU_COLOR_BUFFER_BIT;
	if (stencil) mask |= GU_STENCIL_BUFFER_BIT;
	if (depth) mask |= GU_DEPTH_BUFFER_BIT;
	sceGuClear(mask|GU_FAST_CLEAR_BIT);
}


// Wrappers for triGraphics functions
void tri3dEnable( triS32 what )
{
	triEnable( what );
}

void tri3dDisable( triS32 what )
{
	triDisable( what );
}

void tri3dPerspective( triFloat fov )
{
	triPerspective( fov );
}

void tri3dOrtho()
{
	triOrtho();
}
