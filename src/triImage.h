/*
 * triImage.h: Header for image loading/saving
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
 
#ifndef __TRIIMAGE_H__
#define __TRIIMAGE_H__

#include "triTypes.h"
#include "streams/streams.h"
#include "triHeap.h"


/** @defgroup triImage Image
 *  @{
 */

#ifdef __PSP__
#include <pspgu.h>

#define IMG_FORMAT_5650	GU_PSM_5650	/**< Pixelformat R5:G6:B5:A0. */
#define IMG_FORMAT_5551	GU_PSM_5551	/**< Pixelformat R5:G5:B5:A1. */
#define IMG_FORMAT_4444	GU_PSM_4444	/**< Pixelformat R4:G4:B4:A4. */
#define IMG_FORMAT_8888	GU_PSM_8888	/**< Pixelformat R8:G8:B8:A8. */
#define IMG_FORMAT_T4	GU_PSM_T4	/**< Pixelformat 4bit indexed. */
#define IMG_FORMAT_T8	GU_PSM_T8	/**< Pixelformat 8bit indexed. */
#define IMG_FORMAT_T16	GU_PSM_T16	/**< Pixelformat 16bit indexed. */
#define IMG_FORMAT_T32	GU_PSM_T32	/**< Pixelformat 32bit indexed. */
#define IMG_FORMAT_DXT1	GU_PSM_DXT1	/**< Pixelformat DXT1 compressed. */
#define IMG_FORMAT_DXT3	GU_PSM_DXT3	/**< Pixelformat DXT3 compressed. */
#define IMG_FORMAT_DXT5	GU_PSM_DXT5	/**< Pixelformat DXT5 compressed. */
#else // __PSP__
#define IMG_FORMAT_5650	0	/**< Pixelformat R5:G6:B5:A0. */
#define IMG_FORMAT_5551	1	/**< Pixelformat R5:G5:B5:A1. */
#define IMG_FORMAT_4444	2	/**< Pixelformat R4:G4:B4:A4. */
#define IMG_FORMAT_8888	3	/**< Pixelformat R8:G8:B8:A8. */
#define IMG_FORMAT_T4	4	/**< Pixelformat 4bit indexed. */
#define IMG_FORMAT_T8	5	/**< Pixelformat 8bit indexed. */
#define IMG_FORMAT_T16	6	/**< Pixelformat 16bit indexed. */
#define IMG_FORMAT_T32	7	/**< Pixelformat 32bit indexed. */
#define IMG_FORMAT_DXT1	8	/**< Pixelformat DXT1 compressed. */
#define IMG_FORMAT_DXT3	9	/**< Pixelformat DXT3 compressed. */
#define IMG_FORMAT_DXT5	10	/**< Pixelformat DXT5 compressed. */
#endif // __PSP__


// triImageLoad flags
#define TRI_RAM		0x0000	/**< Load image in system ram */
#define TRI_VRAM	0x0001	/**< Load image in VRAM */
#define TRI_SWIZZLE	0x0002	/**< Swizzle image */
// Bits 3-7 are reserved for later use
#define TRI_FRAME(x)	(((x) & ~0x7FF) << 11)	/**< Load frame number x [0-2097151] from file (only for .tri) */
#define TRI_LEVEL(x)	(((x) & 0x7) << 8)		/**< Load level number x [0-7] from file (only for .tri) */


// triImageChunkHeader/triImageSave flags
#define TRI_IMG_FLAGS_SWIZZLE	0x0001	/**< Swizzled image data */
#define TRI_IMG_FLAGS_RLE		0x0002	/**< RLE compressed image data */
#define TRI_IMG_FLAGS_GZIP		0x0004	/**< GZIP compressed image data */
#define TRI_IMG_FLAGS_WAVELET	0x0008	/**< Wavelet compressed image data (stores mipmaps as integral part of the actual image) */


#define PACKED __attribute__((packed))


/** Image File Header.
  */
typedef struct triImageFileHeader
{
	triChar			magic[8];		/**< "triImage". */
	triU32			numFrames;		/**< Number of frames for animations. */
	triU32			reserved;
} PACKED triImageFileHeader;	// 16 byte header


typedef struct triImageChunkHeader
{
	triU16			format;			/**< Image format - one of IMG_FORMAT_*. */
	triU16			palformat;		/**< Palette format - one of IMG_FORMAT_5650, IMG_FORMAT_5551, IMG_FORMAT_4444, IMG_FORMAT_8888. */
	triU16			flags;			/**< Is image swizzled/compressed. */
	triU16			numLevels;		/**< Number of mipmap levels stored in the image. */
	triU16			delay;			/**< Delay for image animation */
	triS16			xOffs;			// placement offset on screen
	triS16			yOffs;
	triU16			reserved;
} PACKED triImageChunkHeader;	// 16 byte Image Header


typedef struct triImageChunk
{
	triU32			width;			/**< Image width. */
	triU32			height;			/**< Image height. */
	triU32			stride;			/**< Image allocated width (power of two). */
	triU32			size;			/**< Size of data in bytes. */
} PACKED triImageChunk;		// 16 byte Image chunk


/* .tri File Format Spec:
 *
 * <triImageFileHeader> (tIH)
 *   numFrames times:
 *   <triImageChunkHeader> (tCH)
 *   [palette]
 *     numLevels+1 times:
 *     <triImageChunk> (tIC)
 *     <data>
 *
 * Palette is either 16 (format = IMG_FORMAT_T4) or 256 (format = IMG_FORMAT_T*) entries.
 * One palette entry is either 2 bytes (palformat = IMG_FORMAT_4444/5650/5551)
 * or 4 bytes ( palformat = IMG_FORMAT_8888).
 *
 * Data is tIC.size bytes of data, either in raw, rle compressed (tCH.flags&TRI_IMG_FLAGS_RLE)
 * or gzip compressed (tCH.flags&TRI_IMG_FLAGS_GZIP).
 *
 * A tIC represents one mipmap level for a texture. The size may vary, but the format and palette
 * must be the same for all levels.
 *
 * A tCH represents one animation frame in an animated image/texture. Each frame
 * can have a different format and palette.
 */


typedef struct triMipLevel
{
	triVoid			*data;
	triU32			size;
	triU16			width;
	triU16			height;
	triU16			stride;
	triU16			tex_height;
	
	struct triMipLevel*	next;
} PACKED triMipLevel;


/** Image struct. Max 1024xN.
  */
typedef struct triImage
{
	triVoid			*palette;		/**< Image palette. */
	triU32			palformat;		/**< Palette format - one of IMG_FORMAT_5650, IMG_FORMAT_5551, IMG_FORMAT_4444, IMG_FORMAT_8888. */

	triVoid			*data;			/**< Image data. */
	triU32			size;			/**< Size of data in bytes. */
	triU16			width;			/**< Image width. */
	triU16			height;			/**< Image height. */
	triU16			stride;			/**< Image allocated width (power of two). */
	triU16			tex_height;		/**< Image texture height (power of two). */
	triU16			format;			/**< Image format - one of IMG_FORMAT_*. */	
	triU8			bits;			/**< Image bits per pixel. */
	
	triU8			levels;			/**< Number of mipmap levels in image */
	triMipLevel		*level;			/**< Linked list of mipmap levels */
	
	triBool			swizzled;		/**< Is image swizzled. */
	
	triChar			filename[64];	// Needed for triModel saver to save filename within triModel
} PACKED triImage;	// 99 bytes


/** Image slice struct. Contains an image region of max 512x512 pixels.
  */
typedef struct triImageSlice
{
	triVoid			*data;			/**< Image data. */
	triU32			size;			/**< Size of data in bytes. */
	triU16			width;			/**< Image width. */
	triU16			height;			/**< Image height. */
	triU16			pitch;			/**< Image allocated width (power of two). */
} PACKED triImageSlice;


/** Large Image struct. Used to load and draw large images.
  */
typedef struct triImageLarge
{
	triVoid			*palette;		/**< Image palette. */
	triU32			palformat;		/**< Palette format - one of IMG_FORMAT_5650, IMG_FORMAT_5551, IMG_FORMAT_4444, IMG_FORMAT_8888. */

	triImageSlice	*data;			/**< Image slices. */
	triU16			vslices;		/**< Number of vertical slices. */
	triU16			hslices;		/**< Number of horizontal slices. */
	triU16			width;			/**< Image width. */
	triU16			height;			/**< Image height. */
	
	triU16			format;			/**< Image format - one of IMG_FORMAT_*. */	
	triU8			bits;			/**< Image bits per pixel. */
	
	triBool			swizzled;		/**< Is image swizzled. */
} PACKED triImageLarge;

/** @} */

// ImageList stays undocumented, as it's internal only
typedef struct triImageList
{
	triImage		*image;
	triS16			sx, sy;
	triS16			sw, sh;			// source rect of image (sprite sheets)
	
	triS16			xOffs;			// placement offset on screen
	triS16			yOffs;
	triU32			delay;			// delay of frame in timeBase resolution
	struct triImageList	*next;
} PACKED triImageList;	// 26 bytes


/** @addtogroup triImage Image
 *  @{
 */

/** ImageAnimation struct.
  */
typedef struct triImageAnimation
{
	triImage		*image;			/**< Pointer to current frame image for easy access. */


	triImageList	*frames;		/**< List of animation frames. */
	triImageList	*curFrame;		/**< Current animation frame. */
	triU32			numFrames;		/**< Number of frames. */

	// size of largest animation frame
	triU16			width;			/**< Maximum width of all frames. */
	triU16			height;			/**< Maximum height of all frames. */
	
	// use a global palette - not currently used
	triVoid			*palette;		/**< Global palette for all animation frames. */
	triU8			globalPalette;	/**< Animation frames use a global palette. */
	triU8			palformat;		/**< Global palette format - one of IMG_FORMAT_5650, IMG_FORMAT_5551, IMG_FORMAT_4444, IMG_FORMAT_8888. */


	triS16			loops;			/**< Number of loops. */
	triS16			loopsDone;		/**< Number of finished loops. */
	triS16			timeBase;		/**< Time base of delays in µs (default 1000 = 1ms). */
	u64				lastUpdate;		/**< Time of last update. */
	triBool			playing;		/**< Is animation playing? */
} PACKED triImageAnimation;	// 43 bytes



/** Get a palette entry from image.
  * @param *img - Pointer to image containing the palette
  * @param col - Palette entry index
  * @param *r - Pointer to U32 to contain the red component (0-255)
  * @param *g - Pointer to U32 to contain the green component (0-255)
  * @param *b - Pointer to U32 to contain the blue component (0-255)
  */
triVoid triImagePaletteGet( triImage* img, triU32 col, triU32 *r, triU32 *g, triU32 *b, triU32 *a );

/** Set a palette entry in image.
  * @param *img - Pointer to image containing the palette
  * @param col - Palette entry index
  * @param r - Red component (0-255)
  * @param g - Green component (0-255)
  * @param b - Blue component (0-255)
  */
triVoid triImagePaletteSet( triImage* img, triU32 col, triU32 r, triU32 g, triU32 b, triU32 a );

/** Swizzle and upload an image to VRAM for faster drawing.
  * @param *img - Pointer to image
  */
triVoid triImageSwizzleToVRAM( triImage *img );

/** Swizzle an image for faster drawing.
  * @param *img - Pointer to image
  */
triVoid triImageSwizzle( triImage *img );

/** Unswizzle an image.
  * @param *img - Pointer to image
  */
triVoid triImageUnswizzle( triImage *img );

/** Upload an image to VRAM for faster drawing.
  * @param *img - Pointer to image
  */
triVoid triImageToVRAM( triImage *img );

/** Download an image to RAM.
  * @param *img - Pointer to image
  */
triVoid triImageToRAM( triImage *img );

/** Free an image.
  * @param *img - Pointer to image
  */
triVoid triImageFree( triImage *img );

/** Free an image animation.
  * @param *img - Pointer to image animation
  */
triVoid triImageAnimationFree( triImageAnimation *img );


/** Capture part of framebuffer to image.
  * @param x - X-coordinate of rect to capture
  * @param y - Y-coordinate of rect to capture
  * @param width - Width of rect to capture
  * @param height - Height of rect to capture
  * @returns Pointer to image containing the framebuffer part
  */
triImage* triImageGet( triS32 x, triS32 y, triS32 width, triS32 height );

/** Create an image from a memory buffer.
  * @param width - Width of image
  * @param height - Height of image
  * @param stride - Allocated width of image (buffer width)
  * @param bits - Bits of image
  * @param psm - Format of image - one of IMG_FORMAT_*
  * @param data - Pointer to memory containing the image data
  * @returns Pointer to image structure containing the data
  */
triImage* triImageSet( triS32 width, triS32 height, triS32 stride, triS32 bits, triS32 psm, triVoid* data );

#ifdef TRI_SUPPORT_PNG
/** Load image from a .png file.
  * @param name - Filename of file to load
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadPng( triChar* name );

/** Load PNG image from a stream
  * @param s - Opened stream to read from
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadPngStream( stream* s );
#endif // TRI_SUPPORT_PNG

/** Load image from a .raw file.
  * @param name - Filename of file to load
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadRaw( triChar* name );

/** Load RAW image from a stream
  * @param s - Opened stream to read from
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadRawStream( stream* s );

/** Load image from a .tga file.
  * @param name - Filename of file to load
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadTga( triChar* name );

/** Load TGA image from a stream
  * @param s - Opened stream to read from
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadTgaStream( stream* s );


/** Load image from a .tri file.
  * @param name - Filename of file to load
  * @param frame - Frame number of file to load
  * @param level - Sublevel to load (0-7)
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadTri( triChar* name, triU32 frame );

/** Load TRI image from a stream
  * @param s - Opened stream to read from
  * @param frame - Frame number of file to load
  * @param level - Sublevel to load (0-7)
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadTriStream( stream* s, triU32 frame );


/** Generic file loader. Loads .tga, .tri and .png.
  * @param name - Filename of file to load
  * @param flags - flags to apply to the image (one of TRI_RAM/TRI_VRAM/TRI_SWIZZLE)
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoad( triChar* name, triU32 flags );

/** Generic stream file loader. Loads .tga, .tri and .png.
  * @param s - Stream to load from
  * @param flags - flags to apply to the image (one of TRI_RAM/TRI_VRAM/TRI_SWIZZLE)
  * @returns Pointer to image structure containing the image
  */
triImage* triImageLoadStream( stream* s, triU32 flags );


#ifdef TRI_SUPPORT_PNG
/** Save image to a .png file.
  * @param name - Filename of file to save
  * @param img - Pointer to image to save
  * @param saveAlpha - Whether to save the alpha channel with the image
  */
triVoid triImageSavePng( triChar* name, triImage *img, triS32 saveAlpha );
#endif // TRI_SUPPORT_PNG

/** Save image to a .tga file.
  * @param name - Filename of file to save
  * @param img - Pointer to image to save
  * @param saveAlpha - Whether to save the alpha channel with the image
  * @param rle - Whether to RLE compress image
  */
triVoid triImageSaveTga( triChar* name, triImage *img, triS32 saveAlpha, triS32 rle );

/** Save image to a .tri file.
  * @param name - Filename of file to save
  * @param img - Pointer to image to save
  * @param flags - any combination of TRI_IMG_FLAGS_*
  */
triVoid triImageSaveTri( triChar* name, triImage *img, triU32 flags );


/** Create a new blank triImageAnimation structure.
  * @returns Pointer to new triImageAnimation struct
  */
triImageAnimation* triImageAnimationCreate();

/** Create an animation from an imagesheet in order left-right, top-down.
  * @param img - Pointer to triImage containing spritesheet
  * @param fwidth - Width of one frame in sheet
  * @param fheight - Height of one frame in sheet
  * @param hframes - Number of horizontal frames
  * @param vframes - Number of vertical frames
  * @param delay - Delay to apply per frame, given in ms
  * @returns Pointer to new triImageAnimation struct
  */
triImageAnimation* triImageAnimationFromSheet( triImage* img, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay );

/** Create an animation from an imagesheet in order left-right, top-down.
  * @param img - Pointer to triImage containing spritesheet
  * @param xoffs - X offset where the frames should start
  * @param yoffs - Y offset where the frames should start
  * @param fwidth - Width of one frame in sheet
  * @param fheight - Height of one frame in sheet
  * @param hframes - Number of horizontal frames
  * @param vframes - Number of vertical frames
  * @param delay - Delay to apply per frame, given in ms
  * @returns Pointer to new triImageAnimation struct
  */
triImageAnimation* triImageAnimationFromSheet2( triImage* img, triS32 xoffs, triS32 yoffs, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay );

/** Create an animation from an imagesheet in a Tga file, in order left-right, top-down.
  * @param name - Filename of Tga image to load from.
  * @param fwidth - Width of one frame in sheet
  * @param fheight - Height of one frame in sheet
  * @param hframes - Number of horizontal frames
  * @param vframes - Number of vertical frames
  * @param delay - Delay to apply per frame, given in ms
  * @returns Pointer to new triImageAnimation struct
  */
triImageAnimation* triImageAnimationFromSheetTga( triChar* name, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay );

#ifdef TRI_SUPPORT_PNG
/** Create an animation from an imagesheet in a Png file, in order left-right, top-down.
  * @param name - Filename of Png image to load from.
  * @param fwidth - Width of one frame in sheet
  * @param fheight - Height of one frame in sheet
  * @param hframes - Number of horizontal frames
  * @param vframes - Number of vertical frames
  * @param delay - Delay to apply per frame, given in ms
  * @returns Pointer to new triImageAnimation struct
  */
triImageAnimation* triImageAnimationFromSheetPng( triChar* name, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay );
#endif // TRI_SUPPORT_PNG

/** Create an animation from an imagesheet in a file, in order left-right, top-down.
  * @param name - Filename of image to load from.
  * @param fwidth - Width of one frame in sheet
  * @param fheight - Height of one frame in sheet
  * @param hframes - Number of horizontal frames
  * @param vframes - Number of vertical frames
  * @param delay - Delay to apply per frame, given in ms
  * @returns Pointer to new triImageAnimation struct
  */
triImageAnimation* triImageAnimationFromSheetFile( triChar* name, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay );

/** Create an animation from an imagesheet in a file, in order left-right, top-down.
  * @param name - Filename of image to load from.
  * @param xoffs - X offset where the frames should start
  * @param yoffs - Y offset where the frames should start
  * @param fwidth - Width of one frame in sheet
  * @param fheight - Height of one frame in sheet
  * @param hframes - Number of horizontal frames
  * @param vframes - Number of vertical frames
  * @param delay - Delay to apply per frame, given in ms
  * @returns Pointer to new triImageAnimation struct
  */
triImageAnimation* triImageAnimationFromSheetFile2( triChar* name, triS32 xoffs, triS32 yoffs, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay );


/** Append a loaded triImage as last frame to an animation.
  * @param ani - Pointer to triAnimationImage to append the image to
  * @param img - Pointer to triImage to append
  * @param sx - X-coordinate of source image rect
  * @param sy - Y-coordinate of source image rect
  * @param sw - Width of source image rect
  * @param sh - Height of source image rect
  * @param x_offs - Offset in X-coordinate to draw the image
  * @param y_offs - Offset in Y-coordinate to draw the image
  * @param delay - Delay to apply per frame, given in ms
  */
triVoid triImageAnimationAppend( triImageAnimation* ani, triImage* img, triS32 sx, triS32 sy, triS32 sw, triS32 sh, triS32 x_offs, triS32 y_offs, triU32 delay );

/** Append a loaded triImage as last frame to an animation.
  * @param ani - Pointer to triAnimationImage to append the image to
  * @param img - Pointer to triImage to append
  * @param delay - Delay to apply per frame, given in ms
  */
triVoid triImageAnimationAppend2( triImageAnimation* ani, triImage* img, triU32 delay );

/** Append a loaded triImage as last frame to an animation.
  * @param ani - Pointer to triAnimationImage to append the image to
  * @param img - Pointer to triImage to append
  * @param x_offs - Offset in X-coordinate to draw the image
  * @param y_offs - Offset in Y-coordinate to draw the image
  * @param delay - Delay to apply per frame, given in ms
  */
triVoid triImageAnimationAppend3( triImageAnimation* ani, triImage* img, triS32 xOffs, triS32 yOffs, triU32 delay );


triImageAnimation* triImageAnimationLoadGif( triChar* name );
triImageAnimation* triImageAnimationLoadGifStream( stream* s );


/** Load image animation from a .tri file.
  * @param name - Filename of file to load
  * @returns Pointer to triImageAnimation structure containing the image animation
  */
triImageAnimation* triImageAnimationLoadTri( triChar* name );

/** Load image animation from a stream.
  * @param s - Opened stream to read from
  * @returns Pointer to triImageAnimation structure containing the image animation
  */
triImageAnimation* triImageAnimationLoadTriStream( stream* s );

/** Save image animation to a .tri file.
  * @param name - Filename of file to save
  * @param ani - Pointer to triImageAnimation structure to save
  * @param flags - any combination of TRI_IMG_FLAGS_*
  */
void triImageAnimationSaveTri( triChar* name, triImageAnimation* ani, triS32 flags );


/** Start animation.
  * @param ani - Pointer to triAnimationImage
  */
triVoid triImageAnimationStart( triImageAnimation* ani );

/** Pause animation.
  * @param ani - Pointer to triAnimationImage
  */
triVoid triImageAnimationPause( triImageAnimation* ani );

/** Reset animation (Stop+Rewind).
  * @param ani - Pointer to triAnimationImage
  */
triVoid triImageAnimationReset( triImageAnimation* ani );

/** Update animation.
  * @param ani - Pointer to triAnimationImage
  * @returns 0 if animation wasn't updated
  */
triS32 triImageAnimationUpdate( triImageAnimation* ani );

/** Set animation speed.
  * @param ani - Pointer to triAnimationImage
  * @param factor - Float specifying the speed - 1.0 = normal speed, 2.0 = double speed
  */
triVoid triImageAnimationSetSpeed( triImageAnimation* ani, triFloat factor );

/** Return if animation is finished.
  * @param ani - Pointer to triAnimationImage
  * @returns boolean indicating the state
  */
triBool triImageAnimationIsDone( triImageAnimation* ani );

/** Return one frame from animation.
  * @param ani - Pointer to triAnimationImage
  * @param nFrame - Number of Frame to return (starting at 0)
  * @returns Pointer to animation image
  */
triImage* triImageAnimationGetFrame( triImageAnimation* ani, triS32 nFrame );

/** @} */

#endif
