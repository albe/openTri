// DO NOT INCLUDE THIS FILE DIRECTLY INTO YOUR CODE
// INCLUDE "streams.h" AND LINK STREAMS.O INSTEAD!

#include "streams.h"


int mstream_close( stream* s )
{
	if (s->_stream->mode&STREAM_CREATE)
	{
		free( ((mstream*)s->_stream)->data );
		((mstream*)s->_stream)->data = 0;
	}
	return (1);
}


long mstream_read( stream* s, void* buf, unsigned long size )
{
	mstream* m = (mstream*)s->_stream;
	
	if (m->data==0 || m->size==0) return(-1);
	
	if (m->cur==0) m->cur = m->data+m->pos;
	
	if (m->pos+size<m->size)
	{
		memcpy_vfpu( buf, m->cur, size );
		m->pos += size;
		m->cur += size;
		return (size);
	}
	
	long remain = m->size-m->pos;
	memcpy_vfpu( buf, m->cur, remain );
	m->pos += remain;
	m->cur += remain;
	s->_stream->eos = 1;
	return(remain);
}

long mstream_write( stream* s, void* buf, unsigned long size )
{
	mstream* m = (mstream*)s->_stream;
	
	if (m->data==0 || m->size==0)
	{
		s->_stream->mode |= STREAM_CREATE|STREAM_APPEND;
		m->data = malloc(size);
		if (m->data==0)
			m->size = 0;
		snprintf(s->_stream->name, 256, "%p", m->data );
	}
	if (m->data==0 || m->size==0) return(-1);
	
	if (m->cur==0) m->cur = m->data+m->pos;
	
	if (m->pos+size<m->size)
	{
		memcpy_vfpu( m->cur, buf, size );
		m->pos += size;
		m->cur += size;
		return (size);
	}
	
	// do a realloc to make room for the extra write request if stream can be appended to
	if (s->_stream->mode&STREAM_APPEND)
	{
		m->data = realloc( m->data, m->pos+size );
		// Critical error!
		if (m->data==0)
		{
			m->pos = 0;
			m->cur = 0;
			m->size = 0;
			s->_stream->size = 0;
			s->_stream->eos = 1;
			return (-1);
		}
		m->cur = m->data + m->pos;
		m->size = m->pos + size;
		s->_stream->size = m->size;
	}
	
	long remain = m->size-m->pos;
	memcpy_vfpu( m->cur, buf, remain );
	m->pos += remain;
	m->cur += remain;
	s->_stream->eos = 1;
	return(remain);
}

long mstream_seek( stream* s, long offs, unsigned int dir )
{
	mstream* m = (mstream*)s->_stream;
	
	switch (dir)
	{
		case STREAM_START:
			m->pos = offs;
			break;
		case STREAM_END:
			m->pos = m->size + offs;
			break;
		case STREAM_CUR:
		default:
			m->pos += offs;
			break;
	}
	
	s->_stream->eos = 0;
	if (m->pos>=m->size)
	{
		m->pos = m->size;
		s->_stream->eos = 1;
	}
	if (m->pos<0) m->pos = 0;
	
	m->cur = m->data + m->pos;
	return(m->pos);
}

long mstream_tell( stream* s )
{
	return (((mstream*)s->_stream)->pos);
}


stream_protocol mstream_protocol = {
	STREAM_TYPE_MEM,
	0,
	0,
	STREAM_RDWR,
	{0},
	stream_mopen,
	mstream_close,
	mstream_read,
	mstream_write,
	mstream_seek,
	mstream_tell };
