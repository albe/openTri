/*
 * triImage.c: Code for image loading/saving
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

#ifdef __PSP__
#include <pspkernel.h>
#include <pspgu.h>
#include <psptypes.h>
#include <psprtc.h>
#endif

#include <stdio.h>
#include <string.h>
#include <zlib.h>


#ifdef TRI_SUPPORT_PNG
#include <png.h>
#endif // TRI_SUPPORT_PNG

#include "rle.h"
#include "tga.h"
#include "triMemory.h"
#include "triVAlloc.h"
#include "triImage.h"
#include "triGraphics.h"
#include "triRefcount.h"
#include "triLog.h"
#include "triHeap.h"


// Heap for image structure headers
#define TRI_IMAGE_HEAP_SIZE	(4*1024)
/*static triU8	triImageStaticHeap[TRI_IMAGE_HEAP_SIZE];
static	triHeap*	triImageHeap = 0;*/
TRI_HEAP( triImageHeap, TRI_IMAGE_HEAP_SIZE, 16 )


#define RGBA5650(col,r,g,b,a)	{ \
		a=0xFF;\
		b=(((col>>11)&0x1F)*255/31);\
		g=(((col>>5)&0x3F)*255/63);\
		r=((col&0x1F)*255/31);\
		}
#define RGBA5551(col,r,g,b,a)  { \
		a=((col>>15)==0?0:0xFF);\
		b=(((col>>10)&0x1F)*255/31);\
		g=(((col>>5)&0x1F)*255/31);\
		r=((col&0x1F)*255/31);\
		}
#define RGBA4444(col,r,g,b,a)	{ \
		a=(((col>>12)&0xF)*255/15);\
		b=(((col>>8)&0xF)*255/15);\
		g=(((col>>4)&0xF)*255/15);\
		r=((col&0xF)*255/15);\
		}
#define RGBA8888(col,r,g,b,a)	{ \
		a=(col>>24)&0xFF;\
		b=(col>>16)&0xFF;\
		g=(col>>8)&0xFF;\
		r=(col&0xFF);\
		}

#define COL5650(r,g,b,a)	((r>>3) | ((g>>2)<<5) | ((b>>3)<<11))
#define COL5551(r,g,b,a)	((r>>3) | ((g>>3)<<5) | ((b>>3)<<10) | (a>0?0x7000:0))
#define COL4444(r,g,b,a)	((r>>4) | ((g>>4)<<4) | ((b>>4)<<8) | ((a>>4)<<12))
#define COL8888(r,g,b,a)	((r) | ((g)<<8) | ((b)<<16) | ((a)<<24))


triVoid triImageFree( triImage *img )
{
	if (img==0) return;
	if (triRefcountRelease( img )!=0) return;
	if (img->data!=0)
	{
		if ((triU32)img->data>=0x4000000 && (triU32)img->data<=0x4200000)
			triVFree(img->data);
		else
			triFree(img->data);
	}
	img->data = 0;
	img->width = 0;
	img->height = 0;
	img->stride = 0;
	img->swizzled = 0;
	img->format = 0;
	if (img->palette!=0) triFree(img->palette);
	img->palette = 0;
	img->palformat = 0;
	
	triMipLevel* lev = img->level;
	triMipLevel* next = 0;
	while (lev!=0)
	{
		next = lev->next;
		if ((triU32)lev->data>=0x4000000 && (triU32)lev->data<=0x4200000)
			triVFree(lev->data);
		else
			triFree(lev->data);
		triFree(lev);
		lev = next;
	}
	triFree(img);
}


triVoid triImageAnimationFree( triImageAnimation *img )
{
	if (img==0) return;
	img->width = 0;
	img->height = 0;
	img->loops = 0;
	if (img->palette!=0) triFree(img->palette);
	img->palette = 0;
	img->palformat = 0;
	triImageList *next = 0;
	while (img->frames!=0)
	{
		next = img->frames->next;
		triImageFree( img->frames->image );
		triFree( img->frames );
		img->frames = next;
	}
	triFree(img);
}



triVoid triImagePaletteGet( triImage* img, triU32 col, triU32 *r, triU32 *g, triU32 *b, triU32 *a )
{
	if (img==0) return;
	if (img->palette==0) return;
	
	triU16 col16;
	
	triU8* pal = img->palette;
	switch (img->palformat)
	{
		case IMG_FORMAT_5650:
			col16 = (pal[col*2] << 8) | (pal[col*2+1]);
			RGBA5650(col16,*r,*g,*b,*a);
			break;
		case IMG_FORMAT_5551:
			col16 = (pal[col*2] << 8) | (pal[col*2+1]);
			RGBA5551(col16,*r,*g,*b,*a);
			break;
		case IMG_FORMAT_4444:
			col16 = (pal[col*2] << 8) | (pal[col*2+1]);
			RGBA4444(col16,*r,*g,*b,*a);
			break;
		case IMG_FORMAT_8888:
			*r = pal[col*4];
			*g = pal[col*4+1];
			*b = pal[col*4+2];
			*a = pal[col*4+3];
			break;
	}
}


triVoid triImagePaletteSet( triImage* img, triU32 col, triU32 r, triU32 g, triU32 b, triU32 a )
{
	if (img==0) return;
	if (img->palette==0) return;
	
	triU16 col16;

	triU8* pal = img->palette;
	switch (img->palformat)
	{
		case IMG_FORMAT_5650:
			col16 = COL5650(r,g,b,a);
			pal[col*2] = col16 >> 8;
			pal[col*2+1] = col16 & 0xFF;
			break;
		case IMG_FORMAT_5551:
			col16 = COL5551(r,g,b,a);
			pal[col*2] = col16 >> 8;
			pal[col*2+1] = col16 & 0xFF;
			break;
		case IMG_FORMAT_4444:
			col16 = COL4444(r,g,b,a);
			pal[col*2] = col16 >> 8;
			pal[col*2+1] = col16 & 0xFF;
			break;
		case IMG_FORMAT_8888:
			*((triU32*)img->palette+col) = COL8888(r,g,b,a);
			break;
	}
}



#define MIN(_a,_b) (_a<_b?_a:_b)
#define MAX(_a,_b) (_a>_b?_a:_b)
#define CLAMP(_a,_min,_max) MIN(_max,MAX(_a,_min))
#define WRAP(_a,_min,_max) (_a>_max?_a-_max+_min:(_a<_min?_a+_max-_min:_a))

static triVoid unswizzle_fast(const triU8* out, const triU8* in, const triS32 width, const triS32 height)
{
	triS32 blockx, blocky;
	triS32 j;
	
	triS32 width_blocks = (width / 16);
	triS32 height_blocks = (height / 8);
	
	triS32 dst_pitch = (width-16)/4;
	triS32 dst_row = width * 8;
	
	triU32* src = (triU32*)in;
	triU8* ydst = (triU8*)out;
	sceKernelDcacheWritebackAll();
	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		triU8* xdst = ydst;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			triU32* dst;
			if ((triU32)out <= 0x04200000)
				dst = (triU32*)((triU32)xdst | 0x40000000);
			else
				dst = (triU32*)xdst;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				dst += dst_pitch;
			}
			xdst += 16;
		}
		ydst += dst_row;
	}
}

static triVoid swizzle_fast(const triU8 *dest, const triU8 *source, const triS32 width, const triS32 height)
{
	triS32 i,j;
	triS32 rowblocks = (width / 16);
	triS32 rowblocks_add = (rowblocks-1)*128;
	triU32 block_address = 0;
	triU32 *src = (triU32*)source;

	// (j-blocky*8)*16 + (i-blockx*16) + (blockx + blocky*rowblocks)*16*8 =
	// j*16 + (blocky*rowblocks-blocky)*16*8 + i + (blockx*8-blockx)*16 =   | blocky8 := j/8*8 = j&~0x7, blockx16 := i/16*16 = i&~0xf
	// j*16 + blocky8*(rowblocks-1)*16 + i + blockx16*8-blockx16 =   | k = 0..15, rowmul := (rowblocks-1)*16
	// j*16 + blocky8*rowmul + k + blockx16 + blockx16*8-blockx16 =
	//sceKernelDcacheWritebackAll();
	for (j = 0; j < height; j++,block_address+=16)
	{
		triU32 *block;
		/*if ((triU32)dest <= 0x04200000)
			block = (triU32*)((triU32)&dest[block_address] | 0x40000000);
		else*/
			block = (triU32*)&dest[block_address];
		for (i = 0; i < rowblocks; i++)
		{
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			block += 28;
		}

		if ((j&0x7)==0x7)
			block_address += rowblocks_add;
	}
}


triVoid triImageSwizzleToVRAM( triImage *img )
{
	if (img == 0 || img->width==0 || img->height==0 || img->data==0 || (triU32)img->data<=0x4200000) return;
	
	triU32* dst = (triU32*)triVAlloc(img->size);
	if (dst==0) return;
	triS32 bytewidth = img->stride*(img->bits>>3);
	swizzle_fast( (triU8*)((triU32)dst|0x40000000), (triU8*)img->data, bytewidth, img->size / bytewidth );
	triFree(img->data);
	img->data = dst;
	img->swizzled = 1;
	
	triMipLevel* lev = img->level;
	while (lev!=0)
	{
		bytewidth >>= 1;
		if ((triU32)lev->data<0x4000000 || (triU32)lev->data>0x4800000)
		{
			triU32* dst = (triU32*)triVAlloc(lev->size);
			if (dst==0) return;
			swizzle_fast( (triU8*)((triU32)dst|0x40000000), lev->data, bytewidth, lev->size / bytewidth );
			triFree(lev->data);
			lev->data = dst;
		}
		lev = lev->next;
	}
}


triVoid triImageSwizzle( triImage *img )
{
	if (img == 0 || img->swizzled==1 || img->width==0 || img->height==0 || img->data==0) return;
	
	triU32* dst = (triU32*)triMalloc(img->size);
	if (dst==0) return;
	triS32 bytewidth = img->stride*(img->bits>>3);
	swizzle_fast( (triU8*)dst, (triU8*)img->data, bytewidth, img->size / bytewidth );
	triFree(img->data);
	img->data = dst;
	img->swizzled = 1;
	
	triMipLevel* lev = img->level;
	while (lev!=0)
	{
		if ((triU32)lev->data<0x4000000 || (triU32)lev->data>0x4800000)
		{
			triU32* dst = (triU32*)triMalloc(lev->size);
			if (dst==0) return;
			triS32 bytewidth = lev->stride*(img->bits>>3);
			swizzle_fast( (triU8*)dst, (triU8*)lev->data, bytewidth, lev->size / bytewidth );
			triFree(lev->data);
			lev->data = dst;
		}
		lev = lev->next;
	}
	sceKernelDcacheWritebackAll();
}


triVoid triImageUnswizzle( triImage *img )
{
	if (img == 0 || img->swizzled==0 || img->width==0 || img->height==0 || img->data==0) return;
	
	triU32* dst = (triU32*)triMalloc(img->size);
	if (dst==0) return;
	triS32 bytewidth = img->stride*(img->bits>>3);
	unswizzle_fast( (triU8*)dst, (triU8*)img->data, bytewidth, img->size / bytewidth );
	triFree(img->data);
	img->data = dst;
	img->swizzled = 0;
	sceKernelDcacheWritebackAll();
}


triVoid triImageToVRAM( triImage *img )
{
	if (img == 0 || img->width==0 || img->height==0 || img->data==0 || (triU32)img->data<=0x4200000) return;
	
	triU32* dst = (triU32*)triVAlloc(img->size);
	if (dst==0) return;
	memcpy( (triU8*)((triU32)dst|0x40000000), img->data, img->size );
	triFree(img->data);
	img->data = dst;
	
	triMipLevel* lev = img->level;
	while (lev!=0)
	{
		if ((triU32)lev->data<0x4000000 || (triU32)lev->data>0x4800000)
		{
			triU32* dst = (triU32*)triVAlloc(lev->size);
			if (dst==0) return;
			memcpy( (triU8*)((triU32)dst|0x40000000), lev->data, lev->size );
			triFree(lev->data);
			lev->data = dst;
		}
		lev = lev->next;
	}
}


triVoid triImageToRAM( triImage *img )
{
	if (img == 0 || img->width==0 || img->height==0 || img->data==0 || (triU32)img->data>=0x4200000) return;
	
	triU32* dst = (triU32*)triMalloc(img->size);
	if (dst==0) return;
	memcpy( dst, (triU8*)((triU32)img->data|0x40000000), img->size );
	triVFree(img->data);
	img->data = dst;
	
	triMipLevel* lev = img->level;
	while (lev!=0)
	{
		if ((triU32)lev->data>=0x4000000 && (triU32)lev->data<=0x4200000)
		{
			triU32* dst = (triU32*)triMalloc(lev->size);
			if (dst==0) return;
			memcpy( dst, (triU8*)((triU32)lev->data|0x40000000), lev->size );
			triVFree(lev->data);
			lev->data = dst;
		}
		lev = lev->next;
	}
	sceKernelDcacheWritebackAll();
}



// Return next power of 2
static triS32 next_pow2(triU32 w)
{
	w -= 1;		// To not change already power of 2 values
	w |= (w >> 1);
	w |= (w >> 2);
	w |= (w >> 4);
	w |= (w >> 8);
	w |= (w >> 16);
	return(w+1);
}


// Return next multiple of 8 (needed for swizzling)
static triS32 next_mul8(triU32 w)
{
	return((w+7)&~0x7);
}

static triS32 next_mul(triU32 w, triU32 n)
{
	return((w+(n-1))&~(n-1));
}


// capture a part of the framebuffer into an triImage structure
triImage* triImageGet( triS32 x, triS32 y, triS32 width, triS32 height )
{
	if (width<=0) width = 480;
	if (height<=0) height = 272;
	if (width>512) width = 512;
	if (height>272) height = 272;
	
	triImage* img = triMalloc( sizeof(triImage) );
	if (img==0) return(0);
	
	img->level = 0;
	img->levels = 0;
	img->palette = 0;
	img->width = width;
	img->height = height;
	img->bits = triBpp<<3;
	img->format = triPsm;
	img->swizzled = 0;
	
	img->tex_height = next_pow2(height);
	img->stride = next_pow2(width);
	img->size = img->stride*next_mul8(img->height)*(img->bits>>3);
	img->data = triMalloc(img->size);
	if (img->data==0)
		return(0);

	triU8* src = (triU8*)(((triU32)triFramebuffer) + (x + y*FRAME_BUFFER_WIDTH)*triBpp);
	triU8* dst = (triU8*)img->data;
	
	triS32 yy;
	for (yy=0;yy<height;yy++)
	{
		memcpy(dst,src,img->width*triBpp);
		dst += img->stride*triBpp;
		src += FRAME_BUFFER_WIDTH*triBpp;
	}

	return(img);
}


triImage* triImageSet( triS32 width, triS32 height, triS32 stride, triS32 bits, triS32 psm, triVoid* data )
{
	triImage* img = triMalloc( sizeof(triImage) );
	if (img==0 || data==0) return(0);
	
	img->level = 0;
	img->levels = 0;
	img->palette = 0;
	img->width = width;
	img->height = height;
	img->bits = bits;
	img->format = psm;
	img->swizzled = 0;
	
	img->tex_height = next_pow2(height);
	img->stride = stride;
	img->size = img->stride*next_mul8(height)*(img->bits>>3);
	img->data = data;

	return(img);
}


#ifdef TRI_SUPPORT_PNG
triVoid user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	// ignore PNG warnings
}


triVoid user_read_fn(png_structp png_ptr, png_bytep ptr, png_size_t size)
{
	stream* s = (stream*)png_get_io_ptr( png_ptr );
	stream_read( s, ptr, size );
}


static triChar* _triImageLoadPngStream( stream* s, triImage *img )
{
	if (img == 0) return("Image is NULL.");
	if (s == 0) return("Stream is NULL.");
	
	img->data = img->palette = 0;
	
	png_structp png_ptr;
	png_infop info_ptr;
	triU32 sig_read = 0;
	png_uint_32 width, height;
	png_uint_32 bit_depth, color_type, interlace_type, y;

	triU8 magic[8];
	sig_read = stream_read( s, magic, 8 );
	/*if (!png_sig_cmp(magic,0,8))
	{
		return("Not a png file!.");
	}*/
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		return("png_create_read_struct failed.");
	}
	
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return("png_create_info_struct failed.");
	}
	
	png_set_read_fn(png_ptr, (png_voidp)s, user_read_fn);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	
	triS32 passes = 1;
	if (interlace_type)
		passes = png_set_interlace_handling(png_ptr);

	triS32 i;
	switch (color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			img->format = IMG_FORMAT_T8;
			img->bits = 8;
			if (bit_depth<8) png_set_gray_1_2_4_to_8(png_ptr);
			img->palette = triMalloc(256*4);
			img->palformat = IMG_FORMAT_8888;
			if (img->palette==0)
			{
				png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
				return("malloc failed on palette.");
			}
			for (i=0;i<256;i++)
				((triS32*)img->palette)[i] = ((0xFF << 24) | (i << 16) | (i << 8) | i);
			break;

		case PNG_COLOR_TYPE_PALETTE:
			img->format = IMG_FORMAT_T8;
			img->bits = 8;
			img->palette = triMalloc(256*4);
			img->palformat = IMG_FORMAT_8888;
			if (img->palette==0)
			{
				png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
				return("malloc failed on palette.");
			}
			
			png_colorp palette = NULL;
			png_bytep trans = NULL;
			triS32 num_palette = 0;
			triS32 num_trans = 0;
			png_color_16p trans_values; // for non-PLTE triImages
			png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
			png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values);
			triS32 col;
			for (i = 0; i < num_palette; i++) {
				col = (palette[i].red) | (palette[i].green << 8) | (palette[i].blue << 16);
				if (trans != NULL && i < num_trans)
				{
					col |= trans[i] << 24;
				}
				else
				{
					col |= 0xff000000;
				}
				((triS32*)img->palette)[i] = col;
			}
			break;
		
		case PNG_COLOR_TYPE_RGB:
			if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
				png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			else
				png_set_tRNS_to_alpha(png_ptr);
		case PNG_COLOR_TYPE_RGB_ALPHA:
			img->format = IMG_FORMAT_8888;
			img->bits = 32;
			break;
	
		default:
			png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
			return("Unknown color type.");
	}

	img->level = 0;
	img->levels = 0;
	img->swizzled = 0;
	img->width = width;
	img->height = height;
	img->tex_height = next_pow2(height);
	img->stride = next_pow2(width);
	//if (img->stride>512) img->stride = next_mul(width,512);
	img->size = img->stride*next_mul8(img->height)*(img->bits>>3);
	img->data = triMalloc(img->size);
	if (img->data==0)
	{
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return("malloc failed on img->data.");
	}
	
	for (i = 0; i < passes; i++)
	{
		triU8* dst = (triU8*)img->data;
		for (y = 0; y < height; y++)
		{
			png_read_row(png_ptr, (triU8*)dst, png_bytep_NULL);
			dst += (img->stride)*(img->bits>>3);
		}
	}

//	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	strncpy( img->filename, stream_name( s ), 64 );
	return(0);
}


triImage* triImageLoadPng( triChar* name )
{
	triVoid* img = triRefcountRetain( name );
	if (img==0)
	{
		stream* s = stream_fopen( name, STREAM_RDONLY ); //stream_bufopen( STREAM_TYPE_FILE, name, STREAM_RDONLY );
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadPngStream( s, i ))!=0)
		{
			stream_close( s );
			triLogError("triImageLoadPng: %s\n",ret);
			triFree(i);
			return(0);
		}
		stream_close( s );
		triRefcountCreate( name, i );
		return(i);
	}
	else
		return((triImage*)img);
}



triImage* triImageLoadPngStream( stream* s )
{
	triVoid* img = triRefcountRetain( stream_name( s ) );
	if (img==0)
	{
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadPngStream( s, i ))!=0)
		{
			triLogError("triImageLoadPngStream: %s\n",ret);
			triFree(i);
			return(0);
		}
		triRefcountCreate( stream_name( s ), i );
		return(i);
	}
	else
		return((triImage*)img);
}
#endif // TRI_SUPPORT_PNG



static triChar* _triImageLoadRawStream( stream* s, triImage *img )
{
	if (img == 0) return("triImage is NULL.");
	if (s == 0) return("stream is NULL.");
	
	img->size = stream_size( s );
	img->data = triMalloc( img->size );

	if (img->data==0)
	{
		return("malloc failed on img->data.");
	}

	img->palette = 0;
	img->palformat = 0;
	img->level = 0;
	img->levels = 0;
	//img->width = width;
	//img->height = height;
	//img->tex_height = next_pow2(height);
	//img->stride = next_pow2(width);
	//if (img->stride>512) img->stride = next_mul8(width);
	img->swizzled = 0;

	stream_read( s, img->data, img->size );
	return 0;
}


triImage* triImageLoadRaw( triChar* name )
{
	triVoid* img = triRefcountRetain( name );
	if (img==0)
	{
		stream* s = stream_fopen( name, STREAM_RDONLY ); //stream_bufopen( STREAM_TYPE_FILE, name, STREAM_RDONLY );
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadRawStream( s, i ))!=0)
		{
			stream_close( s );
			triLogError("triImageLoadRaw: %s\n",ret);
			triFree(i);
			return(0);
		}
		stream_close( s );
		triRefcountCreate( name, i );
		return(i);
	}
	else
		return((triImage*)img);
}


triImage* triImageLoadRawStream( stream* s )
{
	triVoid* img = triRefcountRetain( stream_name( s ) );
	if (img==0)
	{
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadRawStream( s, i ))!=0)
		{
			triLogError("triImageLoadRawStream: %s\n",ret);
			triFree(i);
			return(0);
		}
		triRefcountCreate( stream_name( s ), i );
		return(i);
	}
	else
		return((triImage*)img);
}


#define RD16(x) (triU16)((triU16)x[0] | ((triU16)x[1] << 8))

static triChar* _triImageLoadTgaStream( stream* s, triImage *img )
{
	if (img == 0) return("triImage is NULL.");
	if (s == 0) return("stream is NULL.");
	
	TGAFILEHEADER tfh;
    triU8 pal[256*4];
    
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );

    triS32 fpos = 0;
    stream_read( s, &tfh, sizeof(TGAFILEHEADER) );
    fpos += sizeof(TGAFILEHEADER)+tfh.ImageIDSize;
    stream_seek( s, fpos, STREAM_SET );
    #ifdef DEBUG
	triLogPrint("TGAHEADER (%i)>\n", sizeof(TGAFILEHEADER));
	triLogPrint("triImageIDSize: %i\n", tfh.ImageIDSize);
    triLogPrint("ColorMapType: %i\n", tfh.ColorMapType);
    triLogPrint("ImageTypeCode: %i\n", tfh.ImageTypeCode);                          // triImage Type (normal/paletted/grayscale/rle)
    triLogPrint("ColorMapOrigin: %i\n", RD16(tfh.ColorMapOrigin));
    triLogPrint("ColorMapLength: %i\n", RD16(tfh.ColorMapLength));                         // Palette Size
    triLogPrint("ColorMapESize: %i\n", tfh.ColorMapESize);							// Size in bits of one Palette entry
	triLogPrint("OrigX: %i\n", RD16(tfh.OriginX));                                  // Width of triImage
    triLogPrint("OrigY: %i\n", RD16(tfh.OriginY));
    triLogPrint("Width: %i\n", RD16(tfh.Width));                                  // Width of triImage
    triLogPrint("Height: %i\n", RD16(tfh.Height));                                 // Height of triImage
    triLogPrint("Depth: %i\n", tfh.Depth);                                  // Bits per Pixel of triImage
    triLogPrint("ImageDescrip: %x\n", tfh.ImageDescrip);
    #endif
    
    if (tfh.ImageTypeCode!=1 && tfh.ImageTypeCode!=2 && tfh.ImageTypeCode!=3 &&
		tfh.ImageTypeCode!=9 && tfh.ImageTypeCode!=10 && tfh.ImageTypeCode!=11)
	{
		return("Unknown ImageTypeCode.");
	}
	if (tfh.Depth!=8 && tfh.Depth!=15 && tfh.Depth!=16 && tfh.Depth!=24 && tfh.Depth!=32)
	{
		return("Unknown Bit Depth.");
	}
	
	triS32 y;
	img->palette = 0;
    if ((tfh.ColorMapType==1))
	{
		triS32 maplen = RD16(tfh.ColorMapLength);
		triS32 mapsz = tfh.ColorMapESize;
		
		// triImage has a palette, so read it in and convert it to a useable one
		if (mapsz<=16)
			img->palette = triMalloc(256*2);
		else
			img->palette = triMalloc(256*4);

		if (img->palette==0)
			return("malloc failed on palette.");
		
		if (mapsz<15) mapsz = 24;	// default 24bit
		if (maplen==0) maplen = 256;
		if (maplen>256) maplen = 256;
		stream_read( s, pal, ((mapsz+7)>>3)*maplen );
		fpos += maplen*((mapsz+7)>>3);
		stream_seek(s, fpos, STREAM_SET );
		triU8* ipal = (triU8*)img->palette;
		switch (mapsz)
		{
			case 15:
			case 16:
				img->palformat = IMG_FORMAT_5551;
				for (y=0;y<maplen;y++)
				{
					triU16 col16 = (pal[0 + y*2] | (pal[1 + y*2] << 8));
					col16 = (((col16&0x1F)<<10) | (col16&0x3E0) | ((col16>>10)&0x1F) | (1<<15));
					ipal[0] = (col16 >> 8);
					ipal[1] = (col16 & 0xFF);
					ipal += 2;
				}
				break;
			case 24:
				img->palformat = IMG_FORMAT_8888;
				for (y=0;y<maplen;y++)
				{
					ipal[0] = pal[2 + y*3];
					ipal[1] = pal[1 + y*3];
					ipal[2] = pal[0 + y*3];
					ipal[3] = 0xff;
					ipal += 4;
				}
				break;
			default:
				img->palformat = IMG_FORMAT_8888;
				for (y=0;y<maplen;y++)
				{
					ipal[0] = pal[2 + y*4];
					ipal[1] = pal[1 + y*4];
					ipal[2] = pal[0 + y*4];
					ipal[3] = pal[3 + y*4];
					ipal += 4;
				}
				break;
		}
	}
	else if ( (tfh.ImageTypeCode&3)==3 || (tfh.Depth==8) )
	{
		// Grayscale image or mapped image without palette -> generate default palette
		img->palette = triMalloc(256*4);
		if (img->palette==0)
			return("malloc failed on palette.");

		triU8* ipal = img->palette;
		img->palformat = IMG_FORMAT_8888;
		for (y=0;y<256;y++)
		{
			ipal[0] = y;
			ipal[1] = y;
			ipal[2] = y;
			ipal[3] = 0xff;
			ipal += 4;
		}
	}

	triS32 width = RD16(tfh.Width);// - RD16(tfh.OriginX);
	triS32 height = RD16(tfh.Height);// - RD16(tfh.OriginY);
	triS32 bytesperline = ((width*tfh.Depth)>>3);
	triU8* tdata = (triU8*) triMalloc( height * bytesperline );
    if (tdata==0)
		return("malloc failed on tdata.");


	if ( (tfh.ImageTypeCode&8) == 8 && (tfh.ImageTypeCode&3) > 0 )
	{
		// READ RLE ENCODED triImage
		triS32 rlesize = stream_size( s )-fpos;
		triU8* rle = (triU8*) triMalloc( rlesize );
		if (rle==0)
		{
			triFree( tdata );
			return("malloc failed on rle.");
		}
		stream_read( s, rle, rlesize );

		triChar TRUEVIS[] = "TRUEVISION-XFILE.";
		if (memcmp( &rle[rlesize-26+8], TRUEVIS, 18 )==0)
		{
			// triImage is a TRUEVISION-XFILE and may contain Developer and Extension Areas
			rlesize -= 26;
			long extoffs = rle[rlesize+0] + (rle[rlesize+1]<<8) + (rle[rlesize+2]<<16) + (rle[rlesize+3]<<24);
			long devoffs = rle[rlesize+4] + (rle[rlesize+5]<<8) + (rle[rlesize+6]<<16) + (rle[rlesize+7]<<24);
			if (extoffs!=0 || devoffs!=0)
			{
				// This triImage contains developer and/or extension area :/
				// For now we just assume that the developer or extension area is at the start of the whole extension block
				// Actually, we'd need to go through all following area blocks and find the one that comes first (lowest offset)
				if (devoffs<extoffs)
					rlesize = devoffs-fpos;
				else
					rlesize = extoffs-fpos;
			}
		}
		// DECODE RLE:
		decodeRLE( rle, rlesize, tdata, height * bytesperline, tfh.Depth );
		triFree( rle );
	}
	else if ( ( tfh.ImageTypeCode == 1 ) || ( tfh.ImageTypeCode == 2 ) || ( tfh.ImageTypeCode == 3 ) )
	{
		stream_read( s, tdata, height*bytesperline );
	}
	else
	{
		triFree( tdata );
		return("Unknown ImageTypeCode.");
	}

	switch (tfh.Depth)
	{
		case 8:
			img->bits = 8;
			img->format = IMG_FORMAT_T8;
			break;
		case 15:
		case 16:
			img->bits = 16;
			img->format = IMG_FORMAT_5551;
			break;
		case 24:
		case 32:
			img->bits = 32;
			img->format = IMG_FORMAT_8888;
			break;
		default:
			triFree(tdata);
			return("Unknown bitdepth.");
	}

	img->level = 0;
	img->levels = 0;
	img->width = width;
	img->height = height;
	img->tex_height = next_pow2(height);
	img->stride = next_pow2(width);
	//if (img->stride>512) img->stride = next_mul8(width);
	img->swizzled = 0;
	img->size = img->stride*next_mul8(img->height)*img->bits>>3;
	img->data = triMalloc(img->size);
	if (img->data==0)
	{
		triFree(tdata);
		return("malloc failed on img->data.");
	}
	
	triU8* dst = (triU8*)img->data;
	triU8* src = (triU8*)tdata;
	
	triS32 ddelta = (img->stride-img->width)*img->bits>>3;
	
	triS32 hdelta = bytesperline;
	if ((tfh.ImageDescrip & TGA_DESC_HORIZONTAL) == 0)
	{
		src += (height-1) * bytesperline;
		hdelta = -bytesperline;
	}
	
	triS32 vdelta = (tfh.Depth >> 3);
	if ((tfh.ImageDescrip & TGA_DESC_VERTICAL) != 0)
	{
		src += bytesperline-(tfh.Depth >> 3);
		vdelta = -(tfh.Depth >> 3);
	}
	
	// convert the crappy image and do flipping if neccessary
	triS32 i, j;
	triU16 col16;
	triU32 col32;
	switch (tfh.Depth)
	{
		case 8:
			if (vdelta==1 && hdelta==width)
			{
				src = tdata;
				tdata = img->data;
				img->data = src;
				break;
			}
			for (i = 0; i < height; i++)
			{
				if (vdelta==1)
				{
					memcpy( dst, src, width );
					dst += width;
				}
				else
				{
					triU8* xsrc = src;
					for (j = 0; j < width; j++)
					{
						*dst++ = *xsrc;
						xsrc += vdelta;
					}
				}
				src += hdelta;
				dst += ddelta;
			}
			break;
		case 15:
		case 16:
			for (i = 0; i < height; i++)
			{
				triU8* xsrc = src;
				for (j = 0; j < width; j++)
				{
					col16 = (triU16)(((triU16)xsrc[1] << 8) | (triU16)xsrc[0]);
					col16 = (triU16)(((triU16)(col16&0x1F)<<10) | (triU16)(col16&0x3E0) | ((triU16)(col16>>10)&0x1F) | (triU16)(1<<15));
					*dst++ = (triU8)(col16 & 0xFF);
					*dst++ = (triU8)(col16 >> 8);
					xsrc += vdelta;
				}
				src += hdelta;
				dst += ddelta;
			}
			break;
		case 24:
			for (i = 0; i < height; i++)
			{
				triU8* xsrc = src;
				for (j = 0; j < width; j++)
				{
					*(triU32*)dst = (triU32)(((triU32)xsrc[2]<<16) | ((triU32)xsrc[1]<<8) | (triU32)xsrc[0] | (0xFF<<24));
					dst += 4;
					xsrc += vdelta;
				}
				src += hdelta;
				dst += ddelta;
			}
			break;
		case 32:
			for (i = 0; i < height; i++)
			{
				triU8* xsrc = src;
				for (j = 0; j < width; j++)
				{
					col32 = *(triU32*)(xsrc);
					*(triU32*)dst = ((col32&0xFF00FF00) | ((col32&0xFF)<<16) | ((col32>>16)&0xFF));
					dst += 4;
					xsrc += vdelta;
				}
				src += hdelta;
				dst += ddelta;
			}
			break;
	}
	
	triFree(tdata);
	strncpy( img->filename, stream_name( s ), 64 );
	return(0);
}


triImage* triImageLoadTga( triChar* name )
{
	triVoid* img = triRefcountRetain( name );
	if (img==0)
	{
		stream* s = stream_fopen( name, STREAM_RDONLY ); //stream_bufopen( STREAM_TYPE_FILE, name, STREAM_RDONLY );
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadTgaStream( s, i ))!=0)
		{
			stream_close( s );
			triLogError("triImageLoadTga: %s\n",ret);
			triFree(i);
			return(0);
		}
		stream_close( s );
		triRefcountCreate( name, i );
		return(i);
	}
	else
		return((triImage*)img);
}


triImage* triImageLoadTgaStream( stream* s )
{
	triVoid* img = triRefcountRetain( stream_name( s ) );
	if (img==0)
	{
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadTgaStream( s, i ))!=0)
		{
			triLogError("triImageLoadTgaStream: %s\n",ret);
			triFree(i);
			return(0);
		}
		triRefcountCreate( stream_name( s ), i );
		return(i);
	}
	else
		return((triImage*)img);
}


#ifdef DEBUG
static triChar* _formatString( triS32 format )
{
	switch (format)
	{
		case IMG_FORMAT_T4:
			return "IMG_FORMAT_T4";
		case IMG_FORMAT_T8:
			return "IMG_FORMAT_T8";
		case IMG_FORMAT_T16:
			return "IMG_FORMAT_T16";
		case IMG_FORMAT_T32:
			return "IMG_FORMAT_T32";
		case IMG_FORMAT_4444:
			return "IMG_FORMAT_4444";
		case IMG_FORMAT_5650:
			return "IMG_FORMAT_5650";
		case IMG_FORMAT_5551:
			return "IMG_FORMAT_5551";
		case IMG_FORMAT_8888:
			return "IMG_FORMAT_8888";
		case IMG_FORMAT_DXT1:
			return "IMG_FORMAT_DXT1";
		case IMG_FORMAT_DXT3:
			return "IMG_FORMAT_DXT3";
		case IMG_FORMAT_DXT5:
			return "IMG_FORMAT_DXT5";
		default:
			return "IMG_FORMAT_UNKNOWN";
	}
}


static triChar* _flagsString( triS32 flags )
{
	switch (flags)
	{
		case 0:
			return "NONE";
		case TRI_IMG_FLAGS_SWIZZLE:
			return "SWIZZLE";
		case TRI_IMG_FLAGS_RLE:
			return "RLE";
		case TRI_IMG_FLAGS_GZIP:
			return "GZIP";
		case TRI_IMG_FLAGS_WAVELET:
			return "WAVE";
		case TRI_IMG_FLAGS_SWIZZLE|TRI_IMG_FLAGS_RLE:
			return "SWIZZLE|RLE";
		case TRI_IMG_FLAGS_SWIZZLE|TRI_IMG_FLAGS_GZIP:
			return "SWIZZLE|GZIP";
		case TRI_IMG_FLAGS_SWIZZLE|TRI_IMG_FLAGS_WAVELET:
			return "SWIZZLE|WAVE";
		case TRI_IMG_FLAGS_SWIZZLE|TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_RLE:
			return "SWIZZLE|GZIP|RLE";
		case TRI_IMG_FLAGS_SWIZZLE|TRI_IMG_FLAGS_WAVELET|TRI_IMG_FLAGS_GZIP:
			return "SWIZZLE|WAVE|GZIP";
		case TRI_IMG_FLAGS_SWIZZLE|TRI_IMG_FLAGS_WAVELET|TRI_IMG_FLAGS_RLE:
			return "SWIZZLE|WAVE|RLE";
		case TRI_IMG_FLAGS_SWIZZLE|TRI_IMG_FLAGS_WAVELET|TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_RLE:
			return "SWIZZLE|WAVE|GZIP|RLE";
		default:
			return "UNKNOWN";
	}
}
#endif

static triChar* _triImageLoadTriStream( stream* s, triImage *img, triU32 frame )
{
	if (img == 0) return("triImage is NULL.");
	if (s == 0) return("stream is NULL.");


	triImageFileHeader tfh;
	stream_read( s, &tfh, sizeof(tfh) );
	#ifdef DEBUG
	triLogPrint("triImage (%i)>\n", sizeof(tfh) );
	triLogPrint("tFH.magic: %c%c%c%c%c%c%c%c\n", tfh.magic[0], tfh.magic[1], tfh.magic[2], tfh.magic[3], tfh.magic[4], tfh.magic[5], tfh.magic[6], tfh.magic[7] );
	triLogPrint("tFH.numFrames: %i\n", tfh.numFrames );
	#endif
	
	if (strncmp( tfh.magic, "triImage", 8 )!=0)
		return("Not an triImage!");
    	
	if (frame>=tfh.numFrames)
		return("Invalid frame selected!");


	triImageChunkHeader tch;
	triImageChunk tic;
	triU32 level, i = 0;
	// Skip frames
	while (frame>0 && !stream_eos( s ))
	{
		stream_read( s, &tch, sizeof(tch) );
		
		#ifdef DEBUG
		triLogPrint("Skipping frame %i\n", i );
		triLogPrint("tCH.numLevels: %i\n", tch.numLevels );
		#endif
		
		level = 0;
		while (level<=tch.numLevels && !stream_eos( s ))
		{
			stream_read( s, &tic, sizeof(tic) );
			#ifdef DEBUG
			triLogPrint("Skipping level %i\n", level);
			triLogPrint("tIC.size: %i\n", tic.size );
			#endif
			stream_seek( s, tic.size, SEEK_CUR );
			level++;
		}
		frame--;
		i++;
	}
	

    stream_read( s, &tch, sizeof(tch) );
    
    #ifdef DEBUG
	triLogPrint("tCH.format: %s\n", _formatString(tch.format) );
	triLogPrint("tCH.palformat: %s\n", _formatString(tch.palformat) );
	triLogPrint("tCH.flags: %s\n", _flagsString(tch.flags) );
	triLogPrint("tCH.numLevels: %i\n", tch.numLevels );
	triLogPrint("tCH.delay: %i\n", tch.delay );
	triLogPrint("tCH.xOffs: %i\n", tch.xOffs );
	triLogPrint("tCH.yOffs: %i\n", tch.yOffs );
	#endif
	img->level = 0;
	img->levels = tch.numLevels;
    img->palette = 0;
    triU32 palsize = (tch.palformat==IMG_FORMAT_8888?4:2);
    if (tch.format==IMG_FORMAT_T4)
    {
		img->palette = triMalloc( 16*palsize );
		if (img->palette==0)
			return("Error allocating palette!");
		stream_read( s, img->palette, 16*palsize );
	}
	else
	if (tch.format==IMG_FORMAT_T8 || tch.format==IMG_FORMAT_T16 || tch.format==IMG_FORMAT_T32)
	{
		img->palette = triMalloc( 256*palsize );
		if (img->palette==0)
			return("Error allocating palette!");
		stream_read( s, img->palette, 256*palsize );
	}
    
    img->format = tch.format;
    img->palformat = tch.palformat;
    img->swizzled = (tch.flags&TRI_IMG_FLAGS_SWIZZLE?1:0);
    
	switch (img->format)
	{
		case IMG_FORMAT_T4:
			img->bits = 4;
			break;
		case IMG_FORMAT_T8:
			img->bits = 8;
			break;
		case IMG_FORMAT_T16:
		case IMG_FORMAT_5650:
		case IMG_FORMAT_5551:
		case IMG_FORMAT_4444:
			img->bits = 16;
			break;
		case IMG_FORMAT_T32:
		case IMG_FORMAT_8888:
			img->bits = 32;
			break;
		default:
			img->bits = 16;
			break;
	}


	level = 0;
	triMipLevel* last = 0;
	while (level<=img->levels && !stream_eos( s ))
	{
		stream_read( s, &tic, sizeof(tic) );
		#ifdef DEBUG
		triLogPrint("Read level %i\n", level);
		triLogPrint("tIC.width: %i\n", tic.width );
		triLogPrint("tIC.height: %i\n", tic.height );
		triLogPrint("tIC.stride: %i\n", tic.stride );
		triLogPrint("tIC.size: %i\n", tic.size );
		#endif
		triMipLevel* lev = 0;
		if (level==0)
		{
			img->width = tic.width;
			img->height = tic.height;
			img->stride = tic.stride;
			img->tex_height = next_pow2( tic.height );
			img->size = tic.size;
			img->level = 0;
		}
		else
		{
			lev = triMalloc( sizeof(triMipLevel) );
			if (lev==0) return("Error allocating level structure!");
			lev->next = 0;
			lev->width = tic.width;
			lev->height = tic.height;
			lev->stride = tic.stride;
			lev->tex_height = next_pow2( tic.height );
			lev->size = tic.size;
			
			if (last)
				last->next = lev;
			else
				img->level = lev;
			last = lev;
		}
		
		void* data = triMalloc( tic.size );
		if (data==0)
			return("Error allocating image data!");
	
		stream_read( s, data, tic.size );
		
		if (tch.flags&TRI_IMG_FLAGS_GZIP)
		{
			triU32 dsize = tic.stride*next_mul8(tic.height)*img->bits>>3;
			triU8* ddata = triMalloc( dsize );
			if (ddata==0)
				return("Error allocating GZIP data!");
			int err;
			if ((err=uncompress( ddata, &dsize, data, tic.size ))!=Z_OK)
			{
				printf("ERROR: %i\n", err);
				return("Error uncompressing data!");
			}
			sceKernelDcacheWritebackAll();
			//img->size = sceKernelGzipDecompress( ddata, dsize, img->data, 0 );
			triFree( data );
			data = ddata;
			if (level==0)
			{
				img->data = ddata;
				img->size = dsize;
			}
			else
			{
				lev->data = ddata;
				lev->size = dsize;
			}
		}
		
		if (tch.flags&TRI_IMG_FLAGS_RLE)
		{
			triU32 rsize = tic.stride*next_mul8(tic.height)*img->bits>>3;
			triChar* rleimg = triMalloc( rsize );
			if (rleimg==0)
				return("Error allocating RLE data!");
			decodeRLE( data, tic.size, rleimg, rsize, img->bits );
			triFree( data );
			data = rleimg;
			if (level==0)
			{
				img->data = rleimg;
				img->size = rsize;
			}
			else
			{
				lev->data = rleimg;
				lev->size = rsize;
			}
		}
		
		if (tch.flags&TRI_IMG_FLAGS_WAVELET)
		{
			// TODO: add wavelet decompression into several mipmap-levels (only for non paletted images)
		}
		
		level++;
	}
	strncpy( img->filename, stream_name( s ), 64 );
	return(0);
}


triImage* triImageLoadTri( triChar* name, triU32 frame )
{
	triVoid* img = triRefcountRetain( name );
	if (img==0)
	{
		stream* s = stream_fopen( name, STREAM_RDONLY ); //stream_bufopen( STREAM_TYPE_FILE, name, STREAM_RDONLY );
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadTriStream( s, i, frame ))!=0)
		{
			stream_close( s );
			triLogError("triImageLoadTri: %s\n",ret);
			triFree(i);
			return(0);
		}
		stream_close( s );
		triRefcountCreate( name, i );
		return(i);
	}
	else
		return((triImage*)img);
}


triImage* triImageLoadTriStream( stream* s, triU32 frame )
{
	triVoid* img = triRefcountRetain( stream_name( s ) );
	if (img==0)
	{
		triImage* i = triMalloc(sizeof(triImage));
		triChar* ret;
		if ((ret=_triImageLoadTriStream( s, i, frame ))!=0)
		{
			triLogError("triImageLoadTriStream: %s\n",ret);
			triFree(i);
			return(0);
		}
		triRefcountCreate( stream_name( s ), i );
		return(i);
	}
	else
		return((triImage*)img);
}


triImage* triImageLoad( triChar* name, triU32 flags )
{
	stream* s = stream_fopen( name, STREAM_RDONLY ); //stream_bufopen( STREAM_TYPE_FILE, name, STREAM_RDONLY );
	if (s==0)
	{
		triLogError("ERROR: Could not open stream for file '%s'.\n", name);
		return(0);
	}
	
	triU8 magic[8];
	stream_read( s, magic, 8 );
	stream_seek( s, 0, STREAM_SET );
	
	triImage* img = 0;
	#ifdef TRI_SUPPORT_PNG
	//triLogPrint("magic: %c%c%c%c%c%c%c%c\n", magic[0],magic[1],magic[2],magic[3],magic[4],magic[5],magic[6],magic[7]);
	if (memcmp(magic+1, "PNG", 3)==0) // (png_sig_cmp(magic,0,8)) // [fails??]
		img = triImageLoadPngStream( s );
	else
	#endif

	if (memcmp(magic,"triImage",8)==0)
		img = triImageLoadTriStream( s, (flags >> 11) );
	else
		img = triImageLoadTgaStream( s );
	
	stream_close( s );
	
	if ((flags & (TRI_SWIZZLE|TRI_VRAM))==(TRI_SWIZZLE|TRI_VRAM))
		triImageSwizzleToVRAM( img );
	else
	if (flags & TRI_SWIZZLE)
		triImageSwizzle( img );
	else
	if (flags & TRI_VRAM)
		triImageToVRAM( img );
		
	sceKernelDcacheWritebackAll();
	return img;
}


triImage* triImageLoadStream( stream* s, triU32 flags )
{
	triU32 pos = stream_tell( s );
	triChar magic[8];
	stream_read( s, magic, 8 );
	stream_seek( s, pos, STREAM_SET );
	
	triImage* img = 0;
	#ifdef TRI_SUPPORT_PNG
	if (png_sig_cmp(magic,0,8))
		img = triImageLoadPngStream( s );
	else
	#endif

	if (strncmp(magic,"triImage",8)==0)
		img = triImageLoadTriStream( s, (flags >> 11) );
	else
		img = triImageLoadTgaStream( s );
	
	sceKernelDcacheWritebackAll();
	if ((flags & (TRI_SWIZZLE|TRI_VRAM))==(TRI_SWIZZLE|TRI_VRAM))
		triImageSwizzleToVRAM( img );
	else
	if (flags & TRI_SWIZZLE)
		triImageSwizzle( img );
	else
	if (flags & TRI_VRAM)
		triImageToVRAM( img );
	
	return img;
}



triImageAnimation* triImageAnimationCreate()
{
	triImageAnimation* ani = triMalloc( sizeof(triImageAnimation) );
	if (ani==0) return(0);
	
	ani->image = 0;
	ani->frames = 0;
	ani->curFrame = 0;
	ani->numFrames = 0;
	ani->width = 0;
	ani->height = 0;
	ani->playing = 0;
	
	ani->palette = 0;
	ani->loops = 0;
	ani->loopsDone = 0;
	ani->timeBase = 1000;
	
	return(ani);
}


// delay time in ms
triVoid triImageAnimationAppend( triImageAnimation* ani, triImage* img, triS32 sx, triS32 sy, triS32 sw, triS32 sh, triS32 xOffs, triS32 yOffs, triU32 delay )
{
	if (ani==0 || img==0) return;

	triImageList* new_frame = triMalloc( sizeof(triImageList) );
	if (new_frame==0) return;
	new_frame->next = 0;
	new_frame->image = img;
	new_frame->delay = delay;
	new_frame->xOffs = xOffs;
	new_frame->yOffs = yOffs;
	new_frame->sx = sx;
	new_frame->sy = sy;
	new_frame->sw = sw;
	new_frame->sh = sh;
	if (ani->frames == 0)
	{
		ani->frames = new_frame;
		ani->curFrame = new_frame;
	}
	else
	{
		triImageList* list = ani->frames;
		while (list->next!=0)
			list = list->next;
		list->next = new_frame;
	}
	triRefcountRetainPtr( img );
	ani->numFrames++;
	if (ani->width<img->width) ani->width = img->width;
	if (ani->height<img->height) ani->height = img->height;
}


triVoid triImageAnimationAppend2( triImageAnimation* ani, triImage* img, triU32 delay )
{
	if (ani==0 || img==0) return;
	triImageAnimationAppend( ani, img, 0, 0, img->width, img->height, 0, 0, delay );
}


triVoid triImageAnimationAppend3( triImageAnimation* ani, triImage* img, triS32 xOffs, triS32 yOffs, triU32 delay )
{
	if (ani==0 || img==0) return;
	triImageAnimationAppend( ani, img, 0, 0, img->width, img->height, xOffs, yOffs, delay );
}


// Create an animation from an triImage sheet in order left-right, top-down
triImageAnimation* triImageAnimationFromSheet( triImage* img, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay )
{
	triImageAnimation* ani = triImageAnimationCreate();
	if (ani==0) return(0);
	triS32 i, j;
	triS32 sy = 0;
	for (i=0;i<vframes;i++)
	{
		triS32 sx = 0;
		for (j=0;j<hframes;j++)
		{
			triImageAnimationAppend( ani, img, sx, sy, fwidth, fheight, 0, 0, delay );
			sx += fwidth;
		}
		sy += fheight;
	}
	return(ani);
}


// Create an animation from an triImage sheet in order left-right, top-down
triImageAnimation* triImageAnimationFromSheet2( triImage* img, triS32 xoffs, triS32 yoffs, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay )
{
	triImageAnimation* ani = triImageAnimationCreate();
	if (ani==0) return(0);
	triS32 i, j;
	triS32 sy = yoffs;
	for (i=0;i<vframes;i++)
	{
		triS32 sx = xoffs;
		for (j=0;j<hframes;j++)
		{
			triImageAnimationAppend( ani, img, sx, sy, fwidth, fheight, 0, 0, delay );
			sx += fwidth;
		}
		sy += fheight;
	}
	return(ani);
}


triImageAnimation* triImageAnimationFromSheetTga( triChar* name, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay )
{
	triImage* img = triImageLoadTga( name );
	if (img==0) return(0);
	triImageAnimation* ani = triImageAnimationFromSheet( img, fwidth, fheight, hframes, vframes, delay );
	triRefcountRelease( img );
	return(ani);
}


#ifdef TRI_SUPPORT_PNG
triImageAnimation* triImageAnimationFromSheetPng( triChar* name, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay )
{
	triImage* img = triImageLoadPng( name );
	if (img==0) return(0);
	triImageAnimation* ani = triImageAnimationFromSheet( img, fwidth, fheight, hframes, vframes, delay );
	triRefcountRelease( img );
	return(ani);
}
#endif // TRI_SUPPORT_PNG


triImageAnimation* triImageAnimationFromSheetFile( triChar* name, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay )
{
	triImage* img = triImageLoad( name, TRI_SWIZZLE|TRI_VRAM );
	if (img==0) return(0);
	triImageAnimation* ani = triImageAnimationFromSheet( img, fwidth, fheight, hframes, vframes, delay );
	triRefcountRelease( img );
	return(ani);
}

triImageAnimation* triImageAnimationFromSheetFile2( triChar* name, triS32 xoffs, triS32 yoffs, triS32 fwidth, triS32 fheight, triS32 hframes, triS32 vframes, triU32 delay )
{
	triImage* img = triImageLoad( name, TRI_SWIZZLE|TRI_VRAM );
	if (img==0) return(0);
	triImageAnimation* ani = triImageAnimationFromSheet2( img, xoffs, yoffs, fwidth, fheight, hframes, vframes, delay );
	triRefcountRelease( img );
	return(ani);
}

triVoid triImageAnimationStart( triImageAnimation* ani )
{
	if (ani==0) return;
	ani->playing = 1;
	sceRtcGetCurrentTick(&ani->lastUpdate);
}

triVoid triImageAnimationPause( triImageAnimation* ani )
{
	if (ani==0) return;
	ani->playing = 0;
}

triVoid triImageAnimationReset( triImageAnimation* ani )
{
	if (ani==0) return;
	ani->playing = 0;
	ani->curFrame = ani->frames;
	ani->image = 0;
	ani->loopsDone = 0;
	if (ani->curFrame!=0)
		ani->image = ani->curFrame->image;
	sceRtcGetCurrentTick(&ani->lastUpdate);
}

triS32 triImageAnimationUpdate( triImageAnimation* ani )
{
	if (ani==0 || ani->frames==0 || ani->playing==0) return(0);
	triU64 cur_tick;
	sceRtcGetCurrentTick(&cur_tick);
	
	if (ani->curFrame == 0) ani->curFrame = ani->frames;
	if ((triU32)(cur_tick-ani->lastUpdate)/ani->timeBase >= ani->curFrame->delay)
	{
		if (ani->curFrame->next == 0)	// we completed one loop?
		{
			ani->loopsDone++;
			if (ani->loops>0)	// we have a finite number of loops
			{
				if (ani->loopsDone>=ani->loops)
					triImageAnimationPause( ani );
				else
					ani->curFrame = ani->frames;
			}
			else
				ani->curFrame = ani->frames;
		}
		else
			ani->curFrame = ani->curFrame->next;
		ani->image = ani->curFrame->image;
		ani->lastUpdate = cur_tick;
		return(1);
	}
	return(0);
}

triVoid triImageAnimationSetSpeed( triImageAnimation* ani, triFloat factor )
{
	if (ani==0) return;
	
	ani->timeBase = (int)(factor*1000);
}

triBool triImageAnimationIsDone( triImageAnimation* ani )
{
	if (ani==0) return(1);
	
	return (ani->loops && ani->loopsDone >= ani->loops);
}

triImage* triImageAnimationGetFrame( triImageAnimation* ani, triS32 nFrame )
{
	if (ani==0) return(0);
	
	triImageList* list = ani->frames;
	
	while (nFrame && list)
	{
		list = list->next;
		nFrame--;
	}
	
	if (list==0) return(0);
	return list->image;
}


// ****************************************************************************
// * LZWDecoder (C/C++)                                                       *
// * Codec to perform LZW (GIF Variant) decompression.                        *
// *                         (c) Nov2000, Juan Soulie <jsoulie@cplusplus.com> *
// ****************************************************************************
//
// Parameter description:
//  - bufIn: Input buffer containing a "de-blocked" GIF/LZW compressed triImage.
//  - bufOut: Output buffer where result will be stored.
//  - InitCodeSize: Initial CodeSize to be Used
//    (GIF files include this as the first byte in a picture block)
//  - AlignedWidth : Width of a row in memory (including alignment if needed)
//  - Width, Height: Physical dimensions of triImage.
//  - interlace: 1 for interlaced GIFs.
//
/*
triS32 LZWDecoder (triChar * bufIn, triChar * bufOut,
								triS16 InitCodeSize, triS32 AlignedWidth,
								triS32		Width, triS32 Height, const triS32 triS32erlace)
{
	triS32						n;
	triS32						row=0,col=0;				// used to point output if interlaced
	triS32						nPixels, maxPixels;	// Output pixel counter
								
	triS16					CodeSize;						// Current CodeSize (size in bits of codes)
	triS16					ClearCode;					// Clear code : resets decompressor
	triS16					EndCode;						// End code : marks end of information
								
	triS32						whichBit;						// Index of next bit in bufIn
	triS32						LongCode;						// Temp. var. from which Code is retrieved
	triS16					Code;								// Code extracted
	triS16					PrevCode;						// Previous Code
	triS16					OutCode;						// Code to output

	// Translation Table:
	triS16					Prefix[4096];				// Prefix: index of another Code
	triU8			Suffix[4096];				// Suffix: terminating triCharacter
	triS16					FirstEntry;					// Index of first free entry in table
	triS16					NextEntry;					// Index of next free entry in table

	triU8			OutStack[4097];			// Output buffer
	triS32						OutIndex;						// characters in OutStack

	triS32						RowOffset;					// Offset in output buffer for current row

	// Set up values that depend on InitCodeSize Parameter.
	CodeSize	= InitCodeSize+1;
	ClearCode = (1 << InitCodeSize);
	EndCode		= ClearCode + 1;
	NextEntry = FirstEntry = ClearCode + 2;

	whichBit	= 0;
	nPixels		= 0;
	maxPixels = Width*Height;
	RowOffset = 0;

	while (nPixels<maxPixels) {
		OutIndex = 0;							// Reset Output Stack

		// GET NEXT CODE FROM bufIn:
		// LZW compression uses code items longer than a single byte.
		// For GIF Files, code sizes are variable between 9 and 12 bits 
		// That's why we must read data (Code) this way:
		triLogPrint("0x%x %i (%i %i %i)\n", bufIn, CodeSize, whichBit, whichBit/8, whichBit&7);
		LongCode=*((triS32*)(bufIn+whichBit/8));					// Get some bytes from bufIn
		LongCode>>=(whichBit&7);												// Discard too low bits
		Code =(triS16)((LongCode & ((1<<CodeSize)-1) ));	// Discard too high bits
		triLogPrint("Code: 0x%x\n", Code);
		whichBit += CodeSize;														// Increase Bit Offset

		// SWITCH, DIFFERENT POSIBILITIES FOR CODE:
		if (Code == EndCode)					// END CODE
			break;											// Exit LZW Decompression loop

		if (Code == ClearCode) 
		{			
			// CLEAR CODE:
			CodeSize  = InitCodeSize+1;	// Reset CodeSize
			NextEntry = FirstEntry;			// Reset Translation Table
			PrevCode=Code;							// Prevent next to be added to table.
			continue;										// restart, to get another code
		}
		if (Code < NextEntry)					// CODE IS IN TABLE
			OutCode = Code;							// Set code to output.

		else 
		{															// CODE IS NOT IN TABLE:
			OutIndex++;									// Keep "first" triCharacter of previous output.
			OutCode = PrevCode;					// Set PrevCode to be output
		}

		// EXPAND OutCode IN OutStack
		// - Elements up to FirstEntry are Raw-Codes and are not expanded
		// - Table Prefices contain indexes to other codes
		// - Table Suffices contain the raw codes to be output
		while (OutCode >= FirstEntry) 
		{
			if (OutIndex > 4096) 
				return 0;
			OutStack[OutIndex++] = Suffix[OutCode];	// Add suffix to Output Stack
			OutCode = Prefix[OutCode];							// Loop with preffix
		}

		// NOW OutCode IS A RAW CODE, ADD IT TO OUTPUT STACK.
		if (OutIndex > 4096) 
			return 0;
		OutStack[OutIndex++] = (triU8) OutCode;

		// ADD NEW ENTRY TO TABLE (PrevCode + OutCode)
		// (EXCEPT IF PREVIOUS CODE WAS A CLEARCODE)
		if (PrevCode!=ClearCode) 
		{
			Prefix[NextEntry] = PrevCode;
			Suffix[NextEntry] = (triU8) OutCode;
			NextEntry++;

			// Prevent Translation table overflow:
			if (NextEntry>=4096) 
				return 0;
      
			// INCREASE CodeSize IF NextEntry IS INVALID WITH CURRENT CodeSize
			if (NextEntry >= (1<<CodeSize)) 
			{
				if (CodeSize < 12) CodeSize++;
				else 
				{
					;
				}				// Do nothing. Maybe next is Clear Code.
			}
		}

		PrevCode = Code;

		// AtriVoid the possibility of overflow on 'bufOut'.
		if (nPixels + OutIndex > maxPixels) OutIndex = maxPixels-nPixels;

		// OUTPUT OutStack (LAST-IN FIRST-OUT ORDER)
		for (n=OutIndex-1; n>=0; n--) 
		{
			if (col==Width)						// Check if new row.
			{
				if (interlace) 
				{				
					// If interlaced::
					     if ((row&7)==0) {row+=8; if (row>=Height) row=4;}
					else if ((row&3)==0) {row+=8; if (row>=Height) row=2;}
					else if ((row&1)==0) {row+=4; if (row>=Height) row=1;}
					else row+=2;
				}
				else							// If not interlaced:
					row++;

				RowOffset=row*AlignedWidth;		// Set new row offset
				col=0;
			}
			bufOut[RowOffset+col]=OutStack[n];	// Write output
			col++;	nPixels++;					// Increase counters.
		}

	}	// while (main decompressor loop)

	return whichBit;
}
*/

typedef struct
{
	union
	{
		struct { triU8 a, b, g, r; };
		triU32 color;
	};
} COLOR;

triImageAnimation* triImageAnimationLoadGif( triChar* name )
{
	/*
	if (img == 0) return;
	triS32 n;

	img->image = 0;
	img->frames = 0;
	img->curFrame = 0;
	img->numFrames = 0;
	img->palette = 0;
	img->globalPalette = 0;
	
	// Global GIF variables:
	triS32			GlobalBPP;							// Bits per Pixel.
	COLOR * GlobalColorMap;					// Global colormap (allocate)

	struct GIFGCEtag 
	{																// GRAPHIC CONTROL EXTENSION
		triU8 BlockSize;			// Block Size: 4 bytes
		triU8 PackedFields;		// 3.. Packed Fields. Bits detail:
																	//    0: Transparent Color Flag
																	//    1: User Input Flag
																	//  2-4: Disposal Method
		triU16 Delay;					// 4..5 Delay Time (1/100 seconds)
		triU8 Transparent;		// 6.. Transparent Color Index
	} gifgce;

	struct GIFNetscapeTag 
	{
		triU8  comment[11];		//4...14  NETSCAPE2.0
		triU8  SubBlockLength; //15      0x3															
		triU8  reserved;       //16      0x1
		triU16 iIterations ;    //17..18  number of iterations (lo-hi)															
	} gifnetscape;

	triS32 GraphicExtensionFound = 0;

	// OPEN FILE
	FILE *fd=fopen(name,"rb");
	if (!fd) 
		return;

	// *1* READ HEADERBLOCK (6bytes) (SIGNATURE + VERSION)
	triChar szSignature[6];				// First 6 bytes (GIF87a or GIF89a)
	triS32 iRead=fread(szSignature,1,6,fd);
	if (iRead !=6)
	{
		fclose(fd);
		return;
	}
	if ( memcmp(szSignature,"GIF",2) != 0)
	{
		fclose(fd);
		return;
	}

	// *2* READ LOGICAL SCREEN DESCRIPTOR
	struct GIFLSDtag 
	{
		triU16 ScreenWidth;		// Logical Screen Width
		triU16 ScreenHeight;	// Logical Screen Height
		triU8 PackedFields;		// Packed Fields. Bits detail:
										//  0-2: Size of Global Color Table
										//    3: Sort Flag
										//  4-6: Color Resolution
										//    7: Global Color Table Flag
		triU8 Background;		// Background Color Index
		triU8 PixelAspectRatio;	// Pixel Aspect Ratio
	} giflsd;

	iRead=fread(&giflsd,1,sizeof(giflsd),fd);
	if (iRead !=sizeof(giflsd))
	{
		fclose(fd);
		return;
	}

	triLogPrint("Logical Screen Descriptor:\n");
	triLogPrint("ScreenWidth: %i\nScreenHeight: %i\n", giflsd.ScreenWidth, giflsd.ScreenHeight);
	
	GlobalBPP = (giflsd.PackedFields & 0x07) + 1;

	// fill some animation data:
	img->width  = giflsd.ScreenWidth;
	img->height = giflsd.ScreenHeight;
	img->loops = 1;

	// *3* READ/GENERATE GLOBAL COLOR MAP
	GlobalColorMap = triMalloc(sizeof(COLOR)*(1<<GlobalBPP));
	if (giflsd.PackedFields & 0x80)	// File has global color map?
		for (n=0;n< 1<<GlobalBPP;n++)
		{
			GlobalColorMap[n].r = fgetc(fd);
			GlobalColorMap[n].g = fgetc(fd);
			GlobalColorMap[n].b = fgetc(fd);
			GlobalColorMap[n].a = 255;
		}
	else	// GIF standard says to provide an triS32ernal default Palette:
		for (n=0;n<256;n++)
		{
			GlobalColorMap[n].r=GlobalColorMap[n].g=GlobalColorMap[n].b=n;
			GlobalColorMap[n].a=255;
		}

	// *4* NOW WE HAVE 3 POSSIBILITIES:
	//  4a) Get and Extension Block (Blocks with additional information)
	//  4b) Get an mage Separator (Introductor to an image)
	//  4c) Get the trailer char (End of GIF File)
	do
	{
		triS32 triCharGot = fgetc(fd);
		triLogPrint("triCharGot: %x\n", triCharGot);

		if (triCharGot == 0x21)		// *A* EXTENSION BLOCK 
		{
			triU8 extensionType=fgetc(fd);
			triS32 nBlockLength;
			switch (extensionType)
			{
				case 0xF9:			// Graphic Control Extension
				{
					triLogPrint("Graphics Control Extension found.\n");
					fread((triChar*)&gifgce,1,sizeof(gifgce),fd);
					GraphicExtensionFound++;
					triS32 term = fgetc(fd); // Block Terminator (always 0)
					triLogPrint("Terminator: %x\n", term);
				}
				break;

				case 0xFE:			// Comment Extension: Ignored
				{
					while ((nBlockLength = fgetc(fd))!=0)
						for (n=0;n<nBlockLength;n++) fgetc(fd);
				}
				break;

				case 0x01:			// PlatriS32ext Extension: Ignored
				{
					while ((nBlockLength = fgetc(fd))!=0)
						for (n=0;n<nBlockLength;n++) fgetc(fd);
				}
				break;

				case 0xFF:			// Application Extension: Ignored
				{
					nBlockLength = fgetc(fd);
					if (nBlockLength==0x0b)
					{
						struct GIFNetscapeTag  tag;
						fread((triChar*)&tag,1,sizeof(gifnetscape),fd);
						img->loops=tag.iIterations;  
						if (img->loops) img->loops++;
						triS32 iterm=fgetc(fd); // terminator
					}
					else
					{
						do
						{
							for (n=0;n<nBlockLength;n++) fgetc(fd);
						} while (nBlockLength = fgetc(fd));
					}
				}
				break;

				default:			// Unknown Extension: Ignored
				{
					// read (and ignore) data sub-blocks
					while ((nBlockLength = fgetc(fd)))
						for (n=0;n<nBlockLength;n++) fgetc(fd);
				}
				break;
			}
		}
		else if (triCharGot == 0x2c)
		{	// *B* triImage (0x2c triImage Separator)

			// Create a new triImage Object:
			triImage* next = triMalloc(sizeof(triImage));
			triImageList* nextl = triMalloc(sizeof(triImageList));
			nextl->image = next;

			// Read triImage Descriptor
			struct GIFIDtag 
			{	
				triU16 xPos;					// triImage Left Position
				triU16 yPos;					// triImage Top Position
				triU16 Width;					// triImage Width
				triU16 Height;				// triImage Height
				triU8 PackedFields;		// Packed Fields. Bits detail:
																			//  0-2: Size of Local Color Table
																			//  3-4: (Reserved)
																			//    5: Sort Flag
																			//    6: triS32erlace Flag
																			//    7: Local Color Table Flag
			} gifid;

			fread((triChar*)&gifid, 1,sizeof(gifid),fd);
			
			triLogPrint("Got triImage descriptor:\n");
			triLogPrint("xPos: %i\nyPos: %i\nWidth: %i\nHeight: %i\n", gifid.xPos, gifid.yPos, gifid.Width, gifid.Height);
			triLogPrint("PackedFields: %x\n", gifid.PackedFields);

			triS32 LocalColorMap = (gifid.PackedFields & 0x08)? 1 : 0;

			next->format = IMG_FORMAT_T8;
			next->width = gifid.Width;
			next->height = gifid.Height;
			next->stride = next_pow2(next->width);
			next->tex_height = next_pow2(next->height);
			next->bits = 8; // LocalColorMap ? (gifid.PackedFields&7)+1 : GlobalBPP;
			next->size = next->stride*next->height;
			next->data = memalign(16,next->size);
			if (next->data==0)
			{
				triImageFree(next);
				triFree(nextl);
				GraphicExtensionFound=0;
				continue;
			}
			next->palformat = IMG_FORMAT_8888;
			next->palette = memalign(16,256*4);
			if (next->palette==0)
			{
				triImageFree(next);
				triFree(nextl);
				GraphicExtensionFound=0;
				continue;
			}
			
			nextl->xOffs = gifid.xPos;
			nextl->yOffs = gifid.yPos;

			// Fill NexttriImage Data
			triS32 transparent = -1;
			if (GraphicExtensionFound)
			{
				transparent = (gifgce.PackedFields&0x01) ? gifgce.Transparent : -1;
				//NexttriImage->Transparency= (gifgce.PackedFields&0x1c)>1 ? 1 : 0;
				nextl->delay       = gifgce.Delay*10;
			}

			if (transparent != -1)
				memset(next->data, transparent, next->size);
			else
				memset(next->data, giflsd.Background, next->size);

			if (LocalColorMap)		// Read Color Map (if descriptor says so)
				fread((triChar*)next->palette,1,sizeof(COLOR)*(1<<next->bits),fd);
			else					// Otherwise copy Global
				memcpy(next->palette, GlobalColorMap,sizeof(COLOR)*(1<<next->bits));
			
			if (transparent != -1)
				((triChar*)next->palette)[transparent*4+3]=0;	// set transparent color


			triS16 firstbyte=fgetc(fd);	// 1st byte of img block (CodeSize)
			triLogPrint("firstbyte: %x\n", firstbyte);

			// Calculate compressed triImage block size
				// to fix: this allocates an extra byte per block
			long ImgStart,ImgEnd;
			ImgEnd = ImgStart = ftell(fd);
			while ((n=fgetc(fd))!=0)
			{
				ImgEnd+=n+1;
				fseek (fd,ImgEnd,SEEK_SET );
			}
			if (ImgEnd==ImgStart)
			{
				triLogPrint("skipping zero-size block.\n");
				triImageFree(next);
				triFree(nextl);
				GraphicExtensionFound=0;
				continue;
			}
			
			fseek (fd,ImgStart,SEEK_SET);
			// Allocate Space for Compressed triImage
			triChar* pCompressedtriImage = triMalloc(ImgEnd-ImgStart+4);
			if (pCompressedtriImage==0)
			{
				triImageFree(next);
				triFree(nextl);
				break;
			}
			triLogPrint("allocated %i bytes for compressed triImage.\n", ImgEnd-ImgStart+4);
  
			// Read and store Compressed triImage
			triChar* pTemp = pCompressedtriImage;
			triS32 nBlockLength;
			while ((nBlockLength = fgetc(fd)))
			{
				fread(pTemp,1,nBlockLength,fd);
				pTemp+=nBlockLength;
			}

			// Call LZW/GIF decompressor
			n=LZWDecoder(
									(triChar*) pCompressedtriImage,
									(triChar*) next->data,
									firstbyte, next->stride,
									gifid.Width, gifid.Height,
									((gifid.PackedFields & 0x40)?1:0)	//triS32erlaced?
									);

			if (n)
			{
				if (img->frames==0)
				{
					img->frames = img->curFrame = nextl;
				}
				else
				{
					nextl->next = img->curFrame;
					img->curFrame = nextl;
				}
				img->numFrames++;
			}
			else
			{
				triImageFree(next);
				triFree(nextl);
			}

			// Some cleanup
			triFree(pCompressedtriImage);
			GraphicExtensionFound=0;
			break;
		}
		else if (triCharGot == 0x3b) 
		{	
			break; // Ok. Standard End.
		}

	} while ( !feof(fd) );

	fclose(fd);
	img->curFrame = img->frames;
	img->image = img->curFrame->image;*/
	return(0);
}

triImageAnimation* triImageAnimationLoadGifStream( stream* s )
{
	return(0);
}



triChar* _triImageAnimationLoadTriStream( stream* s, triImageAnimation* ani )
{
	if (ani==0) return("ImageAnimation is NULL.");
	if (s == 0) return("stream is NULL.");


	triImageFileHeader tfh;
	stream_read( s, &tfh, sizeof(tfh) );
	#ifdef DEBUG
	triLogPrint("triImage (%i)>\n", sizeof(tfh) );
	triLogPrint("tFH.magic: %c%c%c%c%c%c%c%c\n", tfh.magic[0], tfh.magic[1], tfh.magic[2], tfh.magic[3], tfh.magic[4], tfh.magic[5], tfh.magic[6], tfh.magic[7] );
	triLogPrint("tFH.numFrames: %i\n", tfh.numFrames );
	#endif
	
	if (strncmp( tfh.magic, "triImage", 8 )!=0)
		return("Not an triImage!");
    
    ani->numFrames = tfh.numFrames;
    
    triImageList* last = 0;
    triSInt i = 0;
    for (;i<tfh.numFrames;i++)
    {
		triImageList* new_list = triMalloc( sizeof(triImageList) );
		if (new_list==0) break;
		new_list->image = triMalloc( sizeof(triImage) );
		if (new_list->image==0) break;
		
		triImage* img = new_list->image;
		new_list->next = 0;
		
		new_list->sx = new_list->sy = new_list->sw = new_list->sh = 0;
		if (last!=0)
			last->next = new_list;
		else
			ani->frames = new_list;
		last = new_list;
		
		
		triImageChunkHeader tch;
		stream_read( s, &tch, sizeof(tch) );
		
		#ifdef DEBUG
		triLogPrint("tCH.format: %s\n", _formatString(tch.format) );
		triLogPrint("tCH.palformat: %s\n", _formatString(tch.palformat) );
		triLogPrint("tCH.flags: %s\n", _flagsString(tch.flags) );
		triLogPrint("tCH.numLevels: %i\n", tch.numLevels );
		triLogPrint("tCH.delay: %i\n", tch.delay );
		triLogPrint("tCH.xOffs: %i\n", tch.xOffs );
		triLogPrint("tCH.yOffs: %i\n", tch.yOffs );
		#endif
		
		new_list->delay = tch.delay;
		new_list->xOffs = tch.xOffs;
		new_list->yOffs = tch.yOffs;
		
		img->palette = 0;
		triU32 palsize = (tch.palformat==IMG_FORMAT_8888?4:2);
		if (tch.format==IMG_FORMAT_T4)
		{
			img->palette = triMalloc( 16*palsize );
			if (img->palette==0)
				return("Error allocating palette!");
			stream_read( s, img->palette, 16*palsize );
		}
		else
		if (tch.format==IMG_FORMAT_T8 || tch.format==IMG_FORMAT_T16 || tch.format==IMG_FORMAT_T32)
		{
			img->palette = triMalloc( 256*palsize );
			if (img->palette==0)
				return("Error allocating palette!");
			stream_read( s, img->palette, 256*palsize );
		}
		
		img->levels = 0;
		img->level = 0;
		img->format = tch.format;
		img->palformat = tch.palformat;
		img->swizzled = (tch.flags&TRI_IMG_FLAGS_SWIZZLE?1:0);
		
		triImageChunk tic;
		stream_read( s, &tic, sizeof(tic) );
		#ifdef DEBUG
		triLogPrint("tIC.width: %i\n", tic.width );
		triLogPrint("tIC.height: %i\n", tic.height );
		triLogPrint("tIC.stride: %i\n", tic.stride );
		triLogPrint("tIC.size: %i\n", tic.size );
		#endif
		img->width = tic.width;
		img->height = tic.height;
		img->stride = tic.stride;
		img->tex_height = next_pow2( tic.height );
		img->size = tic.size;
		new_list->sw = tic.width;
		new_list->sh = tic.height;
		
		switch (img->format)
		{
			case IMG_FORMAT_T4:
				img->bits = 4;
				break;
			case IMG_FORMAT_T8:
				img->bits = 8;
				break;
			case IMG_FORMAT_T16:
			case IMG_FORMAT_5650:
			case IMG_FORMAT_5551:
			case IMG_FORMAT_4444:
				img->bits = 16;
				break;
			case IMG_FORMAT_T32:
			case IMG_FORMAT_8888:
				img->bits = 32;
				break;
			default:
				img->bits = 16;
				break;
		}
		
		img->data = triMalloc( img->size );
		if (img->data==0)
			return("Error allocating image data!");
	
		stream_read( s, img->data, img->size );
		
		if (tch.flags&TRI_IMG_FLAGS_GZIP)
		{
			triU32 dsize = img->stride*img->height*img->bits>>3;
			triU8* ddata = triMalloc( dsize );
			if (ddata==0)
				return("Error allocating GZIP data!");
			if (uncompress( ddata, &dsize, img->data, img->size )!=Z_OK)
				return("Error uncompressing data!");
			sceKernelDcacheWritebackAll();
			//img->size = sceKernelGzipDecompress( ddata, dsize, img->data, 0 );
			triFree( img->data );
			img->data = ddata;
			img->size = dsize;
		}
		
		if (tch.flags&TRI_IMG_FLAGS_RLE)
		{
			triU32 rsize = img->stride*img->height*img->bits>>3;
			triChar* rleimg = triMalloc( rsize );
			if (rleimg==0)
				return("Error allocating RLE data!");
			decodeRLE( img->data, img->size, rleimg, rsize, img->bits );
			triFree( img->data );
			img->data = rleimg;
			img->size = rsize;
		}
		
		if (tch.flags&TRI_IMG_FLAGS_WAVELET)
		{
		}
		
		// Skip Mipmap Levels:
		triSInt i = 0;
		for (;i<tch.numLevels;i++)
		{
			stream_read( s, &tic, sizeof(tic) );
			#ifdef DEBUG
			triLogPrint("tIC.width: %i\n", tic.width );
			triLogPrint("tIC.height: %i\n", tic.height );
			triLogPrint("tIC.stride: %i\n", tic.stride );
			triLogPrint("tIC.size: %i\n", tic.size );
			#endif
			stream_seek( s, tic.size, STREAM_CUR );
		}
	}
	
	ani->width = 0;
	ani->height = 0;
	ani->palette = 0;
	ani->timeBase = 1000;
	ani->loops = 0;
	ani->playing = 0;
	ani->curFrame = ani->frames;
	ani->image = 0;
	ani->loopsDone = 0;
	if (ani->curFrame!=0)
		ani->image = ani->curFrame->image;
	
	return(0);
}


triImageAnimation* triImageAnimationLoadTri( triChar* name )
{
	triVoid* img = triRefcountRetain( name );
	if (img==0)
	{
		stream* s = stream_bufopen( STREAM_TYPE_FILE, name, STREAM_RDONLY );
		triImageAnimation* i = triMalloc(sizeof(triImageAnimation));
		triChar* ret;
		if ((ret=_triImageAnimationLoadTriStream( s, i ))!=0)
		{
			stream_close( s );
			triLogError("triImageAnimationLoadTri: %s\n",ret);
			triFree(i);
			return(0);
		}
		stream_close( s );
		triRefcountCreate( name, i );
		return(i);
	}
	else
		return((triImageAnimation*)img);
}

triImageAnimation* triImageAnimationLoadTriStream( stream* s )
{
	triVoid* img = triRefcountRetain( stream_name( s ) );
	if (img==0)
	{
		triImageAnimation* i = triMalloc(sizeof(triImageAnimation));
		triChar* ret;
		if ((ret=_triImageAnimationLoadTriStream( s, i ))!=0)
		{
			triLogError("triImageAnimationLoadTriStream: %s\n",ret);
			triFree(i);
			return(0);
		}
		triRefcountCreate( stream_name( s ), i );
		return(i);
	}
	else
		return((triImageAnimation*)img);
}


void triImageAnimationSaveTri( triChar* name, triImageAnimation* ani, triS32 flags )
{
	if (ani==0) return;
	if (ani->frames==0 || ani->width==0 || ani->height==0) return;
	
	#ifdef DEBUG
	triLogPrint("Saving animation as .tri file.\n");
	#endif
	FILE* fp;

	fp = fopen(name, "wb");
	if (!fp) return;

	triImageFileHeader tfh;
	memcpy( tfh.magic, "triImage", 8 );
	tfh.numFrames = ani->numFrames;
	tfh.reserved = 0;
	// Write file header
    fwrite( &tfh, 1, sizeof(tfh), fp );
    
    triS32 nframe = 0;
    triImageList* frame = ani->frames;
    while (frame!=0)
    {
		triImage* img = frame->image;
		if (img==0) continue;
		
		#ifdef DEBUG
		triLogPrint("Saving frame %i\n", ++nframe );
		#endif
		triImageChunkHeader tch;
		memset( &tch, 0, sizeof(tch) );
		tch.format = img->format;
		tch.palformat = img->palformat;
		tch.flags = flags;
		tch.delay = frame->delay;
		tch.xOffs = frame->xOffs;
		tch.yOffs = frame->yOffs;
	
		triImageChunk tic;
		tic.width = img->width;
		tic.height = img->height;
		tic.stride = img->stride;
		tic.size = img->size;
		
		triU8* tdata = img->data;
		triS32 talloced = 0;
		
		if (img->swizzled)
			tch.flags |= TRI_IMG_FLAGS_SWIZZLE;
		else
		if (flags&TRI_IMG_FLAGS_SWIZZLE)
		{
			triU8* dst = triMalloc(img->size);
			if (dst==0)
			{
				tch.flags &= ~TRI_IMG_FLAGS_SWIZZLE;
			}
			else
			{
				triS32 bytewidth = img->stride*img->bits>>3;
				swizzle_fast( (triU8*)dst, (triU8*)tdata, bytewidth, img->size / bytewidth );
				tdata = dst;
				talloced = 1;
			}
		}
		
		if (flags&TRI_IMG_FLAGS_WAVELET)
		{
			//TODO: Add wavelet compression
		}
		
		if (flags&TRI_IMG_FLAGS_RLE)
		{
			triU8* rleimg = triMalloc( img->size + (img->size>>4) );
			if (rleimg==0)
			{
				tch.flags &= ~TRI_IMG_FLAGS_RLE;
			}
			else
			{
				triU32 bits = 0;
				switch (img->format)
				{
					case IMG_FORMAT_T4:
						bits = 4;
						break;
					case IMG_FORMAT_T8:
						bits = 8;
						break;
					case IMG_FORMAT_T16:
					case IMG_FORMAT_5650:
					case IMG_FORMAT_5551:
					case IMG_FORMAT_4444:
						bits = 16;
						break;
					case IMG_FORMAT_T32:
					case IMG_FORMAT_8888:
						bits = 32;
						break;
					default:
						bits = 16;
						break;
				}
				tic.size = encodeRLE( tdata, img->size, img->stride, rleimg, img->size + (img->size>>4), bits );
				if (talloced) triFree( tdata );
				tdata = rleimg;
				talloced = 1;
			}
		}
		
		if (flags&TRI_IMG_FLAGS_GZIP)
		{
			triU32 csize = tic.size;
			triU8* cdata = triMalloc( csize );
			if (cdata==0 || compress2( cdata, &csize, tdata, tic.size, 9 )!=Z_OK)
			{
				tch.flags &= ~TRI_IMG_FLAGS_GZIP;
			}
			else
			{
				tic.size = csize;
				if (talloced) triFree(tdata);
				tdata = cdata;
				talloced = 1;
			}
		}
		
		#ifdef DEBUG
		triLogPrint("Writing data...\n" );
		#endif
		// Write chunk header
		fwrite( &tch, 1, sizeof(tch), fp );
		
		// Write palette
		if (tch.format==IMG_FORMAT_T4 || tch.format==IMG_FORMAT_T8 || tch.format==IMG_FORMAT_T16 || tch.format==IMG_FORMAT_T32)
			fwrite( img->palette, 1, (img->format==IMG_FORMAT_T4?16:256)*(img->palformat==IMG_FORMAT_8888?4:2), fp );
		
		// Write chunk
		fwrite( &tic, 1, sizeof(tic), fp );
		
		// Write image data
		fwrite( tdata, 1, tic.size, fp );
	
		if (talloced)
			triFree(tdata);
		
		frame = frame->next;
	}
	fclose(fp);
	#ifdef DEBUG
	triLogPrint("Done.\n");
	#endif
}


#ifdef TRI_SUPPORT_PNG
triVoid triImageSavePng( triChar* name, triImage *img, triS32 saveAlpha )
{
	if (img==0) return;
	if (img->data==0 || img->width==0 || img->height==0) return;
	
	png_structp png_ptr;
	png_infop info_ptr;
	FILE* fp;
	triU8* line;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return;
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return;
	}
	
	triU32 col_type = PNG_COLOR_TYPE_RGB;
	if (saveAlpha!=0)
		col_type = PNG_COLOR_TYPE_RGBA;

	fp = fopen(name, "wb");
	if (!fp) return;
	
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, img->width, img->height,
		8, col_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	
	line = (triU8*) triMalloc(img->width * 4);
	if (line==0)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(fp);
		return;
	}
	
	triU8* src = (triU8*)img->data;
	triU8* src8 = (triU8*)src;
	triU16* src16 = (triU16*)src;
	triU32* src32 = (triU32*)src;

	triS32 x,y,i;
	for (y = 0; y < img->height; y++)
	{
		triU32 swap = 0;
		src8 = (triU8*)src;
		src16 = (triU16*)src;
		src32 = (triU32*)src;
		for (i = 0, x = 0; x < img->width; x++)
		{
			triU32 r = 0, g = 0, b = 0, a = 0;
			triU16 col16;
			triU32 col32;
			triU8 col8;
			switch (img->format)
			{
				case IMG_FORMAT_5650:
					col16 = *src16++;
					RGBA5650(col16,r,g,b,a);
					break;
				case IMG_FORMAT_5551:
					col16 = *src16++;
					RGBA5551(col16,r,g,b,a);
					break;
				case IMG_FORMAT_4444:
					col16 = *src16++;
					RGBA4444(col16,r,g,b,a);
					break;
				case IMG_FORMAT_8888:
					col32 = *src32++;
					RGBA8888(col32,r,g,b,a);
					break;
				case IMG_FORMAT_T4:
					col8 = *src8;
					if (swap==0)
					{
						col8 &= 0xF;
					}
					else
					{
						col8 >>= 4;
						src8++;
					}
					swap ^= 1;
					triImagePaletteGet( img, col8, &r, &g, &b, &a );
					break;
				case IMG_FORMAT_T8:
					col8 = *src8++;
					triImagePaletteGet( img, col8, &r, &g, &b, &a );
					break;
			}
			line[i++] = r;
			line[i++] = g;
			line[i++] = b;
			if (saveAlpha!=0)
				line[i++] = a;
		}
		png_write_row(png_ptr, line);
		src += ((img->stride*img->bits) >> 3);
	}
	triFree(line);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
	
}
#endif // TRI_SUPPORT_PNG


/*
 * Save image as TGA file.
 * Currently supports all triImage formats other than DXT compressed.
 * T4, T16 and T32 are stored as T8 though and shifted to fit (MSBs are taken as indices).
 */
triVoid triImageSaveTga( triChar* name, triImage *img, triS32 saveAlpha, triS32 rle )
	{
	if (img==0) return;
	if (img->data==0 || img->width==0 || img->height==0) return;
	
	FILE* fp;
	triU8* line;

	fp = fopen(name, "wb");
	if (!fp) return;

	TGAFILEHEADER tfh;
    triU8 pal[256*4];
    triS32 x,y;
    
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );

	if (img->palette!=0)
	{
		tfh.ImageTypeCode = 1;
		tfh.ColorMapType = 1;
		triS32 mapsz = (img->format==IMG_FORMAT_T4?16:256);
		tfh.ColorMapLength[1] = mapsz/256;
		tfh.ColorMapLength[0] = mapsz%256;
		switch (img->palformat)
		{
			case IMG_FORMAT_8888:
				tfh.ColorMapESize = (saveAlpha?32:24);
				break;
			case IMG_FORMAT_5650:
			case IMG_FORMAT_5551:
			case IMG_FORMAT_4444:
				tfh.ColorMapESize = 16;
				break;
			default:
				tfh.ColorMapESize = (saveAlpha?32:24);
				break;
		}
		tfh.Depth = 8;
		triU8* ipal = (triU8*)img->palette;
		switch (img->palformat)
		{
			case IMG_FORMAT_5551:
				for (y=0;y<mapsz;y++)
				{
					triU16 col16 = (ipal[0 + y*2] | (ipal[1 + y*2] << 8));
					col16 = (((col16&0x1F)<<10) | (col16&0x3E0) | ((col16>>10)&0x1F) | (saveAlpha?(col16&0x8000):(1<<15)));
					pal[0 + y*2] = (col16 >> 8);
					pal[1 + y*2] = (col16 & 0xFF);
				}
				break;
			case IMG_FORMAT_5650:
				for (y=0;y<mapsz;y++)
				{
					triU16 col16 = (ipal[0 + y*2] | (ipal[1 + y*2] << 8));
					col16 = (((col16&0x1F)<<10) | ((col16 >> 1)&0x3E0) | ((col16>>11)&0x1F) | (1<<15));
					pal[0 + y*2] = (col16 >> 8);
					pal[1 + y*2] = (col16 & 0xFF);
				}
				break;
			case IMG_FORMAT_8888:
				if (saveAlpha)
					for (y=0;y<mapsz;y++)
					{
						pal[0 + y*4] = ipal[2 + y*4];
						pal[1 + y*4] = ipal[1 + y*4];
						pal[2 + y*4] = ipal[0 + y*4];
						pal[3 + y*4] = ipal[3 + y*4];
					}
				else
					for (y=0;y<mapsz;y++)
					{
						pal[0 + y*3] = ipal[2 + y*4];
						pal[1 + y*3] = ipal[1 + y*4];
						pal[2 + y*3] = ipal[0 + y*4];
					}
				break;
			case IMG_FORMAT_4444:
				for (y=0;y<mapsz;y++)
				{
					triU16 col16 = (ipal[0 + y*2] | (ipal[1 + y*2] << 8));
					col16 = (((col16&0xF)<<10) | ((col16 << 1)&0x3C0) | ((col16>>7)&0x1E) | (saveAlpha?(col16&0x8000):(1<<15)));
					pal[0 + y*2] = (col16 >> 8);
					pal[1 + y*2] = (col16 & 0xFF);
				}
				break;
		}
	}
	else
	{
		tfh.ImageTypeCode = 2;
		switch (img->format)
		{
			case IMG_FORMAT_8888:
				tfh.Depth = (saveAlpha?32:24);
				break;
			case IMG_FORMAT_5650:
			case IMG_FORMAT_5551:
			case IMG_FORMAT_4444:
				tfh.Depth = 16;
				break;
			case IMG_FORMAT_T4:
			case IMG_FORMAT_T8:
				tfh.Depth = 8;
				break;
			default:
				tfh.Depth = (saveAlpha?32:24);
				break;
		}
	}
	
	if (rle)
		tfh.ImageTypeCode |= 8;
	
	tfh.ImageDescrip = TGA_DESC_VERTICAL;
	tfh.Width[1] = img->width/256;
	tfh.Width[0] = img->width%256;
	tfh.Height[1] = img->height/256;
	tfh.Height[0] = img->height%256;
	
	#ifdef DEBUG
	triLogPrint("TGAHEADER (%i)>\n", sizeof(TGAFILEHEADER));
	triLogPrint("triImageIDSize: %i\n", tfh.ImageIDSize);
    triLogPrint("ColorMapType: %i\n", tfh.ColorMapType);
    triLogPrint("triImageTypeCode: %i\n", tfh.ImageTypeCode);                          // triImage Type (normal/paletted/grayscale/rle)
    triLogPrint("ColorMapOrigin: %i\n", RD16(tfh.ColorMapOrigin));
    triLogPrint("ColorMapLength: %i\n", RD16(tfh.ColorMapLength));                         // Palette Size
    triLogPrint("ColorMapESize: %i\n", tfh.ColorMapESize);							// Size in bits of one Palette entry
	triLogPrint("OrigX: %i\n", RD16(tfh.OriginX));                                  // Width of triImage
    triLogPrint("OrigY: %i\n", RD16(tfh.OriginY));
    triLogPrint("Width: %i\n", RD16(tfh.Width));                                  // Width of triImage
    triLogPrint("Height: %i\n", RD16(tfh.Height));                                 // Height of triImage
    triLogPrint("Depth: %i\n", tfh.Depth);                                  // Bits per Pixel of triImage
    triLogPrint("triImageDescrip: %x\n", tfh.ImageDescrip);
	#endif
    
    fwrite( &tfh, 1, sizeof(tfh), fp );
    if (tfh.ColorMapType==1)
		fwrite( pal, 1, (img->format==IMG_FORMAT_T4?16:256)*(tfh.ColorMapESize>>3), fp );

	line = (triU8*) triMalloc(img->width * 4);
	if (line==0)
	{
		fclose(fp);
		return;
	}
	memset( line, 0, img->width*4 );
	
	triU8* rleline = 0;
	if (rle)
	{
		rleline = (triU8*) triMalloc(img->width * 6);
		// if we fail here, we just fall back to no RLE encoding rather than quitting
		if (rleline==0)
		{
			rle = 0;
			tfh.ImageTypeCode &= ~8;
			long pos = fseek( fp, 0, SEEK_CUR );
			fseek( fp, 0, SEEK_SET );
			fwrite( &tfh, 1, sizeof(tfh), fp );
			fseek( fp, pos, SEEK_SET );
		}
	}

	triU8* src = (triU8*)img->data;
	for (y=0;y<img->height;y++)
	{
		switch (img->format)
		{
			case IMG_FORMAT_5551:
				for (x=0;x<img->width;x++)
				{
					triU16 col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
					col16 = (((col16&0x1F)<<10) | (col16&0x3E0) | ((col16>>10)&0x1F) | (saveAlpha?(col16&0x8000):(1<<15)));
					line[0 + x*2] = (col16 >> 8);
					line[1 + x*2] = (col16 & 0xFF);
				}
				break;
			case IMG_FORMAT_5650:
				for (x=0;x<img->width;x++)
				{
					triU16 col16 = (src[0 + y*2] | (src[1 + y*2] << 8));
					col16 = (((col16&0x1F)<<10) | ((col16 >> 1)&0x3E0) | ((col16>>11)&0x1F) | (1<<15));
					line[0 + y*2] = (col16 >> 8);
					line[1 + y*2] = (col16 & 0xFF);
				}
				break;
			case IMG_FORMAT_8888:
				if (saveAlpha)
					for (x=0;x<img->width;x++)
					{
						line[0 + x*4] = src[2 + x*4];
						line[1 + x*4] = src[1 + x*4];
						line[2 + x*4] = src[0 + x*4];
						line[3 + x*4] = src[3 + x*4];
					}
				else
					for (x=0;x<img->width;x++)
					{
						line[0 + x*3] = src[2 + x*4];
						line[1 + x*3] = src[1 + x*4];
						line[2 + x*3] = src[0 + x*4];
					}
				break;
			case IMG_FORMAT_4444:
				for (x=0;x<img->width;x++)
				{
					triU16 col16 = (src[0 + y*2] | (src[1 + y*2] << 8));
					col16 = (((col16&0xF)<<10) | ((col16 << 1)&0x3C0) | ((col16>>7)&0x1E) | (saveAlpha?(col16&0x8000):(1<<15)));
					line[0 + x*2] = (col16 >> 8);
					line[1 + x*2] = (col16 & 0xFF);
				}
				break;
			case IMG_FORMAT_T8:
				memcpy( line, src, img->width );
				break;
			case IMG_FORMAT_T4:
				for (x=0;x<img->width;x++)
				{
					if (x&1)
						line[x] = src[x>>1] & 0xF;
					else
						line[x] = src[x>>1] >> 4;
				}
				break;
			case IMG_FORMAT_T16:
				for (x=0;x<img->width;x++)
				{
					line[x] = src[x<<1];
				}
				break;
			case IMG_FORMAT_T32:
				for (x=0;x<img->width;x++)
				{
					line[x] = src[x<<2];
				}
				break;
		}
		if (rle)
		{
			long sz = encodeRLE( line, img->width*(img->bits>>3), 0, rleline, img->width*6, tfh.Depth );
			fwrite( rleline, 1, sz, fp );
		}
		else
			fwrite( line, 1, img->width*(img->bits>>3), fp );
		src += img->stride*(img->bits>>3);
	}
	if (rle)
		triFree(rleline);
	triFree(line);
	fclose(fp);
}




/*
 * Save image as TRI file.
 * Currently supports all triImage formats, as well as swizzling and RLE and GZIP compression.
 */
triVoid triImageSaveTri( triChar* name, triImage *img, triU32 flags )
{
	if (img==0) return;
	if (img->data==0 || img->width==0 || img->height==0) return;
	
	FILE* fp;

	fp = fopen(name, "wb");
	if (!fp) return;

	flags &= 0xFF;
	int talloced = 0;
	triImageFileHeader tfh;
	memcpy( tfh.magic, "triImage", 8 );
	tfh.numFrames = 1;
	tfh.reserved = 0;
	// Write file header
    fwrite( &tfh, 1, sizeof(tfh), fp );
    
    triImageChunkHeader tch;
    memset( &tch, 0, sizeof(tch) );
    tch.format = img->format;
    tch.palformat = img->palformat;
    tch.flags = flags;
    tch.numLevels = img->levels;

	// Write chunk header
	fwrite( &tch, 1, sizeof(tch), fp );
    
    // Write palette
    if (tch.format==IMG_FORMAT_T4 || tch.format==IMG_FORMAT_T8 || tch.format==IMG_FORMAT_T16 || tch.format==IMG_FORMAT_T32)
		fwrite( img->palette, 1, (img->format==IMG_FORMAT_T4?16:256)*(img->palformat==IMG_FORMAT_8888?4:2), fp );
	
	triU32 level = 0;
	triU32 bits = 0;
	switch (img->format)
	{
		case IMG_FORMAT_T4:
			bits = 4;
			break;
		case IMG_FORMAT_T8:
			bits = 8;
			break;
		case IMG_FORMAT_T16:
		case IMG_FORMAT_5650:
		case IMG_FORMAT_5551:
		case IMG_FORMAT_4444:
			bits = 16;
			break;
		case IMG_FORMAT_T32:
		case IMG_FORMAT_8888:
			bits = 32;
			break;
		default:
			bits = 16;
			break;
	}
	
	triMipLevel* lev = img->level;
	while (level<=img->levels)
	{
		triImageChunk tic;
		triU8* tdata = 0;
		talloced = 0;
		
		if (level==0)
		{
			tic.width = img->width;
			tic.height = img->height;
			tic.stride = img->stride;
			tic.size = img->size;
			tdata = img->data;
		}
		else
		{
			tic.width = lev->width;
			tic.height = lev->height;
			tic.stride = lev->stride;
			tic.size = lev->size;
			tdata = lev->data;
			lev = lev->next;
		}
		
		if (img->swizzled)
			tch.flags |= TRI_IMG_FLAGS_SWIZZLE;
		else
		if (flags&TRI_IMG_FLAGS_SWIZZLE)
		{
			triU8* dst = triMalloc(tic.size);
			if (dst==0)
			{
				// Always save at least the first level
				if (level==0)
					tch.flags &= ~TRI_IMG_FLAGS_SWIZZLE;
				else
				{
					tch.numLevels = level;
					break;
				}
			}
			else
			{
				triS32 bytewidth = tic.stride*img->bits>>3;
				swizzle_fast( (triU8*)dst, (triU8*)tdata, bytewidth, tic.size / bytewidth );
				tdata = dst;
				talloced = 1;
			}
		}
		
		if (flags&TRI_IMG_FLAGS_WAVELET)
		{
			//TODO: Add wavelet compression
		}
		
		if (flags&TRI_IMG_FLAGS_RLE)
		{
			triU8* rleimg = triMalloc( tic.size + (tic.size>>4) );
			if (rleimg==0)
			{
				if (level==0)
					tch.flags &= ~TRI_IMG_FLAGS_RLE;
				else
				{
					if (talloced) triFree( tdata );
					tch.numLevels = level;
					break;
				}
			}
			else
			{
				tic.size = encodeRLE( tdata, tic.size, tic.stride, rleimg, tic.size + (tic.size>>4), bits );
				if (talloced) triFree( tdata );
				tdata = rleimg;
				talloced = 1;
			}
		}
		
		if (flags&TRI_IMG_FLAGS_GZIP)
		{
			triU32 csize = tic.size;
			triU8* cdata = triMalloc( csize );
			if (cdata==0 || compress2( cdata, &csize, tdata, tic.size, 9 )!=Z_OK)
			{
				if (level==0)
					tch.flags &= ~TRI_IMG_FLAGS_GZIP;
				else
				{
					if (talloced) triFree( tdata );
					tch.numLevels = level;
					break;
				}
			}
			else
			{
				tic.size = csize;
				if (talloced) triFree(tdata);
				tdata = cdata;
				talloced = 1;
			}
		}
		
		sceKernelDcacheWritebackAll();
		// Write chunk
		fwrite( &tic, 1, sizeof(tic), fp );
		
		// Write image data
		fwrite( tdata, 1, tic.size, fp );
	
		if (talloced)
			triFree(tdata);
		
		if (lev==0)
		{
			tch.numLevels = level;
			break;
		}
		level++;
	}
	
	fseek( fp, sizeof(tfh), SEEK_SET );
	// ReWrite chunk header (might have gotten different flags)
	fwrite( &tch, 1, sizeof(tch), fp );
	
	fclose(fp);
}
