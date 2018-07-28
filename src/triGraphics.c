/*
 * triGraphics.c: Code for 2D Graphics Engine
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 Alexander Berl 'Raphael' <raphael@fx-world.org>
 *
 * $Id: triGraphics.c 25 2008-08-22 20:06:45Z Raphael $
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
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psprtc.h>
#include <stdio.h>
#include <string.h>
#include "triTypes.h"
#include "triMemory.h"
#include "triGraphics.h"
#include "triVAlloc.h"
#include "triLog.h"


extern triU32 triDlistSizeUser __attribute__((weak));
static triU32 triDlistSize = 256*1024;

// Globals
//--------------------------------------------------
static triVoid* triList[2];
//static triU32 __attribute__((aligned(16))) triList[2][262144/2];	// 2*256kb
static triU32 triListIdx = 0;

static	void*	triFrontbuffer	= 0;
static void*	triBackbuffer	= (void*)(FRAME_BUFFER_SIZE*4);
void*			triFramebuffer	= (void*)(0x04000000);	// always holds the absolute pointer to current drawbuffer
void*			triFramebuffer2	= (void*)(0x04000000);	// always holds the absolute pointer to last drawbuffer
void*			triDispbuffers[2] = { 0, 0 };
void*			triDepthbuffer	= 0;
triS32			triPsm = GU_PSM_8888;
triS32			triBpp = 4;
static triS32	triDoublebuffer = 1;
static triS32	triInitialized = 0;
static triS32	triListAvail = 0;

static triS32	triVBlankEnable = 0;
static triS32	triPseudoAAEnable	= 0;
static triS32	triSmoothditherEnable = 1;


static triS32	triVBlankFlag = 0;		// was VBlank called manually this frame?
static triS32	triSmoothditherFlag = 0;
static triS32	triPseudoAAFlag = 0;


static PspGeContext triGeContext;
static triFloat triFpsValue = 0.f;
static triFloat triFpsMinValue = 9999.f;
static triFloat triFpsMaxValue = 0.f;
static triU64	triLastTick = 0;
static triU32	triTickFrequency = 0;
static triU64	triFrameCount = 0;
static triU64	triFrameTime = 0;
static triU64	triCPUTime = 0;
static triU64	triGPUTime = 0;
static triU64	triVblankTime = 0;
static triS32	triColorKey = 0;
static triS32	triBlend = 0;
static triS32	triCurContext = 0;


static triFloat triSpriteWidth = 0.f;
static triFloat triSpriteHeight = 0.f;
static triFloat triSpriteAngle = 0.f;


#define ALIGN16 __attribute__((aligned(16)))

#define UNCACHED(x)		(triVoid*)(0x40000000 | (triU32)(x))
#define VRAM_BASE 		(0x04000000)
#define VRAM_SIZE 		(0x00200000)
#define VRAM_UNSWIZZLED (0x04200000)			// VRAM mirror for unswizzled read
#define VRAM_LINEAR 	(0x04600000)			// VRAM mirror for linear read (unswizzled + deinterleave)


#define MIN(a,b) ((a)<(b)?(a):(b))

// Do a fast blend of two colors x and y by alpha value z (255-srcalpha), where x is alpha premultiplied
#define FASTBLEND(x,y,z) (((x&0xFF00FF) + (((z*(y&0xFF00FF))>>8)&0xFF00FF))|((x&0xFF00) + (((z*(y&0xFF00))>>8)&0xFF00)))

static triS32 DitherMatrix[2][16] = { { 0, 8, 0, 8,
										 8, 0, 8, 0,
										 0, 8, 0, 8,
										 8, 0, 8, 0 },
									   { 8, 8, 8, 8,
										 0, 8, 0, 8,
										 8, 8, 8, 8,
										 0, 8, 0, 8 } };
							/*{ {	 0,  8,  2, 10,
									12,  4, 14,  6,
									 3, 11,  1,  9,
									15,  7, 13,  5 },
								{ 	 0,  8,  2, 10,
									12, 10, 14, 12,
									 2,  9,  1, 11,
									14, 12, 13, 12 } };*/

static triFloat ProjectionMatrix[2][16];

/* //Default dither matrix:
triS32 dither_matrix[16] = {
			-4, 0,-3, 1,
			 2,-2, 3,-1,
			-3, 1,-4, 0,
			 3,-1, 2,-2 };
*/

// {6,0,7,1,2,4,3,5,7,1,6,0,3,5,2,4};

void triGuFinishCallback( int what )
{
	triLogPrint("triGuFinishCallback( %i ) - Frame %i\n", what, triFrameCount);

	sceRtcGetCurrentTick(&triGPUTime);
}

void triInit( triS32 psm, triBool doublebuffer )
{
	if (triInitialized!=0) return;
	triInitialized = 1;

	triDoublebuffer = doublebuffer;
	
	switch (psm)
	{
		case GU_PSM_4444:
		case GU_PSM_5650:
		case GU_PSM_5551:
			triBpp = 2;
			triPsm = psm;
			break;
		default:
			triBpp = 4;
			triPsm = GU_PSM_8888;
			break;
	}
	triBackbuffer = vrelptr(triVAlloc(FRAME_BUFFER_SIZE*triBpp));
	if (triDoublebuffer)
	{
		triFrontbuffer = vrelptr(triVAlloc(FRAME_BUFFER_SIZE*triBpp));
	}
	else
	{
		triFrontbuffer = triBackbuffer;
		triDispbuffers[0] = triMalloc(FRAME_BUFFER_SIZE*triBpp);
		triDispbuffers[1] = triMalloc(FRAME_BUFFER_SIZE*triBpp);
		memset(triDispbuffers[0],0,FRAME_BUFFER_SIZE*triBpp);
		memset(triDispbuffers[1],0,FRAME_BUFFER_SIZE*triBpp);
	}
	
	triFramebuffer = vabsptr(triFrontbuffer);
	triFramebuffer2 = vabsptr(triBackbuffer);
	if (&triDlistSizeUser != NULL)
	{
		triDlistSize = triDlistSizeUser;
	}
	triList[0] = triMalloc(triDlistSize);
	triList[1] = triMalloc(triDlistSize);
	sceGuInit();

	// setup GU
	sceGuStart(GU_DIRECT,triList[triListIdx]);
	sceGuDrawBuffer(triPsm, triFrontbuffer, FRAME_BUFFER_WIDTH);
	sceGuDispBuffer(512, 512, triBackbuffer, FRAME_BUFFER_WIDTH);

	sceGuOffset(2048 - (SCREEN_WIDTH/2), 2048 - (SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Scissoring
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);

	// Backface culling
	sceGuFrontFace(GU_CCW);
	sceGuDisable(GU_CULL_FACE);		// no culling in 2D

	// Depth test
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_TRUE);		// disable z-writes

	// Color keying
	sceGuDisable(GU_COLOR_TEST);

	sceGuDisable(GU_ALPHA_TEST);
	
	
	sceGuDisable(GU_CLIP_PLANES);

	// Texturing
	sceGuEnable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexEnvColor(0xFFFFFFFF);
	sceGuColor(0xFFFFFFFF);
	sceGuAmbientColor(0xFFFFFFFF);
	sceGuTexOffset(0.0f, 0.0f);
	sceGuTexScale(1.0f, 1.0f);
	
	// Blending
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	sceGuDisable(GU_DITHER);
	if (triBpp<4)
	{
		sceGuSetDither( (ScePspIMatrix4*)DitherMatrix[0] );
		sceGuEnable(GU_DITHER);
	}

	
	// Projection
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[0] );
	gumOrtho( (ScePspFMatrix4*)ProjectionMatrix[0], 0.0f, 480.0f, 272.0f, 0.0f, -1.0f, 1.0f );
	
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[1] );
	ScePspFVector3 displace = { -0.002f, 0.00367f, 0.0f };	// ~ 1/480, 1/272
	gumTranslate( (ScePspFMatrix4*)ProjectionMatrix[1], &displace );
	gumMultMatrix( (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[0] );

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix( (ScePspFMatrix4*)ProjectionMatrix[0] );
	

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	
	sceGuClearColor( 0x0 );
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
	if (!triDoublebuffer)
	{
		sceDisplaySetFrameBuf( triDispbuffers[1], 512, triPsm, PSP_DISPLAY_SETBUF_NEXTFRAME );
	}

	sceGuStart(GU_SEND, triList[triListIdx]);
	triListAvail = 1;
	
	sceRtcGetCurrentTick(&triLastTick);
	triTickFrequency = sceRtcGetTickResolution();
	triFrameCount = 0;
}


void triClose()
{
	if (!triInitialized) return;
	triVFree(vabsptr(triBackbuffer));
	if (triDoublebuffer)
	{
		triVFree(vabsptr(triFrontbuffer));
	}
	else
	{
		triFree(triDispbuffers[0]);
		triFree(triDispbuffers[1]);
	}
	triFree(triList[0]);
	triFree(triList[1]);
	triInitialized = 0;
	triListAvail = 0;
	sceGuTerm();
}


void triBegin()
{
	sceGuStart(GU_DIRECT, triList[triListIdx^1]);
	triCurContext = 1;
}


void triEnd()
{
	if (sceGuFinish()>triDlistSize)
	{
		triLogError("ERROR: Display list overflow!\n");
	}
	triCurContext = 0;
}


void triSync()
{
	//sceGuFinish();
	//sceGuSync(0,0);
	//sceGuStart(GU_DIRECT, (void*)((triU32)triList[triListIdx]|0x40000000));
	//triListIdx ^= 1;
	if (triCurContext==1)
		sceGuSync(GU_SYNC_LIST,GU_SYNC_WHAT_DONE);
	else
		sceGuSync(GU_SYNC_SEND,GU_SYNC_WHAT_DONE);
}


void triSwapbuffers()
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (!triDoublebuffer)
	{
		sceGuCopyImage(triPsm, 0, 0, 480, 272, 512, triFramebuffer, 0, 0, 512, triDispbuffers[triFrameCount&1]);
		sceGuTexSync();
	}
	if (sceGuFinish()>triDlistSize)
	{
		triLogError("ERROR: Display list overflow!\n");
	}
	if ((triFrameCount&15)==0) sceRtcGetCurrentTick(&triCPUTime);
	//sceGuSync(GU_SYNC_SEND,GU_SYNC_WHAT_DONE);
	sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
	if ((triFrameCount&15)==0) sceRtcGetCurrentTick(&triGPUTime);
	if ((triVBlankEnable || triPseudoAAEnable) && !triVBlankFlag)
		sceDisplayWaitVblankStart();
	if ((triFrameCount&15)==0) sceRtcGetCurrentTick(&triVblankTime);
	
	triVBlankFlag = 0;
	triFramebuffer2 = triFramebuffer;
	if (triDoublebuffer)
	{
		triFramebuffer = vabsptr(sceGuSwapBuffers());
	}
	else
	{
		sceDisplaySetFrameBuf( triDispbuffers[triFrameCount&1], 512, triPsm, PSP_DISPLAY_SETBUF_NEXTFRAME );
	}

	// On the double buffered sendlist approach: Possible problem with renderbuffer effects?
	//
	// Renderbuffer effects often depend on waiting for the result of a render, then
	// use that for the next step. Since with the sendlist approach the display is
	// always one frame behind, we have no means of syncing to those results.
	// So either we need a method to switch to an immediate list for those renders
	// or just stay with immediate renders completely.
	// Same problem applies if we are about to change a vertex or texture buffer each frame
	// for animation or alike. The GE might be accessing that buffer at the time we
	// actually fill it for the next frame, giving unpredictable results. This can be
	// avoided for vertices if they are stored in the list (sceGuGetMemory) and for
	// textures if they are double buffered too.
	sceGuSendList(GU_TAIL, triList[triListIdx], &triGeContext);

	sceGuStart(GU_SEND, triList[triListIdx^=1]);
	sceGuDrawBufferList(triPsm, triFramebuffer2, FRAME_BUFFER_WIDTH);

	if (triBpp<4 && triSmoothditherEnable)
		sceGuSetDither( (ScePspIMatrix4*)DitherMatrix[triSmoothditherFlag^=1] );

	if (triPseudoAAEnable)
	{
		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadMatrix( (ScePspFMatrix4*)ProjectionMatrix[triPseudoAAFlag^=1] );
		sceGumMatrixMode(GU_MODEL);
	}

	if ((triFrameCount&15)==0)
	{
		triCPUTime -= triFrameTime;
		triGPUTime -= triFrameTime;
		triVblankTime -= triFrameTime;
	}
	
	triFrameCount++;
	triU64 triEndTick;
	sceRtcGetCurrentTick(&triEndTick);
	triFpsValue = (triFloat)triFrameCount/(triEndTick-triLastTick)*triTickFrequency;
	if ((triFrameCount&31)==0)
	{
		if (triFpsMinValue>triFpsValue) triFpsMinValue = triFpsValue;
		if (triFpsMaxValue<triFpsValue) triFpsMaxValue = triFpsValue;
	}
	if ((triEndTick-triLastTick)>=triTickFrequency)
	{
		triFrameCount = 0;
		triLastTick = triEndTick;
	}
	triFrameTime = triEndTick;
}


void triClear( triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuClearColor( color );
	sceGuClear( GU_COLOR_BUFFER_BIT | GU_FAST_CLEAR_BIT );
}


void triVblank()
{
	sceDisplayWaitVblankStart();
	triVBlankFlag = 1;
}

triFloat triCPULoad()
{
	if (triVblankTime==0)
		return 0.0f;
	else
		return ((triFloat)triCPUTime * 100.f / (triFloat)triVblankTime);
}

triFloat triGPULoad()
{
	if (triVblankTime==0)
		return 0.0f;
	else
		return ((triFloat)triGPUTime * 100.f / (triFloat)triVblankTime);
}

triFloat triFps()
{
	return triFpsValue;
}

triFloat triFpsMax()
{
	return triFpsMaxValue;
}

triFloat triFpsMin()
{
	return triFpsMinValue;
}


void triCopyToScreen( triS32 x, triS32 y, triS32 w, triS32 h, triS32 sx, triS32 sy, triS32 sbw, void* src )
{
	if (triCurContext==1)
		sceGuCopyImage( triPsm, x, y, w, h, 512, triFramebuffer2, sx, sy, sbw, src );
	else
		sceGuCopyImage( triPsm, x, y, w, h, 512, triFramebuffer, sx, sy, sbw, src );
		
	sceGuTexSync();
}

void triCopyFromScreen( triS32 x, triS32 y, triS32 w, triS32 h, triS32 dx, triS32 dy, triS32 dbw, void* dst )
{
	if (triCurContext==1)
		sceGuCopyImage( triPsm, dx, dy, w, h, dbw, dst, x, y, 512, triFramebuffer2 );
	else
		sceGuCopyImage( triPsm, dx, dy, w, h, dbw, dst, x, y, 512, triFramebuffer );
	sceGuTexSync();
}

void triPerspective( triFloat fov )
{
	if (fov<1.0f || fov>179.0f) fov = 75.0f;
	
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[0] );
	gumPerspective( (ScePspFMatrix4*)ProjectionMatrix[0], fov, 16.0f/9.0f, 1.0f, 1000.0f );
	
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[1] );
	ScePspFVector3 displace = { -0.002f, 0.00367f, 0.0f };	// ~ 1/480, 1/272
	gumTranslate( (ScePspFMatrix4*)ProjectionMatrix[1], &displace );
	gumMultMatrix( (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[0] );

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix( (ScePspFMatrix4*)ProjectionMatrix[triPseudoAAFlag] );
	
	sceGumMatrixMode(GU_MODEL);
}


void triOrtho()
{
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[0] );
	gumOrtho( (ScePspFMatrix4*)ProjectionMatrix[0], 0.0f, 480.0f, 272.0f, 0.0f, -1.0f, 1.0f );
	
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[1] );
	ScePspFVector3 displace = { -0.002f, 0.00367f, 0.0f };	// ~ 1/480, 1/272
	gumTranslate( (ScePspFMatrix4*)ProjectionMatrix[1], &displace );
	gumMultMatrix( (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[0] );

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix( (ScePspFMatrix4*)ProjectionMatrix[triPseudoAAFlag] );
	
	sceGumMatrixMode(GU_MODEL);
}


void triEnable( triU32 state )
{
	switch (state)
	{
		case TRI_VBLANK:
			triVBlankEnable = 1;
			break;
		case TRI_PSEUDO_FSAA:
			triPseudoAAEnable = 1;
			break;
		case TRI_SMOOTH_DITHER:
			triSmoothditherEnable = 1;
			break;
		case TRI_DEPTH_TEST:
			if (triDepthbuffer!=0)
				sceGuEnable(GU_DEPTH_TEST);
			break;
		case TRI_DEPTH_MASK:
			if (triDepthbuffer!=0)
				sceGuDepthMask(0);
			break;
		default:
			sceGuEnable( state );
			break;
	}
}


void triDisable( triU32 state )
{
	switch (state)
	{
		case TRI_VBLANK:
			triVBlankEnable = 0;
			break;
		case TRI_PSEUDO_FSAA:
			triPseudoAAEnable = 0;
			triPseudoAAFlag = 0;
			break;
		case TRI_SMOOTH_DITHER:
			triSmoothditherEnable = 0;
			break;
		case TRI_DEPTH_TEST:
			sceGuDisable(GU_DEPTH_TEST);
			break;
		case TRI_DEPTH_MASK:
			sceGuDepthMask(0xFFFF);
			break;
		default:
			sceGuDisable( state );
			break;
	}
}




void triRendertotexture( triS32 psm, void* tbp, triS32 tw, triS32 th, triS32 tbw )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triS32 bpp = 0;
	// check that pixelformat is a legal renderformat
	switch (psm)
	{
		case GU_PSM_4444:
		case GU_PSM_5650:
		case GU_PSM_5551:
			bpp = 2;
			break;
		case GU_PSM_8888:
			bpp = 4;
			break;
		default:
			triLogPrint("Cannot render in pixelformat %x\n", psm);
			return;
	}
	if ((triU32)tbp<VRAM_BASE || ((triU32)tbp+tbw*th*bpp)>VRAM_SIZE+VRAM_LINEAR)
	{
		triLogPrint("Cannot render to non VRAM space.\n");
		return;	// check that texture is in VRAM
	}
	sceGuDrawBufferList( psm, vrelptr(tbp), tbw );
	
	sceGuOffset(2048 - (tw/2), 2048 - (th/2));
	sceGuViewport(2048, 2048, tw, th);
	sceGuScissor(0, 0, tw, th);
}



void triRendertoimage( triImage* img )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triS32 bpp = 0;
	// check that pixelformat is a legal renderformat
	switch (img->format)
	{
		case GU_PSM_4444:
		case GU_PSM_5650:
		case GU_PSM_5551:
			bpp = 2;
			break;
		case GU_PSM_8888:
			bpp = 4;
			break;
		default:
			triLogPrint("Cannot render in pixelformat %x\n", img->format);
			return;
	}

	// make sure the image is in VRAM
	triImageToVRAM( img );
	if ((triU32)img->data<VRAM_BASE || ((triU32)img->data+img->size)>VRAM_SIZE+VRAM_LINEAR)
	{
		triLogPrint("Cannot render to non VRAM space.\n");
		return;	// check that texture is in VRAM
	}
	sceGuDrawBufferList( img->format, vrelptr(img->data), img->stride );
	
	sceGuOffset(2048 - (img->width/2), 2048 - (img->height/2));
	sceGuViewport(2048, 2048, img->width, img->height);
	sceGuScissor(0, 0, img->width, img->height);
}


void triRendertoscreen()
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (triCurContext==1)
		sceGuDrawBufferList(triPsm, vrelptr(triFramebuffer), FRAME_BUFFER_WIDTH);
	else
		sceGuDrawBufferList(triPsm, vrelptr(triFramebuffer2), FRAME_BUFFER_WIDTH);
	
	sceGuOffset(2048 - (SCREEN_WIDTH/2), 2048 - (SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}



void triSetClip( triS32 x, triS32 y, triS32 width, triS32 height )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuScissor(x, y, width, height);
}


void triResetClip()
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}


void triDrawLine( triFloat x0, triFloat y0, triFloat x1, triFloat y1, triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triVertC* vertices = (triVertC*)sceGuGetMemory(2 * sizeof(triVertC));

	vertices[0].color = color;
	vertices[0].x = x0; 
	vertices[0].y = y0; 
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x1; 
	vertices[1].y = y1; 
	vertices[1].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_LINES, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}

void triDrawLines( triVec2* p, triS32 num, triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triVertC* vertices = (triVertC*)sceGuGetMemory(num * sizeof(triVertC));

	int i = 0;
	for (;i<num;i++)
	{
		vertices[i].color = color;
		vertices[i].x = p[i].x; 
		vertices[i].y = p[i].y;
		vertices[i].z = 0.0f;
	}

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, num, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}


void triDrawRectOutline( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triVertC* vertices = (struct triVertC*)sceGuGetMemory(5 * sizeof(triVertC));

	vertices[0].color = color;
	vertices[0].x = x; 
	vertices[0].y = y; 
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x+width; 
	vertices[1].y = y; 
	vertices[1].z = 0.0f;

	vertices[2].color = color;
	vertices[2].x = x+width; 
	vertices[2].y = y+height; 
	vertices[2].z = 0.0f;
	
	vertices[3].color = color;
	vertices[3].x = x; 
	vertices[3].y = y+height; 
	vertices[3].z = 0.0f;
	
	vertices[4].color = color;
	vertices[4].x = x; 
	vertices[4].y = y;
	vertices[4].z = 0.0f;
	
	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 5, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}


void triDrawRect( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triVertC* vertices = (triVertC*)sceGuGetMemory(2 * sizeof(triVertC));

	vertices[0].color = color;
	vertices[0].x = x; 
	vertices[0].y = y; 
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x + width; 
	vertices[1].y = y + height; 
	vertices[1].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_SPRITES, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}




void triDrawRectGrad( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color1, triU32 color2, triU32 color3, triU32 color4 )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	
	if (width<4 || height<4)
	{
		triVertC* vertices = (triVertC*)sceGuGetMemory(4 * sizeof(triVertC));
		
		vertices[0].color = color1;
		vertices[0].x = x;
		vertices[0].y = y;
		vertices[0].z = 0;
		
		vertices[1].color = color2;
		vertices[1].x = x + width;
		vertices[1].y = y;
		vertices[1].z = 0;
		
		vertices[2].color = color3;
		vertices[2].x = x + width;
		vertices[2].y = y + height;
		vertices[2].z = 0;
		
		vertices[3].color = color4;
		vertices[3].x = x;
		vertices[3].y = y + height;
		vertices[3].z = 0;
		
		sceGuDisable(GU_TEXTURE_2D);
		sceGuShadeModel(GU_SMOOTH);
		sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
		sceGuEnable(GU_TEXTURE_2D);
		
	}
	else
	{
		triVertC* vertices = (triVertC*)sceGuGetMemory(6 * sizeof(triVertC));
		// Help the color interpolation by subdividing the quad:
		vertices[0].color = ((color1 >> 2) & 0x3F3F3F) + ((color2 >> 2)  & 0x3F3F3F) + ((color3 >> 2)  & 0x3F3F3F) + ((color4 >> 2)  & 0x3F3F3F);
		vertices[0].x = x + width/2;
		vertices[0].y = y + height/2;
		vertices[0].z = 0;
		
		vertices[1].color = color1;
		vertices[1].x = x;
		vertices[1].y = y;
		vertices[1].z = 0;
		
		vertices[2].color = color2;
		vertices[2].x = x + width;
		vertices[2].y = y;
		vertices[2].z = 0;
		
		vertices[3].color = color3;
		vertices[3].x = x + width;
		vertices[3].y = y + height;
		vertices[3].z = 0;
		
		vertices[4].color = color4;
		vertices[4].x = x;
		vertices[4].y = y + height;
		vertices[4].z = 0;
		
		vertices[5].color = color1;
		vertices[5].x = x;
		vertices[5].y = y;
		vertices[5].z = 0;
	
		sceGuDisable(GU_TEXTURE_2D);
		sceGuShadeModel(GU_SMOOTH);
		sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 6, 0, vertices);
		sceGuEnable(GU_TEXTURE_2D);
	}
}



void triDrawTriOutline( triFloat x0, triFloat y0,  triFloat x1, triFloat y1,  triFloat x2, triFloat y2, triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triVertC* vertices = (struct triVertC*)sceGuGetMemory(4 * sizeof(triVertC));

	vertices[0].color = color;
	vertices[0].x = x0; 
	vertices[0].y = y0; 
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x1; 
	vertices[1].y = y1; 
	vertices[1].z = 0.0f;

	vertices[2].color = color;
	vertices[2].x = x2; 
	vertices[2].y = y2; 
	vertices[2].z = 0.0f;
	
	vertices[3].color = color;
	vertices[3].x = x0; 
	vertices[3].y = y0; 
	vertices[3].z = 0.0f;
	
	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}


void triDrawTri( triFloat x0, triFloat y0,  triFloat x1, triFloat y1,  triFloat x2, triFloat y2, triU32 color )
{
	triDrawTriGrad( x0, y0, x1, y1, x2, y2, color, color, color );
}


void triDrawTriGrad( triFloat x0, triFloat y0,  triFloat x1, triFloat y1,  triFloat x2, triFloat y2, triU32 color1, triU32 color2, triU32 color3 )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triVertC* vertices = (struct triVertC*)sceGuGetMemory(3 * sizeof(triVertC));

	vertices[0].color = color1;
	vertices[0].x = x0; 
	vertices[0].y = y0; 
	vertices[0].z = 0.0f;

	vertices[1].color = color2;
	vertices[1].x = x1; 
	vertices[1].y = y1; 
	vertices[1].z = 0.0f;

	vertices[2].color = color3;
	vertices[2].x = x2; 
	vertices[2].y = y2; 
	vertices[2].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_TRIANGLES, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 3, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}


// FIXME: Move to triMath.c
void triSinCos( triFloat angle, triFloat* sin, triFloat* cos )
{
	asm (
		"vcst.s   S003, VFPU_2_PI\n"
		"mtv %2, s002\n"
		"vmul.s s002, s002, s003\n"
		"vrot.p c000, s002, [s,c]\n"
		"mfv %0, s000\n"
		"mfv %1, s001\n"
		:"=r"(*sin),"=r"(*cos)
		:"r"(angle)
		);
}


void triDrawRectRotate( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color, triFloat angle )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	triVertC* vertices = (triVertC*)sceGuGetMemory(4 * sizeof(triVertC));
	width *= 0.5f;
	height *= 0.5f;
	triFloat sin, cos;
	triSinCos( angle, &sin, &cos );
	triFloat sw = sin*width;
	triFloat sh = sin*height;
	triFloat cw = cos*width;
	triFloat ch = cos*height;

	vertices[0].color = color;
	vertices[0].x = x - cw - sh;
	vertices[0].y = y - sw + ch;
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x - cw + sh; 
	vertices[1].y = y - sw - ch; 
	vertices[1].z = 0.0f;
	
	vertices[2].color = color;
	vertices[2].x = x + cw + sh; 
	vertices[2].y = y + sw - ch; 
	vertices[2].z = 0.0f;

	vertices[3].color = color;
	vertices[3].x = x + cw - sh; 
	vertices[3].y = y + sw + ch; 
	vertices[3].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}



void triDrawCircle( triFloat x, triFloat y, triFloat radius, triU32 color )
{
	triDrawCircleGrad( x, y, radius, color, color );
}


void triDrawCircleOutline( triFloat x, triFloat y, triFloat radius, triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (radius<=0) return;
	triS32 numSteps = 5 + ((int)radius/32);
	
	triFloat stepSize = (2*GU_PI)/(numSteps*4);
	
	triVertC* vertices = (triVertC*) sceGuGetMemory((numSteps*4 + 1) * sizeof(triVertC));

	unsigned int numSteps2 = numSteps*2, numSteps3 = numSteps*3;
	int step;
	triFloat angle = 0;
	for(step = 0; step < numSteps; step++)
	{
		triFloat s;
		triFloat c;
		triSinCos(angle,&s,&c);
		s *= radius;
		c *= radius;
		angle += stepSize;
		
		vertices[step].color = color;
		vertices[step].x = x + s;
		vertices[step].y = y + c;
		vertices[step].z = 0.0f;
		
		vertices[step+numSteps].color = color;
		vertices[step+numSteps].x = x + c;
		vertices[step+numSteps].y = y - s;
		vertices[step+numSteps].z = 0.0f;

		vertices[step+numSteps2].color = color;
		vertices[step+numSteps2].x = x - s;
		vertices[step+numSteps2].y = y - c;
		vertices[step+numSteps2].z = 0.0f;

		vertices[step+numSteps3].color = color;
		vertices[step+numSteps3].x = x - c;
		vertices[step+numSteps3].y = y + s;
		vertices[step+numSteps3].z = 0.0f;
	}

	vertices[numSteps*4].color = color;
	vertices[numSteps*4].x = vertices[0].x;
	vertices[numSteps*4].y = vertices[0].y;
	vertices[numSteps*4].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, numSteps*4 + 1, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}


void triDrawCircleGrad( triFloat x, triFloat y, triFloat radius, triU32 color1, triU32 color2 )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (radius<=0) return;
	triS32 numSteps = 5 + ((int)radius/32);

	triFloat stepSize = (2*GU_PI)/(numSteps*4);
	
	triVertC* vertices = (triVertC*) sceGuGetMemory((numSteps*4 + 2) * sizeof(triVertC));

	vertices[0].color = color1;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	unsigned int numSteps2 = numSteps*2, numSteps3 = numSteps*3;
	int step;
	triFloat angle = 0;
	for(step = 1; step < numSteps+1; step++)
	{
		triFloat s;
		triFloat c;
		triSinCos(angle,&s,&c);
		s *= radius;
		c *= radius;
		angle += stepSize;
		
		vertices[step].color = color2;
		vertices[step].x = x + s;
		vertices[step].y = y + c;
		vertices[step].z = 0.0f;
		
		vertices[step+numSteps].color = color2;
		vertices[step+numSteps].x = x + c;
		vertices[step+numSteps].y = y - s;
		vertices[step+numSteps].z = 0.0f;

		vertices[step+numSteps2].color = color2;
		vertices[step+numSteps2].x = x - s;
		vertices[step+numSteps2].y = y - c;
		vertices[step+numSteps2].z = 0.0f;

		vertices[step+numSteps3].color = color2;
		vertices[step+numSteps3].x = x - c;
		vertices[step+numSteps3].y = y + s;
		vertices[step+numSteps3].z = 0.0f;
	}

	vertices[numSteps*4+1].color = color2;
	vertices[numSteps*4+1].x = vertices[1].x;
	vertices[numSteps*4+1].y = vertices[1].y;
	vertices[numSteps*4+1].z = 0.0f;
	
	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, numSteps*4 + 2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}



void triDrawRegPoly( triFloat x, triFloat y, triFloat radius, triU32 color, triU32 numSteps, triFloat angle )
{
	triDrawRegPolyGrad( x, y, radius, color, color, numSteps, angle );
}

void triDrawRegPolyOutline( triFloat x, triFloat y, triFloat radius, triU32 color, triU32 numSteps, triFloat angle )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (radius<=0) return;
	if (numSteps<3) numSteps = 3;
	
	triFloat stepSize = (2*GU_PI)/numSteps;
	
	triVertC* vertices = (triVertC*) sceGuGetMemory((numSteps + 1) * sizeof(triVertC));

	int step;
	for(step = 0; step < numSteps; step++)
	{
		triFloat s;
		triFloat c;
		triSinCos( angle, &s, &c );
		s *= radius;
		c *= radius;
		angle += stepSize;
		
		vertices[step].color = color;
		vertices[step].x = x - s;
		vertices[step].y = y + c;
		vertices[step].z = 0.0f;
	}

	vertices[numSteps].color = color;
	vertices[numSteps].x = vertices[0].x;
	vertices[numSteps].y = vertices[0].y;
	vertices[numSteps].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, numSteps + 1, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}


void triDrawRegPolyGrad( triFloat x, triFloat y, triFloat radius, triU32 color1, triU32 color2, triU32 numSteps, triFloat angle )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (radius<=0) return;
	if (numSteps<3) numSteps = 3;
	triFloat stepSize = (2*GU_PI)/numSteps;
	
	triVertC* vertices = (triVertC*) sceGuGetMemory((numSteps + 2) * sizeof(triVertC));

	vertices[0].color = color1;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	int step;
	for(step = 1; step < numSteps+1; step++)
	{
		triFloat s;
		triFloat c;
		triSinCos( angle, &s, &c );
		s *= radius;
		c *= radius;
		angle += stepSize;
		
		vertices[step].color = color2;
		vertices[step].x = x - s;
		vertices[step].y = y + c;
		vertices[step].z = 0.0f;
	}

	vertices[numSteps+1].color = color2;
	vertices[numSteps+1].x = vertices[1].x;
	vertices[numSteps+1].y = vertices[1].y;
	vertices[numSteps+1].z = 0.0f;
	
	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, numSteps + 2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void triDrawStar( triFloat x, triFloat y, triFloat radiusInner, triFloat radiusOuter, triU32 color, triU32 numSteps, triFloat angle )
{
	triDrawStarGrad( x, y, radiusInner, radiusOuter, color, color, numSteps, angle );
}

void triDrawStarOutline( triFloat x, triFloat y, triFloat radiusInner, triFloat radiusOuter, triU32 color, triU32 numSteps, triFloat angle )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (radiusInner<=0) return;
	if (radiusOuter<=0) return;
	if (numSteps<3) numSteps = 3;
	
	triFloat stepSize = (2*GU_PI)/(numSteps*2);
	
	triVertC* vertices = (triVertC*) sceGuGetMemory((numSteps*2 + 1) * sizeof(triVertC));

	triFloat radi[2] = { radiusOuter, radiusInner };
	int radius = 0;
	int radiusOpp = numSteps&1;
	
	int step;
	for(step = 0; step < numSteps; step++)
	{
		triFloat s, c;
		triSinCos( angle, &s, &c );
		angle += stepSize;
		
		vertices[step].color = color;
		vertices[step].x = x - s*radi[radius];
		vertices[step].y = y + c*radi[radius];
		vertices[step].z = 0.0f;
		radius ^= 1;
		
		vertices[step+numSteps].color = color;
		vertices[step+numSteps].x = x + s*radi[radiusOpp];
		vertices[step+numSteps].y = y - c*radi[radiusOpp];
		vertices[step+numSteps].z = 0.0f;
		radiusOpp ^= 1;
	}

	vertices[numSteps*2].color = color;
	vertices[numSteps*2].x = vertices[0].x;
	vertices[numSteps*2].y = vertices[0].y;
	vertices[numSteps*2].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, numSteps*2 + 1, 0, vertices);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);
}


void triDrawStarGrad( triFloat x, triFloat y, triFloat radiusInner, triFloat radiusOuter, triU32 color1, triU32 color2, triU32 numSteps, triFloat angle )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (radiusInner<=0) return;
	if (radiusOuter<=0) return;
	if (numSteps<3) numSteps = 3;
	
	triFloat stepSize = (2*GU_PI)/(numSteps*2);
	
	triVertC* vertices = (triVertC*) sceGuGetMemory((numSteps*2 + 2) * sizeof(triVertC));

	triFloat radi[2] = { radiusOuter, radiusInner };
	int radius = 0;
	int radiusOpp = numSteps&1;
	
	vertices[0].color = color1;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;
	int step;
	for(step = 1; step < numSteps+1; step++)
	{
		triFloat s, c;
		triSinCos( angle, &s, &c );
		angle += stepSize;
		if (angle>=360.f) angle -= 360.f;
		
		vertices[step].color = color2;
		vertices[step].x = x - s*radi[radius];
		vertices[step].y = y + c*radi[radius];
		vertices[step].z = 0.0f;
		radius ^= 1;

		vertices[step+numSteps].color = color2;
		vertices[step+numSteps].x = x + s*radi[radiusOpp];
		vertices[step+numSteps].y = y - c*radi[radiusOpp];
		vertices[step+numSteps].z = 0.0f;
		radiusOpp ^= 1;
	}

	vertices[numSteps*2+1].color = color2;
	vertices[numSteps*2+1].x = vertices[1].x;
	vertices[numSteps*2+1].y = vertices[1].y;
	vertices[numSteps*2+1].z = 0.0f;
	
	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, numSteps*2 + 2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void triColorOp( triS32 op )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuEnable(GU_COLOR_LOGIC_OP);
	sceGuLogicalOp( op );
}

void triNoColorOp()
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuDisable(GU_COLOR_LOGIC_OP);
}


/**
  * Set how textures are applied
  *
  * Key for the apply-modes:
  *   - Cv - Color value result
  *   - Ct - Texture color
  *   - Cf - Fragment color (vcolor)
  *   - Cc - Constant color (ccolor)
  *
  * Available apply-modes are: (TFX)
  *   - GU_TFX_MODULATE - Cv=Ct*Cf TCC_RGB: Av=Af TCC_RGBA: Av=At*Af
  *   - GU_TFX_DECAL - TCC_RGB: Cv=Ct,Av=Af TCC_RGBA: Cv=Cf*(1-At)+Ct*At Av=Af
  *   - GU_TFX_BLEND - Cv=(Cf*(1-Ct))+(Cc*Ct) TCC_RGB: Av=Af TCC_RGBA: Av=At*Af
  *   - GU_TFX_REPLACE - Cv=Ct TCC_RGB: Av=Af TCC_RGBA: Av=At
  *   - GU_TFX_ADD - Cv=Cf+Ct TCC_RGB: Av=Af TCC_RGBA: Av=At*Af
  */
void triImageTint( triS32 mode, triS32 comp, triU32 vcolor, triU32 ccolor )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuTexFunc(mode, comp);
	sceGuTexEnvColor(ccolor);
	sceGuAmbientColor(vcolor);
}

void triImageNoTint()
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
}

// Same as triImageTint( GU_TFX_DECAL, GU_TCC_RGB, (alpha<<24)|0xFFFFFF, 0 );
void triImageConstAlpha( triU32 alpha )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuTexFunc(GU_TFX_DECAL, GU_TCC_RGB);
	sceGuAmbientColor((alpha<<24)|0xFFFFFF);
}

void triImageBlend( triS32 op, triS32 src_op, triS32 dst_op, triU32 src_fix, triU32 dst_fix )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc( op, src_op, dst_op, src_fix, dst_fix );
}

void triImageNoBlend()
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuDisable(GU_BLEND);
}

void triImageColorkey( triU32 color )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuEnable(GU_COLOR_TEST);
	sceGuColorFunc(GU_NOTEQUAL,color,0xffffff);
}

void triImageNoColorkey()
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	sceGuDisable(GU_COLOR_TEST);
}


void triSpriteMode( triFloat width, triFloat height, triFloat angle )
{
	if (width>512.f) width = 512.f;
	if (height>512.f) height = 512.f;
	triSpriteWidth = width;
	triSpriteHeight = height;
	triSpriteAngle = angle;
}

void triDrawSprite( triFloat x, triFloat y, triFloat u, triFloat v, triImage* img )
{
	if (img==0) return;
	int width = triSpriteWidth>0?triSpriteWidth:img->width;
	int height = triSpriteHeight>0?triSpriteHeight:img->height;
	if (triSpriteAngle==0.f)
		triDrawImage( x, y, width, height, u, v, u+width, v+height, img );
	else
		triDrawImageRotate( x, y, width, height, u, v, u+width, v+height, triSpriteAngle, img );
}

void triDrawImage2( triFloat x, triFloat y, triImage* img )
{
	if (img==0) return;
	triDrawImage( x, y, MIN(512,img->width), MIN(512,img->height), 0, 0, MIN(512,img->width), MIN(512,img->height), img );
}


void triBltSprite( triFloat x, triFloat y, triFloat u, triFloat v, triImage* img )
{
	if (img==0) return;
	int width = triSpriteWidth>0?triSpriteWidth:img->width;
	int height = triSpriteHeight>0?triSpriteHeight:img->height;
	if (img->swizzled==0 && img->format==triPsm)
	{
		sceGuCopyImage( img->format, (int)u, (int)v, MIN(480,width), MIN(272,height),
						img->stride, img->data,
						(int)x, (int)y, 512, triFramebuffer );
	}
	else
		triDrawImage( x, y, width, height, u, v, u+width, v+height, img );
}

void triDrawImage( triFloat x, triFloat y, triFloat width, triFloat height,	// rect pos and size on screen
				triFloat u0, triFloat v0, triFloat u1, triFloat v1,					// area of texture to render
			   triImage* img )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (img==0 || img->width==0 || img->height==0 || width==0 || height==0) return;
	
	/*if (img->swizzled==0 && img->format==triPsm && (u1-u0)==width && (v1-v0)==height)
	{
		sceGuCopyImage( img->format, u0, v0, width, height, img->stride, img->data,
						x, y, 512, triFramebuffer );
		return;
	}*/

	if (img->format==IMG_FORMAT_T4)
	{
		sceGuClutMode(img->palformat, 0, 0xff, 0);
		sceGuClutLoad(2, img->palette);
	}
	else if (img->format==IMG_FORMAT_T8)
	{
		sceGuClutMode(img->palformat, 0, 0xff, 0);
		sceGuClutLoad(32, img->palette);
	}
	sceGuTexMode(img->format, 0, 0, img->swizzled);
	triChar* data = img->data;
	if (u1>512.f)
	{
		int off = (int)u0 & ~31;
		data += ((off*img->bits) >> 3);
		u1 -= off;
		u0 -= off;
	}
	if (v1>512.f)
	{
		int off = (int)v0/* & ~15*/;
		data += off*img->stride*img->bits >> 3;
		v1 -= off;
		v0 -= off;
	}
	sceGuTexImage(0, MIN(512,img->stride), MIN(512,img->tex_height), img->stride, data);
	
	triFloat start, end;
	triFloat cur_u = u0;
	triFloat cur_x = x;
	triFloat x_end = x + width;
	triFloat slice = 64.f;
	triFloat ustep = (u1-u0)/width * slice;

	// blit maximizing the use of the texture-cache
	for( start=0, end=width; start<end; start+=slice )
	{
		triVertUV* vertices = (triVertUV*)sceGuGetMemory(2 * sizeof(triVertUV));

		triFloat poly_width = ((cur_x+slice) > x_end) ? (x_end-cur_x) : slice;
		triFloat source_width = ((cur_u+ustep) > u1) ? (u1-cur_u) : ustep;

		vertices[0].u = cur_u;
		vertices[0].v = v0;
		vertices[0].x = cur_x; 
		vertices[0].y = y; 
		vertices[0].z = 0;

		cur_u += source_width;
		cur_x += poly_width;

		vertices[1].u = cur_u;
		vertices[1].v = v1;
		vertices[1].x = cur_x;
		vertices[1].y = (y + height);
		vertices[1].z = 0;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
}




void triDrawImageRotate2( triFloat x, triFloat y, triFloat angle, triImage* img )
{
	triDrawImageRotate( x, y, MIN(512,img->width), MIN(512,img->height), 0, 0, MIN(512,img->width), MIN(512,img->height), angle, img );
}


void triDrawImageRotate( triFloat x, triFloat y, triFloat width, triFloat height,	// rect pos and size on screen
				triFloat u0, triFloat v0, triFloat u1, triFloat v1,					// area of texture to render
			   triFloat angle, triImage* img )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (img==0 || img->width==0 || img->height==0) return;

	if (img->format==IMG_FORMAT_T4)
	{
		sceGuClutMode(img->palformat, 0, 0xff, 0);
		sceGuClutLoad(2, img->palette);
	}
	else if (img->format==IMG_FORMAT_T8)
	{
		sceGuClutMode(img->palformat, 0, 0xff, 0);
		sceGuClutLoad(32, img->palette);
	}
	sceGuTexMode(img->format, 0, 0, img->swizzled);
	triChar* data = img->data;
	if (u1>=512.f)
	{
		data += ((int)u0)*img->bits >> 3;
		u1 -= u0;
		u0 = 0.f;
	}
	if (v1>=512.f)
	{
		data += ((int)v0)*img->stride*img->bits >> 3;
		v1 -= v0;
		v0 = 0.f;
	}
	sceGuTexImage(0, MIN(512,img->stride), MIN(512,img->tex_height), img->stride, data);
	sceGuTexScale( 1.0f / (triFloat)MIN(512,img->stride), 1.0f / (triFloat)MIN(512,img->tex_height) );
	
	triVertUV* vertices = (triVertUV*)sceGuGetMemory(4 * sizeof(triVertUV));
	width *= 0.5f;
	height *= 0.5f;
	triFloat sin, cos;
	triSinCos( angle, &sin, &cos );
	triFloat sw = sin*width;
	triFloat sh = sin*height;
	triFloat cw = cos*width;
	triFloat ch = cos*height;

	vertices[0].u = u0;
	vertices[0].v = v0;
	vertices[0].x = x - cw - sh;
	vertices[0].y = y - sw + ch;
	vertices[0].z = 0.0f;

	vertices[1].u = u0;
	vertices[1].v = v1;
	vertices[1].x = x - cw + sh; 
	vertices[1].y = y - sw - ch; 
	vertices[1].z = 0.0f;
	
	vertices[2].u = u1;
	vertices[2].v = v1;
	vertices[2].x = x + cw + sh; 
	vertices[2].y = y + sw - ch; 
	vertices[2].z = 0.0f;

	vertices[3].u = u1;
	vertices[3].v = v0;
	vertices[3].x = x + cw - sh; 
	vertices[3].y = y + sw + ch; 
	vertices[3].z = 0.0f;

	sceGuDrawArray(GU_TRIANGLE_FAN,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,4,0,vertices);

	//x' = x cos f - y sin f =: x1 - y1
	//y' = y cos f + x sin f =: y2 + x2
}


void triDrawImageAnimation( triFloat x, triFloat y, triImageAnimation* ani )
{
	if (ani->curFrame==0) return;
	triDrawImage( x + ani->curFrame->xOffs, y + ani->curFrame->yOffs, ani->curFrame->sw, ani->curFrame->sh, 
					ani->curFrame->sx, ani->curFrame->sy, ani->curFrame->sx+ani->curFrame->sw, ani->curFrame->sy+ani->curFrame->sh, ani->curFrame->image );
}

void triDrawImageAnimationRotate( triFloat x, triFloat y, triFloat angle, triImageAnimation* ani )
{
	if (ani->curFrame==0) return;
	triDrawImageRotate( x, y, ani->curFrame->sw, ani->curFrame->sh, 
					ani->curFrame->sx, ani->curFrame->sy, ani->curFrame->sx+ani->curFrame->sw, ani->curFrame->sy+ani->curFrame->sh, angle, ani->curFrame->image );

}

