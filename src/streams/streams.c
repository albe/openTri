/*
STREAMS - General purpose IO streams interface
Copyright (C) 2007 Raphael

E-mail:   raphael@fx-world.org
homepage: http://wordpress.fx-world.org
*/

#include <malloc.h>
#include <stdio.h>
#include "streams.h"


#include "fstream.inc"
#include "mstream.inc"
#include "afstream.inc"
#include "bstream.inc"
#include "bufstream.inc"


#ifdef _PSP
void* memcpy_vfpu( void* dst, void* src, unsigned int size )
{
	u8* src8 = (u8*)src;
	u8* dst8 = (u8*)dst;
	
	// < 8 isn't worth trying any optimisations...
	if (size<8) goto bytecopy;

	// < 64 means we don't gain anything from using vfpu...
	if (size<64)
	{
		// Align dst on 4 bytes or just resume if already done
		while (((((u32)dst8) & 0x3)!=0) && size) {
			*dst8++ = *src8++;
			size--;
		}
		if (size<4) goto bytecopy;

		// We are dst aligned now and >= 4 bytes to copy
		u32* src32 = (u32*)src8;
		u32* dst32 = (u32*)dst8;
		switch(((u32)src8)&0x3)
		{
			case 0:
				while (size&0xC)
				{
					*dst32++ = *src32++;
					size -= 4;
				}
				if (size==0) return (dst);		// fast out
				while (size>=16)
				{
					*dst32++ = *src32++;
					*dst32++ = *src32++;
					*dst32++ = *src32++;
					*dst32++ = *src32++;
					size -= 16;
				}
				if (size==0) return (dst);		// fast out
				src8 = (u8*)src32;
				dst8 = (u8*)dst32;
				break;
			default:
				{
					register u32 a, b, c, d;
					while (size>=4)
					{
						a = *src8++;
						b = *src8++;
						c = *src8++;
						d = *src8++;
						*dst32++ = (d << 24) | (c << 16) | (b << 8) | a;
						size -= 4;
					}
					if (size==0) return (dst);		// fast out
					dst8 = (u8*)dst32;
				}
				break;
		}
		goto bytecopy;
	}

	// Align dst on 16 bytes to gain from vfpu aligned stores
	while ((((u32)dst8) & 0xF)!=0 && size) {
		*dst8++ = *src8++;
		size--;
	}

	// We use uncached dst to use VFPU writeback and free cpu cache for src only
	u8* udst8 = (u8*)((u32)dst8 | 0x40000000);
	// We need the 64 byte aligned address to make sure the dcache is invalidated correctly
	u8* dst64a = ((u32)dst8&~0x3F);
	// Invalidate the first line that matches up to the dst start
	if (size>=64)
	asm(".set	push\n"					// save assembler option
		".set	noreorder\n"			// suppress reordering
		"cache 0x1B, 0(%0)\n"
		"addiu	%0, %0, 64\n"
		"sync\n"
		".set	pop\n"
		:"+r"(dst64a));
	switch(((u32)src8&0xF))
	{
		// src aligned on 16 bytes too? nice!
		case 0:
			while (size>=64)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache	0x1B,  0(%2)\n"			// Dcache writeback invalidate
					"lv.q	c000,  0(%1)\n"
					"lv.q	c010, 16(%1)\n"
					"lv.q	c020, 32(%1)\n"
					"lv.q	c030, 48(%1)\n"
					"sync\n"						// Wait for allegrex writeback
					"sv.q	c000,  0(%0), wb\n"
					"sv.q	c010, 16(%0), wb\n"
					"sv.q	c020, 32(%0), wb\n"
					"sv.q	c030, 48(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu  %3, %3, -64\n"
					"addiu	%2, %2, 64\n"
					"addiu	%1, %1, 64\n"
					"addiu	%0, %0, 64\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(dst64a),"+r"(size)
					:
					:"memory"
					);
			}
			if (size>16)
			{
				// Invalidate the last cache line where the max remaining 63 bytes are
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache	0x1B, 0(%0)\n"
					"sync\n"
					".set	pop\n"					// restore assembler option
					::"r"(dst64a));
				while (size>=16)
				{
					asm(".set	push\n"					// save assembler option
						".set	noreorder\n"			// suppress reordering
						"lv.q	c000, 0(%1)\n"
						"sv.q	c000, 0(%0), wb\n"
						// Lots of variable updates... but get hidden in sv.q latency anyway
						"addiu	%2, %2, -16\n"
						"addiu	%1, %1, 16\n"
						"addiu	%0, %0, 16\n"
						".set	pop\n"					// restore assembler option
						:"+r"(udst8),"+r"(src8),"+r"(size)
						:
						:"memory"
						);
				}
			}
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"vflush\n"						// Flush VFPU writeback cache
				".set	pop\n"					// restore assembler option
				);
			dst8 = (u8*)((u32)udst8 & ~0x40000000);
			break;
		// src is only qword unaligned but word aligned? We can at least use ulv.q
		case 4:
		case 8:
		case 12:
			while (size>=64)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache	0x1B,  0(%2)\n"			// Dcache writeback invalidate
					"ulv.q	c000,  0(%1)\n"
					"ulv.q	c010, 16(%1)\n"
					"ulv.q	c020, 32(%1)\n"
					"ulv.q	c030, 48(%1)\n"
					"sync\n"						// Wait for allegrex writeback
					"sv.q	c000,  0(%0), wb\n"
					"sv.q	c010, 16(%0), wb\n"
					"sv.q	c020, 32(%0), wb\n"
					"sv.q	c030, 48(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu  %3, %3, -64\n"
					"addiu	%2, %2, 64\n"
					"addiu	%1, %1, 64\n"
					"addiu	%0, %0, 64\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(dst64a),"+r"(size)
					:
					:"memory"
					);
			}
			if (size>16)
			// Invalidate the last cache line where the max remaining 63 bytes are
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"cache	0x1B, 0(%0)\n"
				"sync\n"
				".set	pop\n"					// restore assembler option
				::"r"(dst64a));
			while (size>=16)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"ulv.q	c000, 0(%1)\n"
					"sv.q	c000, 0(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu	%2, %2, -16\n"
					"addiu	%1, %1, 16\n"
					"addiu	%0, %0, 16\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(size)
					:
					:"memory"
					);
			}
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"vflush\n"						// Flush VFPU writeback cache
				".set	pop\n"					// restore assembler option
				);
			dst8 = (u8*)((u32)udst8 & ~0x40000000);
			break;
		// src not aligned? too bad... have to use unaligned reads
		default:
			while (size>=64)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache 0x1B,  0(%2)\n"

					"lwr	 $8,  0(%1)\n"			//
					"lwl	 $8,  3(%1)\n"			// $8  = *(s + 0)
					"lwr	 $9,  4(%1)\n"			//
					"lwl	 $9,  7(%1)\n"			// $9  = *(s + 4)
					"lwr	$10,  8(%1)\n"			//
					"lwl	$10, 11(%1)\n"			// $10 = *(s + 8)
					"lwr	$11, 12(%1)\n"			//
					"lwl	$11, 15(%1)\n"			// $11 = *(s + 12)
					"mtv	 $8, s000\n"
					"mtv	 $9, s001\n"
					"mtv	$10, s002\n"
					"mtv	$11, s003\n"

					"lwr	 $8, 16(%1)\n"
					"lwl	 $8, 19(%1)\n"
					"lwr	 $9, 20(%1)\n"
					"lwl	 $9, 23(%1)\n"
					"lwr	$10, 24(%1)\n"
					"lwl	$10, 27(%1)\n"
					"lwr	$11, 28(%1)\n"
					"lwl	$11, 31(%1)\n"
					"mtv	 $8, s010\n"
					"mtv	 $9, s011\n"
					"mtv	$10, s012\n"
					"mtv	$11, s013\n"
					
					"lwr	 $8, 32(%1)\n"
					"lwl	 $8, 35(%1)\n"
					"lwr	 $9, 36(%1)\n"
					"lwl	 $9, 39(%1)\n"
					"lwr	$10, 40(%1)\n"
					"lwl	$10, 43(%1)\n"
					"lwr	$11, 44(%1)\n"
					"lwl	$11, 47(%1)\n"
					"mtv	 $8, s020\n"	
					"mtv	 $9, s021\n"
					"mtv	$10, s022\n"
					"mtv	$11, s023\n"

					"lwr	 $8, 48(%1)\n"
					"lwl	 $8, 51(%1)\n"
					"lwr	 $9, 52(%1)\n"
					"lwl	 $9, 55(%1)\n"
					"lwr	$10, 56(%1)\n"
					"lwl	$10, 59(%1)\n"
					"lwr	$11, 60(%1)\n"
					"lwl	$11, 63(%1)\n"
					"mtv	 $8, s030\n"
					"mtv	 $9, s031\n"
					"mtv	$10, s032\n"
					"mtv	$11, s033\n"
					
					"sync\n"
					"sv.q 	c000,  0(%0), wb\n"
					"sv.q 	c010, 16(%0), wb\n"
					"sv.q 	c020, 32(%0), wb\n"
					"sv.q 	c030, 48(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu	%3, %3, -64\n"
					"addiu	%2, %2, 64\n"
					"addiu	%1, %1, 64\n"
					"addiu	%0, %0, 64\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(dst64a),"+r"(size)
					:
					:"$8","$9","$10","$11","memory"
					);
			}
			if (size>16)
			// Invalidate the last cache line where the max remaining 63 bytes are
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"cache	0x1B, 0(%0)\n"
				"sync\n"
				".set	pop\n"					// restore assembler option
				::"r"(dst64a));
			while (size>=16)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"lwr	 $8,  0(%1)\n"			//
					"lwl	 $8,  3(%1)\n"			// $8  = *(s + 0)
					"lwr	 $9,  4(%1)\n"			//
					"lwl	 $9,  7(%1)\n"			// $9  = *(s + 4)
					"lwr	$10,  8(%1)\n"			//
					"lwl	$10, 11(%1)\n"			// $10 = *(s + 8)
					"lwr	$11, 12(%1)\n"			//
					"lwl	$11, 15(%1)\n"			// $11 = *(s + 12)
					"mtv	 $8, s000\n"
					"mtv	 $9, s001\n"
					"mtv	$10, s002\n"
					"mtv	$11, s003\n"

					"sv.q	c000, 0(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu	%2, %2, -16\n"
					"addiu	%1, %1, 16\n"
					"addiu	%0, %0, 16\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(size)
					:
					:"$8","$9","$10","$11","memory"
					);
			}
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"vflush\n"						// Flush VFPU writeback cache
				".set	pop\n"					// restore assembler option
				);
			dst8 = (u8*)((u32)udst8 & ~0x40000000);
			break;
	}
	
bytecopy:
	// Copy the remains byte per byte...
	while (size--)
	{
		*dst8++ = *src8++;
	}
	
	return (dst);
}
#else // _PSP
void* memcpy_vfpu( void* dst, void* src, unsigned int size )
{
	return memcpy( dst, src, size );
}
#endif // _PSP


stream* stream_fopen( char* name, long flags )
{
	stream* s = (stream*)malloc(sizeof(stream) + sizeof(fstream));
	if (s==0) return(0);
	
	s->_stream = (stream_base*)(s+1);
	*s->_stream = fstream_protocol;
	s->_stream->mode &= flags;
	
	fstream* f = (fstream*)s->_stream;
	#ifdef _PSP
	f->fd = sceIoOpen( name, s->_stream->mode, 0777 );
	if (f->fd<0)
	#else
	f->fd = fopen( name, s->_stream->mode );
	if (f->fd==0)
	#endif
	{
		//free(s->_stream);
		free(s);
		return(0);
	}
	snprintf(s->_stream->name, 256, "%s", name );
	
	f->fsize = sceIoLseek( f->fd, 0, PSP_SEEK_END );
	s->_stream->size = f->fsize;
	sceIoLseek( f->fd, 0, PSP_SEEK_SET );
	return(s);
}

stream* stream_afopen( char* name, long flags )
{
	stream* s = (stream*)malloc(sizeof(stream) + sizeof(afstream));
	if (s==0) return(0);
	
	s->_stream = (stream_base*)(s+1);
	*s->_stream = afstream_protocol;
	s->_stream->mode &= flags;

	afstream* f = (afstream*)s->_stream;
	f->fd = sceIoOpen( name, s->_stream->mode, 0777 );
	if (f->fd<0)
	{
		//free( s->_stream );
		free( s );
		return(0);
	}
	snprintf(s->_stream->name, 256, "%s", name );
	
	f->cur = f->data;
	f->bsize = 0;
	f->fsize = sceIoLseek( f->fd, 0, PSP_SEEK_END );
	f->async_error = 0;
	f->fpos = 0;
	s->_stream->size = f->fsize;
	sceIoLseek( f->fd, 0, PSP_SEEK_SET );
	if (flags & (STREAM_RDONLY|STREAM_RDWR))
	{
		sceIoReadAsync( f->fd, f->data, BFILE_BUFFER_SIZE );
		f->wait = BFILE_WAIT_READ;
	}
	return(s);
}

stream* stream_mopen( char* data, long size )
{
	stream* s = (stream*)malloc(sizeof(stream) + sizeof(mstream));
	if (s==0) return(0);
	
	s->_stream = (stream_base*)(s+1);
	if (data==0)
	{
		s->_stream->mode |= STREAM_CREATE|STREAM_APPEND;
		data = malloc(size);
		if (data==0)
			size = 0;
	}
	*s->_stream = mstream_protocol;
	
	snprintf(s->_stream->name, 256, "%p", data );
	s->_stream->size = size;
	
	((mstream*)s->_stream)->data = data;
	((mstream*)s->_stream)->cur  = data;
	((mstream*)s->_stream)->size = size;
	((mstream*)s->_stream)->pos  = 0;
	
	return(s);
}



stream* stream_bopen( char* data, long size )
{
	stream* s = (stream*)malloc(sizeof(stream) + sizeof(bstream));
	if (s==0) return(0);
	
	s->_stream = (stream_base*)(s+1);
	*s->_stream = bstream_protocol;
	
	// User did not specify a memory location, so we assume he wants to create one
	if (data==0)
	{
		s->_stream->mode |= STREAM_CREATE|STREAM_APPEND;
		data = malloc(((size+7)>>3));
		if (data==0)
			size = 0;
	}
	/*else
		// If our stream sits in user memory, we may append to it (through realloc)
		// - don't do this, since user might want to depend on the pointer being intact after write operations!
		if (((u32)data&~0x40000000) >= 0x08800000 && ((u32)data&~0x40000000) < 0x08800000+0x01800000) s->_stream->mode |= STREAM_APPEND;
	*/
	snprintf(s->_stream->name, 256, "%p", data );
	
	// We need memory aligned to 4 bytes
	if (((u32)data&3)!=0)
	{
		//free( s->_stream );
		free( s );
		return(0);
	}


	unsigned int* idata = (unsigned int*)data;
	((bstream*)s->_stream)->data = idata;
	((bstream*)s->_stream)->cur  = idata;
	((bstream*)s->_stream)->len  = size;
	((bstream*)s->_stream)->size = ((size+7)>>3);
	((bstream*)s->_stream)->pos  = 0;
	((bstream*)s->_stream)->bpos = 0;
	((bstream*)s->_stream)->buf1 = 0;
	((bstream*)s->_stream)->buf2 = 0;
	if (idata!=0)
	{
		((bstream*)s->_stream)->buf1 = *idata;
		((bstream*)s->_stream)->buf2 = *(idata+1);
	}
	s->_stream->size = ((size+7)>>3);
	return(s);
}


stream* stream_uopen( char* name, long flags )
{
	return(0);
}


stream* stream_zopen( int type, char* param1, long param2 )
{
	return(0);
	/*type &= 0x7F;
	
	stream* s = (stream*)malloc(sizeof(stream) + sizeof(zstream));
	if (s==0) return(0);
	
	s->_stream = (stream_base*)(s+1);

	zstream* zs = (zstream*)s->_stream;
	zs->_s = stream_open( type, param1, param2 );
	if (zs->_s==0)
	{
		//free( s->_stream );
		free( s );
		return(0);
	}
	
	*s->_stream = zstream_protocol;
	
	strcpy( s->_stream->name, stream_name( zs->_s ) );
	s->_stream->size = stream_size( zs->_s );
	

	zs->bsize = stream_read( zs->_s, zs->data, BFILE_BUFFER_SIZE );
	zs->cur = bs->data;
	zs->wait = 0;
	zs->fpos = 0;
	
	zs->z.next_in = zs->inbuf;
	zs->z.avail_in = zs->bsize;
	zs->z.next_out = zs->outbuf;
	zs->z.avail_out = BFILE_BUFFER_SIZE;
	zs->z.zalloc = (alloc_func)0;
	zs->z.zfree = (free_func)0;
	zs->z.opaque = 0;
	inflateInit(&zs->z);
	inflate(&zs->z, Z_FINISH);
	zs->z.next_in += zs->z.total_in;
	zs->z.avail_in -= zs->z.total_in;
	zs->z.next_out += zs->z.total_out;
	zs->z.avail_out -= zs->z.total_out;
	return (s);*/
}

stream* stream_bufopen( int type, char* param1, long param2 )
{
	type &= 0x7F;
	// Avoid buffering already memory buffered streams.
	switch (type)
	{
		case STREAM_TYPE_AFILE:
			return stream_afopen( param1, param2 );
		case STREAM_TYPE_MEM:
			return stream_mopen( param1, param2 );
		case STREAM_TYPE_BITS:
			return stream_bopen( param1, param2 );
	}
	
	stream* s = (stream*)malloc(sizeof(stream) + sizeof(bufstream));
	if (s==0) return(0);
	
	s->_stream = (stream_base*)(s+1);

	bufstream* bs = (bufstream*)s->_stream;
	bs->_s = stream_open( type, param1, param2 );
	if (bs->_s==0)
	{
		//free( s->_stream );
		free( s );
		return(0);
	}
	
	*s->_stream = bufstream_protocol;
	
	strcpy( s->_stream->name, stream_name( bs->_s ) );
	s->_stream->size = stream_size( bs->_s );
	
	bs->bsize = stream_read( bs->_s, bs->data, BFILE_BUFFER_SIZE );
	bs->cur = bs->data;
	bs->wait = 0;
	bs->fpos = 0;
	
	return (s);
}


static stream_protocol_list* registered_protocols = 0;


stream* stream_open( int type, char* param1, long param2 )
{
	if (type&STREAM_TYPE_ZIP)
		return stream_zopen( type, param1, param2 );
	if (type&STREAM_TYPE_BUF)
		return stream_bufopen( type, param1, param2 );
	
	type &= 0x7F;
	switch (type)
	{
		case STREAM_TYPE_FILE:
			return stream_fopen( param1, param2 );
		case STREAM_TYPE_AFILE:
			return stream_afopen( param1, param2 );
		case STREAM_TYPE_MEM:
			return stream_mopen( param1, param2 );
		case STREAM_TYPE_BITS:
			return stream_bopen( param1, param2 );
		default:
			{
				stream_protocol_list* list = registered_protocols;
				while (list!=0)
				{
					if (list->protocol->type==type)
					{
						if (list->protocol->open==0) return(0);
						return list->protocol->open( param1, param2 );
					}
					list = list->next;
				}
				return(0);
			}
	}
}


int stream_close( stream* s )
{
	if (s==0 || s->_stream==0 || s->_stream->close==0) return(-1);
	int ret = s->_stream->close( s );
	//free( s->_stream );
	s->_stream = 0;
	free( s );
	return ret;
}


long stream_read( stream* s, void* buf, unsigned long size )
{
	if (s==0 || s->_stream==0 || s->_stream->read==0 || (s->_stream->mode&(STREAM_RDONLY|STREAM_RDWR))==0) return(-1);
	return s->_stream->read( s, buf, size );
}

long stream_write( stream* s, void* buf, unsigned long size )
{
	if (s==0 || s->_stream==0 || s->_stream->write==0 || (s->_stream->mode&(STREAM_WRONLY|STREAM_RDWR))==0) return(-1);
	return s->_stream->write( s, buf, size );
}

long stream_seek( stream* s, long offs, unsigned int dir )
{
	if (s==0 || s->_stream==0 || s->_stream->seek==0) return(-1);
	return s->_stream->seek( s, offs, dir );
}

long stream_tell( stream* s )
{
	if (s==0 || s->_stream==0 || s->_stream->tell==0) return(-1);
	return s->_stream->tell( s );
}

long stream_rewind( stream* s )
{
	return stream_seek( s, 0, SEEK_SET );
}

long stream_eos( stream* s )
{
	if (s==0 || s->_stream==0 || s->_stream->eos!=0) return(1);
	else return(0);
}

long stream_size( stream* s )
{
	if (s==0 || s->_stream==0) return(-1);
	return s->_stream->size;
}

long stream_type( stream* s )
{
	if (s==0 || s->_stream==0) return(-1);
	return (s->_stream->type);
}

char* stream_name( stream* s )
{
	if (s==0 || s->_stream==0) return(0);
	return (s->_stream->name);
}


int stream_register_protocol( stream_protocol* p )
{
	if (p==0) return(0);
	stream_protocol_list* new_item = malloc(sizeof(stream_protocol_list));
	if (new_item==0) return(0);
	new_item->protocol = p;
	new_item->next = registered_protocols;
	registered_protocols = new_item;
	return(1);
}


int stream_unregister_protocol( stream_protocol* p )
{
	stream_protocol_list* list = registered_protocols;
	stream_protocol_list* prev = 0;
	while (list!=0)
	{
		if (list->protocol==p)
		{
			if (prev!=0)
				prev->next = list->next;
			else
				registered_protocols = list->next;
			
			free( list );
			return(1);
		}
		prev = list;
		list = list->next;
	}
	return(0);
}


int stream_get_typeid()
{
	int typeID = 0x20;
	
	while(typeID<0x80)
	{
		int found = 0;
		stream_protocol_list* list = registered_protocols;
		while (list!=0)
		{
			if (list->protocol && list->protocol->type==typeID)
			{
				found = 1;
				break;
			}
		}
		if (found) return(typeID);
	}
	return(-1);
}
