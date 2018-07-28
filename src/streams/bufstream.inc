// DO NOT INCLUDE THIS FILE DIRECTLY INTO YOUR CODE
// INCLUDE "streams.h" AND LINK STREAMS.O INSTEAD!

#include <pspiofilemgr.h>
#include "streams.h"


/*
 * Buffered stream reader/writer interface layer
 *
 * Tries to read ahead for read operations, and buffers write operations.
 * 
 */


static inline void bufstream_fill( stream* s )
{
	bufstream* f = (bufstream*)s->_stream;

	f->cur = f->data;
	f->bsize = stream_read( f->_s, f->data, BFILE_BUFFER_SIZE );
}


static inline void bufstream_flush( bufstream* f )
{
	if ((f->wait & BFILE_WAIT_WRITEINCOMPLETE)==0) return;
	
	long write_sz = (f->cur - f->data);
	long old_pos = stream_tell( f->_s );
	stream_seek( f->_s, f->fpos-write_sz, STREAM_SET );
	long wrote = stream_write( f->_s, f->data, write_sz );
	if (wrote<write_sz)
	{
		f->bsize += write_sz-wrote;
		f->cur -= write_sz-wrote;
		f->fpos -= write_sz-wrote;
	}
	f->wait &= ~BFILE_WAIT_WRITEINCOMPLETE;
	stream_seek( f->_s, old_pos, STREAM_SET );
}


int bufstream_close( stream* s )
{
	bufstream* f = (bufstream*)s->_stream;
	if (f->wait & BFILE_WAIT_WRITEINCOMPLETE)	// flush write buffer
		bufstream_flush( f );
	return stream_close( f->_s );
}


long bufstream_read( stream* s, void* buf, unsigned long size )
{
	bufstream* f = (bufstream*)s->_stream;
	
	if (buf==0 || size==0) return(-1);
	
	long remain = size;
	if (f->bsize>0)
	{
		// feed from buffer
		if (f->bsize>size)
		{
			memcpy_vfpu( buf, f->cur, size );
			f->cur += size;
			f->bsize -= size;
			f->fpos += size;
			return(size);
		} else {
			bufstream_flush( f );
			memcpy_vfpu( buf, f->cur, f->bsize );
			remain -= f->bsize;
			buf += f->bsize;
			f->fpos += f->bsize;
			f->cur = f->data;
			f->bsize = 0;
		}
	}
	else
		bufstream_flush( f );
	
	if (remain>0)
	{
		long read = stream_read( f->_s, buf, remain );
		f->fpos = stream_tell( f->_s );
		s->_stream->eos = stream_eos( f->_s );
		
		if (read<=0)
			return (remain<size?size-remain:-1);
		remain-=read;
	} else
		if (f->fpos>=s->_stream->size)		// we drained the buffer and previously finished the file
				s->_stream->eos = 1;
	
	bufstream_fill( s );
	
	return(size-remain);
}



long bufstream_write( stream* s, void* buf, unsigned long size )
{
	bufstream* f = (bufstream*)s->_stream;
	
	if (buf==0 || size==0) return(-1);
	
	long ret_sz = 0;
	// There is still something left in the write buffer from a previous write operation
	if (f->wait & BFILE_WAIT_WRITEINCOMPLETE)
	{
		long bufsize = (f->cur - f->data);
		long remain = BFILE_BUFFER_SIZE - bufsize;	// how much still fits into the writebuffer
		if (size<remain)
		{
			memcpy_vfpu( f->cur, buf, size );
			f->bsize -= size;	// Maybe we didn't completely overwrite our readahead buffer
			if (f->bsize<0) f->bsize = 0; // Check if we wrote over the readahead buffer (writing over the end of the file)
			f->cur += size;
			f->fpos += size;
			if (s->_stream->size<f->fpos) s->_stream->size = f->fpos;
			return(size);
		}
		memcpy_vfpu( f->cur, buf, remain );	// fill writebuffer
		f->bsize = 0;
		f->cur = f->data;
		buf += remain;
		size -= remain;
		stream_seek( f->_s, f->fpos - bufsize, STREAM_SET );
		ret_sz += stream_write( f->_s, f->data, BFILE_BUFFER_SIZE ) - bufsize;	// and flush it to file
		s->_stream->size = stream_size( f->_s );
		if (ret_sz>=0)
			f->fpos += ret_sz;
		f->wait &= ~BFILE_WAIT_WRITEINCOMPLETE;
	}
	
	// Our write buffer is empty here and we still have size bytes to write
	long remain = size % BFILE_BUFFER_SIZE;
	
	// It's more than can fit into our buffer, so we send a write request for all above what can fit
	if (size>=BFILE_BUFFER_SIZE)
	{
		stream_seek( f->_s, f->fpos, STREAM_SET );
		ret_sz += stream_write( f->_s, buf, size-remain );
		f->bsize = 0;
		f->cur = f->data;
		f->fpos = stream_tell( f->_s );
		s->_stream->size = stream_size( f->_s );
	}
	
	if (remain>0)
	{
		// Still something left to fill into the writebuffer:
		memcpy_vfpu( f->data, buf + size-remain, remain );
		f->bsize -= remain;
		f->fpos += remain;
		if (s->_stream->size<f->fpos) s->_stream->size = f->fpos;
		ret_sz += remain;
		if (f->bsize<0) f->bsize = 0; // Check if we wrote over the readahead buffer (writing over the end of the file)
		f->cur = f->data + remain;
		f->wait |= BFILE_WAIT_WRITEINCOMPLETE;
	}
	
	s->_stream->eos = (s->_stream->size == f->fpos);
	
	return(ret_sz);
}


long bufstream_seek( stream* s, long offs, unsigned int dir )
{
	bufstream* f = (bufstream*)s->_stream;
	
	bufstream_flush( f );

	unsigned long oldpos = f->fpos;
	if (dir==STREAM_CUR)
		stream_seek( f->_s, f->fpos, STREAM_SET );
	
	f->fpos = stream_seek( f->_s, offs, dir );
	s->_stream->eos = stream_eos( f->_s );
	
	// We changed the file position and there is something in our buffer, but maybe we don't need to flush it?
	long bufsize = (f->bsize+(f->cur-f->data));
	if (s->_stream->mode & (STREAM_RDWR|STREAM_RDONLY))
	{
		if (oldpos!=f->fpos && bufsize > 0)
		{
			long off = (long)(f->fpos - oldpos);
			if (f->cur+off>=f->data && (f->cur+off) < (f->data+bufsize))
			{
				f->bsize -= off;
				f->cur += off;
			}
			else
			{
				f->cur = f->data;
				f->bsize = 0;
			}
		}
		
		if ((f->wait & BFILE_WAIT_WRITEINCOMPLETE)==0 && f->bsize<=0)
			bufstream_fill( s );		// fill our readahead buffer
	}
	
	return f->fpos;
}


long bufstream_tell( stream* s )
{
	return ((bufstream*)s->_stream)->fpos;
}


stream_protocol bufstream_protocol = {
	STREAM_TYPE_BUF,
	0,
	0,
	STREAM_ALL,
	{0},
	0,
	bufstream_close,
	bufstream_read,
	bufstream_write,
	bufstream_seek,
	bufstream_tell };
