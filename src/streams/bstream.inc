
// DO NOT INCLUDE THIS FILE DIRECTLY INTO YOUR CODE
// INCLUDE "streams.h" AND LINK STREAMS.O INSTEAD!

#include <string.h> // memcpy
#include "streams.h"


int bstream_close( stream* s )
{
	if (s->_stream->mode&STREAM_CREATE)
	{
		free( ((bstream*)s->_stream)->data );
		((bstream*)s->_stream)->data = 0;
	}
	return (1);
}


// copy size bits from src+(srcOff bits) to dst+(dstOff bits)
long bitcpy( unsigned char* dst, int dstOff, unsigned char* src, int srcOff, long size )
{
	unsigned char* src8 = src + (srcOff>>3);
	unsigned char* dst8 = dst + (dstOff>>3);
	srcOff &= 7;
	dstOff &= 7;
	
	if (dstOff==0 && srcOff==0)
	{
		memcpy_vfpu( dst8, src8, (size+7)>>3 );
		return( size );
	}

	unsigned char srcb1, dstb1 = *dst8, srcb2 = *src8;
	long sz = size;
	if (dstOff==0)
	{
		unsigned char mask = (1 << srcOff)-1;
		while (sz>0)
		{
			srcb1 = srcb2;
			srcb2 = *src8++;
			unsigned char s = ((srcb1 >> srcOff) & ~mask) | ((srcb2 << (8-srcOff)) & mask);
			*dst8++ = s;
			sz -= 8;
		}
		return( size );
	}
	
	if (srcOff==0)
	{
		unsigned char mask = (1 << dstOff)-1;
		while (sz>0)
		{
			unsigned char s = *src8++;
			unsigned char d = ((dstb1 & ~mask) | ((s << dstOff) & mask));
			dstb1 = (s >> (8-dstOff));
			*dst8++ = d;
			*dst8 = dstb1;
			sz -= 8;
		}
		return( size );
	}
	
	unsigned char smask = (1 << srcOff)-1;
	unsigned char dmask = (1 << dstOff)-1;
	while (sz>0)
	{
		srcb1 = srcb2;
		srcb2 = *src8++;
		unsigned char s = ((srcb1 >> srcOff) & ~smask) | ((srcb2 << (8-srcOff)) & smask);
		unsigned char d = ((dstb1 & ~dmask) | ((s << dstOff) & dmask));
		dstb1 = (s >> (8-dstOff));
		*dst8++ = d;
		*dst8 = dstb1;
		sz -= 8;
	}
	return(size);
}


// read size number of bits from stream into buf
long bstream_read( stream* s, void* buf, unsigned long size )
{
	bstream* m = (bstream*)s->_stream;
	
	if (m->data==0 || m->size==0) return(-1);
	
	if (size==0 || m->len==0) return(0);
	
	if (m->cur==0)
	{
		m->cur = m->data+(m->pos>>5);
		m->bpos = m->pos&31;
		if (m->len>m->pos)
			m->buf1 = *m->cur;
		if (m->len>m->pos+32)
			m->buf2 = *(m->cur+1);
	}
	
	//if (m->bpos+size<64)
	{
	}
	if (size<=32)
	{
		unsigned int bits = (m->buf1>>m->bpos) | (m->buf2<<(32-m->bpos));
		// we can fill from our current bitbuffer
		
		if (((u32)buf&0x3)==0)
			*(u32*)buf = (bits & ((1<<size)-1));
		else
		{
			unsigned int sz = size;
			unsigned char* bbuf = (unsigned char*)buf;
			while (sz>0)
			{
				*bbuf++ = (unsigned char)(bits&0xFF);
				bits >>= 8;
				sz -= 8;
			}
		}
		m->bpos += size;
		m->pos += size;
		if (m->bpos >= 32)
		{
			m->bpos -= 32;
			m->buf1 = m->buf2;
			if (m->pos<m->len)
			{
				m->cur++;
				m->buf2 = *m->cur;
			}
		}
		if (m->pos>=m->len)
			s->_stream->eos = 1;
		return(size);
	}
	
	long remain = m->len-m->pos;
	if (remain>size) remain = size;
	
	if ((m->bpos&7)==0)
		memcpy_vfpu( buf, (void*)m->cur + (m->bpos>>3), ((remain+7)>>3) );
	else
	{
		bitcpy( buf, 0, (unsigned char*)m->cur, m->bpos, remain );
		/*unsigned int bits = m->buf>>m->bpos;
		unsigned int nbits;
		unsigned int shift = m->bpos;
		unsigned int bshift = 32-m->bpos;
		unsigned char* bbuf = (unsigned char*)buf;
		
		unsigned long sz = remain;
		// copy in blocks
		while (sz>0)
		{
			m->cur++;
			//if (sz<32) printf("%x:%i,",m->cur,*m->cur);
			nbits = *(m->cur);
			bits |= (nbits << bshift);
			int i=0;
			for (;i<4 && sz>0;i++,sz -= 8, bits >>= 8)
				*bbuf++ = (bits&0xFF);

			bits = (nbits >> shift);
		}*/
	}
	m->pos += remain;
	m->cur = m->data+(m->pos>>5);
	m->bpos = m->pos&31;
	if (m->pos<m->len)
		m->buf1 = *m->cur;
	if (m->pos+32<m->len)
		m->buf2 = *(m->cur+1);

	if (m->pos>=m->len)
		s->_stream->eos = 1;
	return(remain);
}


// write size number of bits from buf into stream
long bstream_write( stream* s, void* buf, unsigned long size )
{
	bstream* m = (bstream*)s->_stream;
	
	if (m->data==0) s->_stream->mode |= STREAM_CREATE|STREAM_APPEND;
	
	if (m->cur==0)
	{
		m->cur = m->data+(m->pos>>5);
		m->bpos = m->pos&31;
		m->buf1 = *m->cur;
		m->buf2 = *(m->cur+1);
	}
	
	
	if (m->pos+size>(m->size<<3))
	{
		if ((s->_stream->mode & STREAM_APPEND)!=0)
		{
			// we need to increase our membuffer (and may so)
			m->size += 4;
			s->_stream->size = m->size;
			m->data = realloc( m->data, m->size );
			m->cur = m->data+(m->pos>>5);
		}
		else
		{
			size = (m->size<<3)-m->pos;
		}
	}

	if (m->bpos+size<32)
	{
		// we can fill into our current bitbuffer
		unsigned int bits;
		unsigned int sz = size;
		unsigned char* bbuf = (unsigned char*)buf;
		if (((u32)buf&0x3)==0)
			m->buf1 = (*(unsigned int*)buf << m->bpos);
		else
			while (sz>0)
			{
				bits = *bbuf++;
				m->buf1 |= (bits << m->bpos);
				m->bpos += 8;
				sz -= 8;
			}
		m->pos += size;
		m->bpos = m->pos&31;
		return(size);
	}

	if ((m->bpos&7)==0)
		memcpy_vfpu( (void*)m->cur + (m->bpos>>3), buf, ((size+7)>>3) );
	else
	{
		unsigned int bits;
		unsigned long sz = size;
		unsigned char* bbuf = (unsigned char*)buf;
		
		// copy in blocks
		while (sz>0)
		{
			// fill current bitbuffer to 32bit and flush it to stream
			for (;sz>0 && m->bpos<32;sz -= 8,m->bpos += 8)
			{
				bits = *bbuf++;
				m->buf1 |= (bits << m->bpos);
			}
			*m->cur++ = m->buf1;
			m->bpos -= 32;
			m->buf1 = bits >> (8 - m->bpos) | (m->buf2 & ~((1 << m->bpos)-1));
		}
	}
	m->pos += size;
	m->cur = m->data+(m->pos>>5);
	m->bpos = m->pos&31;
	m->buf1 = *m->cur;
	m->buf2 = *(m->cur+1);
	
	if (m->pos>=m->len)
	{
		m->len = m->pos;
		s->_stream->eos = 1;
	}
	return(size);
}

long bstream_seek( stream* s, long offs, unsigned int dir )
{
	bstream* m = (bstream*)s->_stream;
	
	switch (dir) {
	case STREAM_START:
		m->pos = offs;
		break;
	case STREAM_END:
		m->pos = m->len + offs;
		break;
	case STREAM_CUR:
	default:
		m->pos += offs;
		break;
	}
	
	s->_stream->eos = 0;
	if (m->pos>=m->len)
	{
		m->pos = m->len;
		s->_stream->eos = 1;
	}
	if (m->pos<0) m->pos = 0;
	
	m->cur = m->data + (m->pos>>5);
	m->bpos = (m->pos&31);
	m->buf1 = *m->cur;
	m->buf2 = *(m->cur+1);
	return(m->pos);
}

long bstream_tell( stream* s )
{
	return (((bstream*)s->_stream)->pos);
}


stream_protocol bstream_protocol = {
	STREAM_TYPE_BITS,
	0,
	0,
	STREAM_RDWR,
	{0},
	stream_bopen,
	bstream_close,
	bstream_read,
	bstream_write,
	bstream_seek,
	bstream_tell };
