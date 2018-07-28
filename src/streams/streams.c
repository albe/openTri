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


#ifdef __PSP__
void memcpy_vfpu( void* dst, void* src, unsigned long size )
{
	if (size<16)
	{
		memcpy( dst, src, size );
		return;
	}
	
	u8* src8 = (u8*)src;
	u8* dst8 = (u8*)dst;
	
	if ((((u32)src8&0xF)|((u32)dst8&0xF))==0)
	{
		while (size>63)
		{
			asm(".set noreorder\n"
				"lv.q c000, 0+%1\n"
				"lv.q c010, 16+%1\n"
				"lv.q c020, 32+%1\n"
				"lv.q c030, 48+%1\n"
				"sv.q c000, 0+%0\n"
				"sv.q c010, 16+%0\n"
				"sv.q c020, 32+%0\n"
				"sv.q c030, 48+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 64;
			dst8 += 64;
			size -= 64;
		}
		
		while (size>15)
		{
			asm(".set noreorder\n"
				"lv.q c000, 0+%1\n"
				"sv.q c000, 0+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 16;
			dst8 += 16;
			size -= 16;
		}
	}
	else
	if ((((u32)src8&0xF)|((u32)dst8&0x3))==0)
	{
		s32 len = 0x10-((u32)dst8&0xF);
		while (len>0)
		{
			*dst8++ = *src8++;
			size--;
			len--;
		}
		
		while (size>63)
		{
			asm(".set noreorder\n"
				"ulv.q c000, 0+%1\n"
				"ulv.q c010, 16+%1\n"
				"ulv.q c020, 32+%1\n"
				"ulv.q c030, 48+%1\n"
				"sv.q c000, 0+%0\n"
				"sv.q c010, 16+%0\n"
				"sv.q c020, 32+%0\n"
				"sv.q c030, 48+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 64;
			dst8 += 64;
			size -= 64;
		}
		
		while (size>15)
		{
			asm(".set noreorder\n"
				"ulv.q c000, 0+%1\n"
				"sv.q c000, 0+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 16;
			dst8 += 16;
			size -= 16;
		}
	}
	else
	if ((((u32)dst8&0xF)|((u32)src8&0x3))==0)
	{
		while (size>63)
		{
			asm(".set noreorder\n"
				"ulv.q c000, 0+%1\n"
				"ulv.q c010, 16+%1\n"
				"ulv.q c020, 32+%1\n"
				"ulv.q c030, 48+%1\n"
				"sv.q c000, 0+%0\n"
				"sv.q c010, 16+%0\n"
				"sv.q c020, 32+%0\n"
				"sv.q c030, 48+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 64;
			dst8 += 64;
			size -= 64;
		}
		
		while (size>15)
		{
			asm(".set noreorder\n"
				"ulv.q c000, 0+%1\n"
				"sv.q c000, 0+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 16;
			dst8 += 16;
			size -= 16;
		}
	}
	else
	/*if ((((u32)dst8&0x3)|((u32)src8&0x3))==0)
	{
		s32 len = 0x10-((u32)dst8&0xF);
		while (len>0)
		{
			*dst8++ = *src8++;
			size--;
			len--;
		}
		
		while (size>63)
		{
			asm(".set noreorder\n"
				"lv.q c000, 0+%1\n"
				"lv.q c010, 16+%1\n"
				"lv.q c020, 32+%1\n"
				"lv.q c030, 48+%1\n"
				"sv.q c000, 0+%0\n"
				"sv.q c010, 16+%0\n"
				"sv.q c020, 32+%0\n"
				"sv.q c030, 48+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 64;
			dst8 += 64;
			size -= 64;
		}

		while (size>15)
		{
			asm(".set noreorder\n"
				"lv.q c000, 0+%1\n"
				"sv.q c000, 0+%0\n"
				".set reorder\n"
				:"=m"(*dst8):"m"(*src8)
				);
			src8 += 16;
			dst8 += 16;
			size -= 16;
		}
	}
	else*/
	{
		memcpy( dst8, src8, size );
		return;
	}
	
	while (size>0)
	{
		*dst8++ = *src8++;
		size--;
	}
}
#else // __PSP__
void memcpy_vfpu( void* dst, void* src, unsigned long size )
{
	memcpy( dst, src, size );
}
#endif // __PSP__


stream* stream_fopen( char* name, long flags )
{
	stream* s = (stream*)malloc(sizeof(stream) + sizeof(fstream));
	if (s==0) return(0);
	
	s->_stream = (stream_base*)(s+1);
	*s->_stream = fstream_protocol;
	s->_stream->mode &= flags;
	
	fstream* f = (fstream*)s->_stream;
	#ifdef __PSP__
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
	if (flags & STREAM_RDONLY)
	{
		sceIoReadAsync( f->fd, f->data, BFILE_BUFFER_SIZE*2 );
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
