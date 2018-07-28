/*
STREAMS - General purpose IO streams interface
Copyright (C) 2007 Raphael

E-mail:   raphael@fx-world.org
homepage: http://wordpress.fx-world.org
*/


#ifndef __streams_h__
#define __streams_h__


#ifdef _PSP
#include <pspiofilemgr.h>
#endif

#include <zlib.h>


#define STREAM_TYPE_MEM 0x01
#define STREAM_TYPE_FILE 0x02
#define STREAM_TYPE_AFILE 0x03
#define STREAM_TYPE_BITS 0x04
#define STREAM_TYPE_URL 0x05
#define STREAM_TYPE_CON 0x06
// 0x07 - 0x1F are reserved for future hardcoded streams
// 0x20 - 0x7F are reserved for custom streams (use stream_get_typeid())
#define STREAM_TYPE_BUF 0x80
#define STREAM_TYPE_ZIP 0x100


// seek function flags
#define STREAM_START 0x0
#define STREAM_SET STREAM_START
#define STREAM_CUR 0x1
#define STREAM_END 0x2


#ifdef _PSP
#define STREAM_RDONLY PSP_O_RDONLY
#define STREAM_WRONLY PSP_O_WRONLY
#define STREAM_RDWR   PSP_O_RDWR
#define STREAM_APPEND PSP_O_APPEND
#define STREAM_CREATE PSP_O_CREAT
#define STREAM_TRUNC  PSP_O_TRUNC
#else
#define STREAM_RDONLY _O_RDONLY
#define STREAM_WRONLY _O_WRONLY
#define STREAM_RDWR   _O_RDWR
#define STREAM_APPEND _O_APPEND
#define STREAM_CREATE _O_CREAT
#define STREAM_TRUNC  _O_TRUNC
#endif

#define STREAM_ALL (STREAM_RDWR|STREAM_APPEND|STREAM_CREATE|STREAM_TRUNC)

typedef struct stream_struct stream;

typedef stream* (*stream_open_func)( char*, long );
typedef int  (*stream_close_func)( stream* );
typedef long (*stream_read_func)( stream*, void*, unsigned long );
typedef long (*stream_write_func)( stream*, void*, unsigned long );
typedef long (*stream_seek_func)( stream*, long, unsigned int );
typedef long (*stream_tell_func)( stream* );


typedef struct stream_base_struct {
	int					type;			// Stream type ID
	int					eos;			// EOS flag (0 for protocols)
	unsigned int		size;			// Stream size in bytes (0 for protocols)
	unsigned int		mode;			// Stream open mode (possible open modes for protocols)
	char				name[256];		// Stream name (filename/URL/memory address)

	stream_open_func	open;
	stream_close_func	close;
	stream_read_func	read;
	stream_write_func	write;
	stream_seek_func	seek;
	stream_tell_func	tell;
} stream_base, stream_protocol;


typedef struct stream_protocol_list_struct {
	stream_protocol*	protocol;
	struct stream_protocol_list_struct*	next;
	} stream_protocol_list;



// Memory IO stream
typedef struct mstream_struct {
	stream_base	s;
	char*	data;
	char*	cur;

	unsigned long		size;
	unsigned long		pos;
} mstream;



// File IO stream
typedef struct fstream_struct {
	stream_base	s;
	#ifdef _PSP
	int		fd;
	#else
	FILE*	fd;
	#endif
	
	unsigned long	fsize;
} fstream;


#define BFILE_WAIT_READ 1
#define BFILE_WAIT_WRITE 2
#define BFILE_WAIT_WRITEINCOMPLETE 4


#define BFILE_BUFFER_SIZE (1024*128)			// 128kb seems best size for MemStick
// Buffered file IO stream with asynchronous reads/writes
typedef struct afstream_struct {
	stream_base	s;
	#ifdef _PSP
	int		fd;
	#else
	FILE*	fd;
	#endif

	char	data[BFILE_BUFFER_SIZE];		// Read/Write buffer
	char*	cur;
	int		buf;							// Current buffer number
	
	int					wait;				// flag to show that an async read operation is pending
	int					async_error;		// holding last async read/write error status

	unsigned long		bsize;				// size of remaining buffered data
	unsigned long		fsize;				// size of complete file
	unsigned long		fpos;				// current file position (needed for async read/writes)
} afstream;



// Bitstream
typedef struct bstream_struct {
	stream_base s;
	
	unsigned int*		data;
	unsigned int*		cur;
	
	unsigned int		len;			// current number of bits in bitstream
	unsigned int		pos;			// current bitposition in bitstream
	unsigned int		size;			// size of bitstream in bytes (allocated mem)
	unsigned int		buf1;			// current bitbuffer
	unsigned int		buf2;			// next bitbuffer (to make sure 32bit minimum can be read at once without problems)
	unsigned int		bpos;			// current position in bitbuffer
} bstream;



// Buffered IO stream layer
typedef struct bufstream_struct {
	stream_base 		s;					// the stream protocol
	
	stream*				_s;					// the real opened stream

	char	data[BFILE_BUFFER_SIZE];
	char*	cur;							// start of read-ahead buffer
	long	wpos;							// last real write file position
	
	int					wait;
	long				bsize;				// size of remaining buffered data
	long				fpos;				// Virtual file position
} bufstream;


// ZLib compressed stream (not a .zip file reader!)
typedef struct zstream_struct {
	stream_base 		s;					// the stream protocol
	
	stream*				_s;					// the real opened stream

	z_stream			z;					// the z stream structure
	
	char	inbuf[BFILE_BUFFER_SIZE];
	char	outbuf[BFILE_BUFFER_SIZE];
	char*	cur;							// start of read-ahead buffer
	long	wpos;							// last real write file position
	unsigned long	crc;
	
	int					wait;
	long				bsize;				// size of remaining buffered data
	long				fpos;				// Virtual file position
	
	long				spos;				// Start of compressed stream data (no header)
} zstream;


struct stream_struct {
	stream_base*	_stream;
};



stream* stream_fopen( char* name, long flags );		// open local file stream
stream* stream_afopen( char* name, long flags );	// open local async buffered file stream
stream* stream_mopen( char* data, long size );		// open memory stream
stream* stream_bopen( char* data, long size );		// open bitstream
stream* stream_uopen( char* name, long flags );		// open net URL stream (not implemented yet)
stream* stream_open( int type, char* param1, long param2 );	// generic stream opener

stream* stream_zopen( int type, char* param1, long param2 );
stream* stream_bufopen( int type, char* param1, long param2 );	// buffered stream opener (can also use stream_open( STREAM_TYPE_BUF | STREAM_TYPE_*, ... ))


int stream_close( stream* s );

// Read size bytes from stream into buf
long stream_read( stream* s, void* buf, unsigned long size );

// Write size bytes from buf into stream
long stream_write( stream* s, void* buf, unsigned long size );

// Change position in stream
long stream_seek( stream* s, long offs, unsigned int dir );

// Get current position in stream
long stream_tell( stream* s );

// Reset stream position
long stream_rewind( stream* s );

// Get size of stream in bytes
long stream_size( stream* s );

// Get EOS (End of stream) flag of stream
long stream_eos( stream* s );

// Get type ID of stream
long stream_type( stream* s );

// Get stream name (filename/URL/memory address)
char* stream_name( stream* s );


// Extend the supported protocols at runtime
int stream_register_protocol( stream_protocol* p );
int stream_unregister_protocol( stream_protocol* p );
// Retrieve the next free custom type ID or -1 if no more are available
int stream_get_typeid();


// VFPU optimized memcpy function for usage by protocols (falls back to libc malloc if not on PSP)
void* memcpy_vfpu( void* dst, void* src, unsigned int size );


#endif //__streams_h__
