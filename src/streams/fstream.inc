// DO NOT INCLUDE THIS FILE DIRECTLY INTO YOUR CODE
// INCLUDE "streams.h" AND LINK STREAMS.O INSTEAD!

#include <pspiofilemgr.h>
#include "streams.h"

int fstream_close( stream* s )
{
	return sceIoClose( ((fstream*)s->_stream)->fd );
}

long fstream_read( stream* s, void* buf, unsigned long size )
{
	long read = sceIoRead( ((fstream*)s->_stream)->fd, buf, size );
	if (((fstream*)s->_stream)->fsize == sceIoLseek32( ((fstream*)s->_stream)->fd, 0, STREAM_CUR ))
		s->_stream->eos = 1;

	return read;
}

long fstream_write( stream* s, void* buf, unsigned long size )
{
	long written = sceIoWrite( ((fstream*)s->_stream)->fd, buf, size );
	if (written>0)
	{
		long oldsize = ((fstream*)s->_stream)->fsize;
		long newsize = sceIoLseek32( ((fstream*)s->_stream)->fd, 0, STREAM_CUR );
		if (newsize>oldsize || (s->_stream->mode&STREAM_TRUNC))
		{
			((fstream*)s->_stream)->fsize = newsize;
			s->_stream->size = newsize;
			s->_stream->eos = 1;
		}
	}
	return written;
}

long fstream_seek( stream* s, long offs, unsigned int dir )
{
	s->_stream->eos = 0;
	long pos = sceIoLseek32( ((fstream*)s->_stream)->fd, offs, dir );
	if (pos == ((fstream*)s->_stream)->fsize)
		s->_stream->eos = 1;
	return pos;
}

long fstream_tell( stream* s )
{
	return sceIoLseek32( ((fstream*)s->_stream)->fd, 0, STREAM_CUR );
}


stream_protocol fstream_protocol = {
	STREAM_TYPE_FILE,
	0,
	0,
	STREAM_ALL,
	{0},
	stream_fopen,
	fstream_close,
	fstream_read,
	fstream_write,
	fstream_seek,
	fstream_tell };
