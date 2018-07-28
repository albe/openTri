/*
 * triGraphics.h: Header for 2D Graphics Engine
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 Alexander Berl 'Raphael' <raphael@fx-world.org>
 *
 * $Id: triGraphics.h 45 2010-02-02 22:24:41Z raphael $
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

#ifndef __TRIGRAPHICS_H__
#define __TRIGRAPHICS_H__

#include <pspgu.h>
#include "triImage.h"


/** @defgroup triGraphics 2D graphics core
 *  @{
 */

// To override default display list size (256Kb)
#define TRI_DLIST_SIZE_KB(x)	triU32 triDlistSizeUser = ((x)*1024);

// Constants
//--------------------------------------------------
#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

#define FRAME_BUFFER_WIDTH 512
#define FRAME_BUFFER_SIZE		(FRAME_BUFFER_WIDTH*SCREEN_HEIGHT)



/* triEnable/Disable */
#define TRI_ALPHA_TEST					0
#define TRI_DEPTH_TEST					1
#define TRI_SCISSOR_TEST				2
#define TRI_STENCIL_TEST				3
#define TRI_BLEND						4
#define TRI_CULL_FACE					5
#define TRI_DITHER						6
#define TRI_FOG							7
#define TRI_CLIP_PLANES					8
#define TRI_TEXTURE_2D					9
#define TRI_LIGHTING					10
#define TRI_LIGHT0						11
#define TRI_LIGHT1						12
#define TRI_LIGHT2						13
#define TRI_LIGHT3						14
#define TRI_AALINE                      15
#define TRI_COLORKEY	                17
#define TRI_LOGICAL_OP		            18
#define TRI_NORMAL_REVERSE				19
#define TRI_VBLANK						32
#define TRI_PSEUDO_FSAA					33
#define TRI_SMOOTH_DITHER				34
#define TRI_DEPTH_MASK					35
#define TRI_DEPTH_WRITE					35



extern void* triFramebuffer;
extern void* triFramebuffer2;
extern void* triDepthbuffer;
extern triS32 triPsm;
extern triS32 triBpp;

/** Initialize graphics
  * @param psm Pixelformat to set output to (one of GU_PSM_4444,GU_PSM_5551,GU_PSM_5650 or GU_PSM_8888)
  * @param doublebuffer Whether to use doublebuffering or not.
  *
  * @note Doublebuffering will allocate two framebuffers in VRAM and switch those. Else only one
  *       framebuffer is allocated in VRAM and two others in System RAM for display ('kind of' triple buffered).
  *       As VRAM is small, this might be a valuable trade-off.
  *
  *       mode   pixelformat        Free VRAM (bytes)
  *     double   16bit (no depth)   1,540,096
  *     double   32bit (no depth)     983,040
  *     double   16bit (depth)      1,261,568
  *     double   32bit (depth)        704,512
  *     triple   16bit (no depth)   1,818,624
  *     triple   32bit (no depth)   1,540,096
  *     triple   16bit (depth)      1,540,096
  *     triple   32bit (depth)      1,261,568
  */
void triInit( triS32 psm, triBool doublebuffer );

/** Close graphics and free all resources
  */
void triClose();


/** Begin an immediate render command block
  * This is needed when you depend on the render result (e.g. for renderbuffer effects)
  */
void triBegin();

/** End an immediate render command block
  */
void triEnd();

/** Sync CPU to GU for the current immediate block
  */
void triSync();

/** Wait for rendering to finish and swap front- and backbuffer
  */
void triSwapbuffers();

/** Wait for Vblank.
  * Will do nothing if TRI_VBLANK is already enabled.
  */
void triVblank();

/** Return the current CPU load in percent.
  * @returns The current CPU load in percent
  */
triFloat triCPULoad();

/** Return the current GPU load in percent.
  * @returns The current GPU load in percent
  */
triFloat triGPULoad();

/** Return the current FPS.
  * @returns The current FPS
  */
triFloat triFps();

/** Return the maximal FPS.
  * @returns The maximal FPS
  */
triFloat triFpsMax();

/** Return the minimal FPS.
  * @returns The minimal FPS
  */
triFloat triFpsMin();

void triPerspective( triFloat fov );
void triOrtho();


void triEnable( triU32 state );
void triDisable( triU32 state );


void triClear( triU32 color );

// Set clipping area
void triSetClip( triS32 x, triS32 y, triS32 width, triS32 height );
// Reset clipping area
void triResetClip();


void triRendertoscreen();
void triRendertotexture( triS32 psm, void* tbp, triS32 tw, triS32 th, triS32 tbw );
void triRendertoimage( triImage* img );

void triCopyToScreen( triS32 x, triS32 y, triS32 w, triS32 h, triS32 sx, triS32 sy, triS32 sbw, void* src );
void triCopyFromScreen( triS32 x, triS32 y, triS32 w, triS32 h, triS32 dx, triS32 dy, triS32 dbw, void* dst );

/** Draw a line from (x0,y0) to (x1,y1) in color 'color'
  * @param x0 X position of first point
  * @param y0 Y position of first point
  * @param x1 X position of second point
  * @param y1 Y position of second point
  * @param color Color of line
  */
void triDrawLine( triFloat x0, triFloat y0, triFloat x1, triFloat y1, triU32 color );

/** Draw a line strip consisting of num points in color 'color'
  * @param p Array of triVec2's
  * @param num Number of vertices in line strip
  * @param color Color of the line strip
  */
void triDrawLines( triVec2* p, triS32 num, triU32 color );

/** Draw filled rectangle from (x,y) to (x+width,y+height) in color 'color'
  * @param x X position of rectangles top-left point
  * @param y Y position of rectangles top-left point
  * @param width Width of the rectangle
  * @param height Height of the rectangle
  * @param color Color of rectangle
  */
void triDrawRect( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color );

/** Draw rectangle from (x,y) to (x+width,y+height) in color 'color' with outline only
  * @param x X position of rectangles top-left point
  * @param y Y position of rectangles top-left point
  * @param width Width of the rectangle
  * @param height Height of the rectangle
  * @param color Color of rectangle
  */
void triDrawRectOutline( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color );

/** Draw Rectangle from (x,y) to (x+width,y+height) in color 'color' rotated by angle degree around it's center
  * @param x X position of rectangles top-left point
  * @param y Y position of rectangles top-left point
  * @param width Width of the rectangle
  * @param height Height of the rectangle
  * @param color Color of rectangle
  * @param angle Number of degree to rotate the rectangle
  */
void triDrawRectRotate( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color, triFloat angle );

/** Draw Rectangle from (x,y) to (x+width,y+height) with gradient
  * @param x X position of rectangles top-left point
  * @param y Y position of rectangles top-left point
  * @param width Width of the rectangle
  * @param height Height of the rectangle
  * @param color1 Color of top-left point
  * @param color2 Color of top-right point
  * @param color3 Color of bottom-right point
  * @param color4 Color of bottom-left point
  */
void triDrawRectGrad( triFloat x, triFloat y, triFloat width, triFloat height, triU32 color1, triU32 color2, triU32 color3, triU32 color4 );

/** Draw Circle
  * @param x X position of circle center
  * @param y Y position of circle center
  * @param radius Radius of circle
  * @param color Color of circle
  */
void triDrawCircle( triFloat x, triFloat y, triFloat radius, triU32 color );

/** Draw Circle (outline only)
  * @param x X position of circle center
  * @param y Y position of circle center
  * @param radius Radius of circle
  * @param color Color of circle
  */
void triDrawCircleOutline( triFloat x, triFloat y, triFloat radius, triU32 color );

/** Draw Circle with gradient
  * @param x X position of circle center
  * @param y Y position of circle center
  * @param radius Radius of circle
  * @param color1 Inner color of circle
  * @param color2 Outer color of circle
  */
void triDrawCircleGrad( triFloat x, triFloat y, triFloat radius, triU32 color1, triU32 color2 );

/** Draw arbitrary triangle
  * @param x0 X position of first point
  * @param y0 Y position of first point
  * @param x1 X position of second point
  * @param y1 Y position of second point
  * @param x2 X position of third point
  * @param y2 Y position of third point
  * @param color Color of triangle
  */
void triDrawTri( triFloat x0, triFloat y0,  triFloat x1, triFloat y1,  triFloat x2, triFloat y2, triU32 color );

/** Draw arbitrary triangle with outline only
  * @param x0 X position of first point
  * @param y0 Y position of first point
  * @param x1 X position of second point
  * @param y1 Y position of second point
  * @param x2 X position of third point
  * @param y2 Y position of third point
  * @param color Color of triangle
  */
void triDrawTriOutline( triFloat x0, triFloat y0,  triFloat x1, triFloat y1,  triFloat x2, triFloat y2, triU32 color );

/** Draw arbitrary triangle with outline only
  * @param x0 X position of first point
  * @param y0 Y position of first point
  * @param x1 X position of second point
  * @param y1 Y position of second point
  * @param x2 X position of third point
  * @param y2 Y position of third point
  * @param color1 Color of first point
  * @param color2 Color of second point
  * @param color3 Color of third point
  */
void triDrawTriGrad( triFloat x0, triFloat y0,  triFloat x1, triFloat y1,  triFloat x2, triFloat y2, triU32 color1, triU32 color2, triU32 color3 );

/** Draw regular Polygon (Pentagon,Hexagon,Octagon,etc)
  * @param x X position of polygon center
  * @param y Y position of polygon center
  * @param radius Radius of polygon points
  * @param color Color of polygon
  * @param numSteps Number of points of polygon (>2)
  * @param angle Number of degree to rotate the polygon by
  */
void triDrawRegPoly( triFloat x, triFloat y, triFloat radius, triU32 color, triU32 numSteps, triFloat angle );

/** Draw regular Polygon (Pentagon,Hexagon,Octagon,etc) with outline only
  * @param x X position of polygon center
  * @param y Y position of polygon center
  * @param radius Radius of polygon points
  * @param color Color of polygon
  * @param numSteps Number of points of polygon (>2)
  * @param angle Number of degree to rotate the polygon by
  */
void triDrawRegPolyOutline( triFloat x, triFloat y, triFloat radius, triU32 color, triU32 numSteps, triFloat angle );

/** Draw regular Polygon (Pentagon,Hexagon,Octagon,etc) with gradient
  * @param x X position of polygon center
  * @param y Y position of polygon center
  * @param radius Radius of polygon points
  * @param color1 Color of polygon center
  * @param color2 Color of polygon outer points
  * @param numSteps Number of points of polygon (>2)
  * @param angle Number of degree to rotate the polygon by
  */
void triDrawRegPolyGrad( triFloat x, triFloat y, triFloat radius, triU32 color1, triU32 color2, triU32 numSteps, triFloat angle );

/** Draw Star
  * @param x X position of star center
  * @param y Y position of star center
  * @param radiusInner Radius of inner star points
  * @param radiusOuter Radius of outer star points
  * @param color Color of star
  * @param numSteps Number of points of star (>2)
  * @param angle Number of degree to rotate the star by
  */
void triDrawStar( triFloat x, triFloat y, triFloat radiusInner, triFloat radiusOuter, triU32 color, triU32 numSteps, triFloat angle );

/** Draw Star with outline only
  * @param x X position of star center
  * @param y Y position of star center
  * @param radiusInner Radius of inner star points
  * @param radiusOuter Radius of outer star points
  * @param color Color of star
  * @param numSteps Number of points of star (>2)
  * @param angle Number of degree to rotate the star by
  */
void triDrawStarOutline( triFloat x, triFloat y, triFloat radiusInner, triFloat radiusOuter, triU32 color, triU32 numSteps, triFloat angle );

/** Draw Star with gradient
  * @param x X position of star center
  * @param y Y position of star center
  * @param radiusInner Radius of inner star points
  * @param radiusOuter Radius of outer star points
  * @param color1 Color of star center
  * @param color2 Color of star outer points
  * @param numSteps Number of points of star (>2)
  * @param angle Number of degree to rotate the star by
  */
void triDrawStarGrad( triFloat x, triFloat y, triFloat radiusInner, triFloat radiusOuter, triU32 color1, triU32 color2, triU32 numSteps, triFloat angle );


void triColorOp( triS32 op );
void trinoColorOp();

void triImageTint( triS32 mode, triS32 comp, triU32 vcolor, triU32 ccolor );
void triImageNoTint();
// const alpha overrides Tint mode!
void triImageConstAlpha( triU32 alpha );
void triImageBlend( triS32 op, triS32 srcOp, triS32 dstOp, triU32 src_fix, triU32 dst_fix );
void triImageNoBlend();
void triImageColorkey( triU32 color );
void triImageNoColorkey();


/** Set sprite mode.
  * @param width Width of all subsequent sprites to render
  * @param height Height of all subsequent sprites to render
  * @param angle Angle of all subsequent sprites to rotate
  */
void triSpriteMode( triFloat width, triFloat height, triFloat angle );

/** Render a sprite.
  * @param x Screen position to draw the sprite to
  * @param y Screen position to draw the sprite to
  * @param u Vertical sprite position to start drawing from (sprite sheet/tile map)
  * @param v Horizontal sprite position to start drawing from (sprite sheet/tile map)
  * @param img Image to draw
  */
void triDrawSprite( triFloat x, triFloat y, triFloat u, triFloat v, triImage* img );

/** Render a sprite without effects (no blending, transparency, etc.)
  * @param x Screen position to draw the sprite to
  * @param y Screen position to draw the sprite to
  * @param u Vertical sprite position to start drawing from (sprite sheet/tile map)
  * @param v Horizontal sprite position to start drawing from (sprite sheet/tile map)
  * @param img Image to draw
  *
  * @note This only works if sprite has same pixel format as the framebuffer.
  *       Else this will fall back to triDrawSprite.
  */
void triBltSprite( triFloat x, triFloat y, triFloat u, triFloat v, triImage* img );


/** Render an image centered at position x,y scaled by a factor.
  * This supports large images > 512x512.
  * @param x Screen position to center the image at
  * @param y Screen position to center the image at
  * @param scale Scale factor to scale image with (> 1.0 is zoom in, < 1.0 is zoom out)
  * @param img Image to draw
  */
void triDrawImageCenterScaled( triFloat x, triFloat y, triFloat scale, triImage* img );


/** Render an image scaled by a factor.
  * This supports large images > 512x512.
  * @param x Screen position to draw the image to
  * @param y Screen position to draw the image to
  * @param width Width of image on screen for scaling. A width < 0 flips image vertically.
  * @param height Height of image on screen for scaling. A height < 0 flips image horizontally.
  * @param img Image to draw
  */
void triDrawImageScaled( triFloat x, triFloat y, triFloat width, triFloat height, triImage* img );

/** Render an image at full size.
  * This supports large images > 512x512.
  * @param x Screen position to draw the image to
  * @param y Screen position to draw the image to
  * @param img Image to draw
  */
void triDrawImage2( triFloat x, triFloat y, triImage* img );

/** Render an image.
  * This supports large images > 512x512.
  * @param x Screen position to draw the image to
  * @param y Screen position to draw the image to
  * @param width Width of image on screen for scaling. A width < 0 flips image vertically.
  * @param height Height of image on screen for scaling. A height < 0 flips image horizontally.
  * @param u0 Vertical image position to start drawing from (sprite sheet/tile map)
  * @param v0 Horizontal image position to start drawing from (sprite sheet/tile map)
  * @param u1 Vertical image position to end drawing at (sprite sheet/tile map)
  * @param v1 Horizontal image position to end drawing at (sprite sheet/tile map)
  * @param img Image to draw
  */
void triDrawImage( triFloat x, triFloat y, triFloat width, triFloat height, 
			   triFloat u0, triFloat v0, triFloat u1, triFloat v1, 
			   triImage* img );

/** Render an image with rotation.
  * @param x Screen position to draw the image to
  * @param y Screen position to draw the image to
  * @param angle Angle to rotate the image by in degree
  * @param img Image to draw
  */
void triDrawImageRotate2( triFloat x, triFloat y, triFloat angle, triImage* img );

/** Render an image with rotation.
  * @param x Screen position to draw the image to
  * @param y Screen position to draw the image to
  * @param width Width of image on screen for scaling
  * @param height Height of image on screen for scaling
  * @param u0 Vertical image position to start drawing from (sprite sheet/tile map)
  * @param v0 Horizontal image position to start drawing from (sprite sheet/tile map)
  * @param u1 Vertical image position to end drawing at (sprite sheet/tile map)
  * @param v1 Horizontal image position to end drawing at (sprite sheet/tile map)
  * @param angle Angle to rotate the image by in degree
  * @param img Image to draw
  */
void triDrawImageRotate( triFloat x, triFloat y, triFloat width, triFloat height, 
			   triFloat u0, triFloat v0, triFloat u1, triFloat v1, 
			   triFloat angle, triImage* img );


/** Render an image animation.
  * @param x Screen position to draw the image to
  * @param y Screen position to draw the image to
  * @param ani Image animation to draw
  */
void triDrawImageAnimation( triFloat x, triFloat y, triImageAnimation* ani );

/** Render an image animation with rotation.
  * @param x Screen position to draw the image to
  * @param y Screen position to draw the image to
  * @param angle Angle to rotate the image by in degree
  * @param ani Image animation to draw
  */
void triDrawImageAnimationRotate( triFloat x, triFloat y, triFloat angle, triImageAnimation* ani );

/** @} */

#endif // __TRIGRAPHICS_H__
