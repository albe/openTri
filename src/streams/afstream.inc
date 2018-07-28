// DO NOT INCLUDE THIS FILE DIRECTLY INTO YOUR CODE
// INCLUDE "streams.h" AND LINK STREAMS.O INSTEAD!

#include <pspiofilemgr.h>
#include "streams.h"


/*
 * Buffered file stream async reader/writer interface
 *
 * Tries to read ahead for read operations, and buffers write operations.
 * 
 */
int afstream_close( stream* s )
{
	return sceIoClose( ((afstream*)s->_stream)->fd );
}


static void afstream_sync( afstream* f )
{
	// An async request is still pending, so wait for that to finish
	if (f->wait & ~BFILE_WAIT_WRITEINCOMPLETE)
	{
		long long ret;
		f->async_error = sceIoWaitAsync( f->fd, &ret );
		
		if (f->wait & BFILE_WAIT_READ)
			f->bsize += ret;	// we have filled our buffer with readahead data that is probably useless now, but we will handle that later

		if (f->wait & BFILE_WAIT_WRITE)
		{
			f->fpos = sceIoLseek32( f->fd, 0, STREAM_CUR );
			if (f->wait & BFILE_WAIT_WRITEINCOMPLETE)
				f->fpos += f->cur-f->data;

			if (f->fpos >= f->fsize)
			{
				f->fsize = f->fpos;
				((stream_base*)f)->size = f->fpos;
				((stream_base*)f)->eos = 1;
			}
		}
		f->wait &= BFILE_WAIT_WRITEINCOMPLETE;
	}
}


long afstream_read( stream* s, void* buf, unsigned long size )
{
	afstream* f = (afstream*)s->_stream;
	
	if (buf==0 || size==0) return(-1);
	
	// wait for previous async operations to finish
	afstream_sync( f );
	
	// If a previous write operation left something in our write buffer, we must flush now!
	if (f->wait & BFILE_WAIT_WRITEINCOMPLETE)
	{
		sceIoWrite( f->fd, f->data, (f->cur - f->data) );
		f->fpos = sceIoLseek32( f->fd, 0, STREAM_CUR );
		f->wait &= ~BFILE_WAIT_WRITEINCOMPLETE;
	}
	
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
			memcpy_vfpu( buf, f->cur, f->bsize );
			remain -= f->bsize;
			buf += f->bsize;
			f->fpos += f->bsize;
			f->cur = f->data;
			f->bsize = 0;
		}
	}
	
	if (remain>0)
	{
		long read = sceIoRead( f->fd, buf, remain );
		
		if ((f->fpos = sceIoLseek32( f->fd, 0, STREAM_CUR ))==f->fsize)
			s->_stream->eos = 1;

		if (read<=0)
			return (remain<size?size-remain:-1);
		remain-=read;
	} else
		if (f->fpos==f->fsize)		// we drained the buffer and previously finished the file
				s->_stream->eos = 1;
	
	if (s->_stream->eos==0)
	{
		sceIoReadAsync( f->fd, f->data, BFILE_BUFFER_SIZE );
		f->wait = BFILE_WAIT_READ;
	}
	
	return(size-remain);
}


// This will ALWAYS return size, because any errors cannot be depicted until the async writes are finished
long afstream_write( stream* s, void* buf, unsigned long size )
{
	afstream* f = (afstream*)s->_stream;
	
	if (buf==0 || size==0) return(-1);
	
	// An async request is still pending, so wait for that to finish
	afstream_sync( f );

	long ret_sz = size;
	// There is still something left in the write buffer from a previous write operation
	if (f->wait & BFILE_WAIT_WRITEINCOMPLETE)
	{
		long remain = BFILE_BUFFER_SIZE - (f->cur - f->data);	// how much still fits into the writebuffer
		if (size<remain)
		{
			memcpy_vfpu( f->cur, buf, size );
			f->bsize -= size;	// Maybe we didn't completely overwrite our readahead buffer
			if (f->bsize<0) f->bsize = 0; // Check if we wrote over the readahead buffer (writing over the end of the file)
			f->cur += size;
			f->fpos += size;
			return(size);
		}
		memcpy_vfpu( f->cur, buf, remain );	// fill writebuffer
		f->bsize = 0;
		f->cur = f->data;
		buf += remain;
		size -= remain;
		sceIoWriteAsync( f->fd, f->data, BFILE_BUFFER_SIZE );	// and flush it to file
		f->wait |= BFILE_WAIT_WRITE;
		f->wait &= ~BFILE_WAIT_WRITEINCOMPLETE;
	}
	
	// Our write buffer is empty here and we have still have size bytes to write
	long remain = size % BFILE_BUFFER_SIZE;
	
	// It's more than can fit into our buffer, so we send a write request for all above what can fit
	if (size>=BFILE_BUFFER_SIZE)
	{
		long long ret;
		if (size<ret_sz)
			f->async_error = sceIoWaitAsync( f->fd, &ret );	// wait for previous buffer flush
		sceIoWriteAsync( f->fd, buf, size );
		f->wait |= BFILE_WAIT_WRITE;
		remain = 0;
	}
	
	if (remain>0)
	{
		// Still something left to fill into the writebuffer:
		memcpy_vfpu( f->data, buf + size-remain, remain );
		f->bsize -= remain;
		if (f->bsize<0) f->bsize = 0; // Check if we wrote over the readahead buffer (writing over the end of the file)
		f->cur = f->data + remain;
		f->wait |= BFILE_WAIT_WRITEINCOMPLETE;
	}
	else
	{
		f->bsize = 0;
		f->cur = f->data;
		f->wait &= ~BFILE_WAIT_WRITEINCOMPLETE;
		// This would stall the async write and fail anyway... thanks to lame Sony Engineers
		/*if (s->_stream->mode & STREAM_RDWR)
		{
			sceIoReadAsync( f->fd, f->data, BFILE_BUFFER_SIZE );		// fill our readahead buffer
			f->wait |= BFILE_WAIT_READ;					// can cause the case that both a read and write op are pending!
		}*/
	}
	
	return(ret_sz);

}

long afstream_seek( stream* s, long offs, unsigned int dir )
{
	afstream* f = (afstream*)s->_stream;
	
	afstream_sync( f );
	
	if (f->wait & BFILE_WAIT_WRITEINCOMPLETE)	// flush write buffer
	{
		sceIoWrite( f->fd, f->data, (f->cur - f->data) );
		f->wait &= ~BFILE_WAIT_WRITEINCOMPLETE;
	}

	long oldpos = f->fpos;
	s->_stream->eos = 0;
	long pos = sceIoLseek32( f->fd, offs, dir );
	if (pos >= f->fsize)
		s->_stream->eos = 1;
	
	// We changed the file position and there is something in our buffer, but maybe we don't need to flush it?
	long bufsize = (f->bsize+(f->cur-f->data));
	if (s->_stream->mode & (STREAM_RDWR|STREAM_RDONLY))
	{
		if (oldpos!=f->fpos && bufsize > 0)
		{
			long off = f->fpos - oldpos;
			if (f->cur+off>=f->data && (f->cur+offs) < (f->data+bufsize))
			{
				f->bsize -= offs;
				f->cur += offs;
			}
			else
			{
				f->cur = f->data;
				f->bsize = 0;
			}
		}
		
		if (f->bsize==0 && s->_stream->eos==0)
		{
			sceIoReadAsync( f->fd, f->data, BFILE_BUFFER_SIZE );		// fill our readahead buffer
			f->wait |= BFILE_WAIT_READ;
		}
	}
	f->fpos = pos;
	return pos;
}

long afstream_tell( stream* s )
{
	afstream_sync( (afstream*)s->_stream );
	return ((afstream*)s->_stream)->fpos;
}


stream_protocol afstream_protocol = {
	STREAM_TYPE_AFILE,
	0,
	0,
	STREAM_ALL,
	{0},
	stream_afopen,
	afstream_close,
	afstream_read,
	afstream_write,
	afstream_seek,
	afstream_tell };
