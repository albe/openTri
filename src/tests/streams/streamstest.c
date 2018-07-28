#include <pspkernel.h>
#include <psprtc.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "../../triTypes.h"
#include "../../triLog.h"
#include "../../streams/streams.h"


PSP_MODULE_INFO("triStreamsTest", 0x0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

static int isrunning = 1;

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	isrunning = 0;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread,
				     0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


void test_byte_read( stream* s, char* buf )
{
	triLogPrint("Testing reading byte per byte until eof...\n");
	long i = 0;
	long read = 0;
	while (!stream_eos( s ))
	{
		char tst;
		read += stream_read( s, &buf[read], 1 );
		i++;
	}
	triLogPrint("Did %i read(s)\n", i );
	triLogPrint("Read %i bytes\n", read );
	stream_seek( s, 0, STREAM_SET );
}

void test_block_read( stream* s, char* buf )
{
	triLogPrint("Testing reading in blocks of 1024byte until eof...\n");
	long i = 0;
	long read = 0;
	while (!stream_eos( s ))
	{
		read += stream_read( s, &buf[read], 1024 );
		i++;
	}
	triLogPrint("Did %i read(s)\n", i );
	triLogPrint("Read %i bytes\n", read );
	stream_seek( s, 0, STREAM_SET );
}

void test_read_all( stream* s, char* buf )
{
	triLogPrint("Testing reading whole file until eof...\n");
	triLogPrint("Stream size: %i\n", stream_size( s ) );
	long i = 0;
	long read = 0;
	while (!stream_eos( s ))
	{
		read += stream_read( s, buf, stream_size( s ) );
		i++;
	}
	triLogPrint("Did %i read(s)\n", i );
	triLogPrint("Read %i bytes\n", read );
	stream_seek( s, 0, STREAM_SET );
}


void test_byte_write( stream* s, char* buf, long size )
{
	triLogPrint("Testing writing byte per byte...\n");
	long i = 0;
	long wrote = 0;
	while (wrote<size)
	{
		wrote += stream_write( s, &buf[wrote], 1 );
		i++;
	}
	triLogPrint("Did %i write(s)\n", i );
	triLogPrint("Wrote %i bytes\n", wrote );
	triLogPrint("Stream size: %i\n", stream_size( s ) );
	stream_seek( s, 0, STREAM_SET );
}


void test_write_all( stream* s, char* buf, long size )
{
	triLogPrint("Testing writing whole file...\n");
	long i = 0;
	long wrote = 0;
	while (wrote<size)
	{
		wrote += stream_write( s, buf, size );
		i++;
	}
	triLogPrint("Did %i read/write(s)\n", i );
	triLogPrint("Wrote %i bytes\n", wrote );
	stream_seek( s, 0, STREAM_SET );
}


void test_block_write( stream* s, char* buf, long size )
{
	triLogPrint("Testing writing blocks...\n");
	long i = 0;
	long wrote = 0;
	long block = 1024;
	while (wrote<size && (i<10))
	{
		wrote += stream_write( s, &buf[wrote], block );
		if (wrote+block>size) block = size-wrote;
		triLogPrint("wrote: %i\n", wrote);
		i++;
	}
	triLogPrint("Did %i read/write(s)\n", i );
	triLogPrint("Wrote %i bytes\n", wrote );
	stream_seek( s, 0, STREAM_SET );
}


void test_byte_rdwr( stream* s, char* buf )
{
	triLogPrint("Testing mixed reading&writing byte per byte until eof...\n");
	long i = 0;
	long wrote = 0;
	long read = 0;
	while (!stream_eos( s ))
	{
		char tst;
		if (i&1)
			wrote += stream_write( s, &buf[wrote], 1 );
		else
			read += stream_read( s, &tst, 1 );
		i++;
	}
	triLogPrint("Did %i read/write(s)\n", i );
	triLogPrint("Wrote %i bytes\n", wrote );
	triLogPrint("Read %i bytes\n", read );
	stream_seek( s, 0, STREAM_SET );
}







void test_fstream( char* buf, long size )
{
	triLogPrint("Testing fstream for errors...\n");
	stream* f = stream_fopen( "test.dat", STREAM_RDONLY );
	if (f==0)
		triLogPrint("Error opening test.dat!\n");
	test_byte_read( f, buf );
	test_block_read( f, buf );
	test_read_all( f, buf );
	stream_close( f );
	
	unlink("test.out1");
	f = stream_fopen( "test.out1", STREAM_RDWR|STREAM_CREATE );
	if (f==0)
		triLogPrint("Error opening test.out1!\n");
	test_byte_write( f, buf, size );
	test_block_write( f, buf, size );
	test_write_all( f, buf, size );
	test_byte_rdwr( f, buf );
	stream_close( f );
	triLogPrint("Finished test.\n\n");
}




void test_bufstream( char* buf, long size, char* test )
{
	triLogPrint("Testing bufstream for errors...\n");
	stream* bufs = stream_bufopen( STREAM_TYPE_FILE, "test.dat", STREAM_RDWR );
	if (bufs==0)
		triLogPrint("Error opening test.dat!\n");
	memset(buf,0,size);
	test_byte_read( bufs, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
	memset(buf,0,size);
	test_block_read( bufs, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
	memset(buf,0,size);
	test_read_all( bufs, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
	stream_close( bufs );
	
	unlink("test.out3");
	bufs = stream_bufopen( STREAM_TYPE_FILE, "test.out3", STREAM_RDWR|STREAM_CREATE );
	if (bufs==0)
		triLogPrint("Error opening test.out3!\n");
	test_byte_rdwr( bufs, buf );
	test_byte_write( bufs, buf, size );
	test_block_write( bufs, buf, size );
	test_write_all( bufs, buf, size );
	stream_close( bufs );
	triLogPrint("Finished test.\n\n");
}



void test_mstream( char* buf, long size )
{
	triLogPrint("Testing mstream for errors...\n");
	char* buf2 = malloc( size );
	stream* mem = stream_mopen( buf, size );
	test_byte_read( mem, buf2 );
	test_block_read( mem, buf2 );
	test_read_all( mem, buf2 );
	stream_close( mem );
	
	mem = stream_mopen( 0, size );
	if (mem==0)
		triLogPrint("Error opening stream!\n");
	test_byte_write( mem, buf2, size );
	test_block_write( mem, buf, size );
	test_write_all( mem, buf2, size );
	test_byte_rdwr( mem, buf2 );
	stream_close( mem );
	triLogPrint("Finished test.\n\n");
	free(buf2);
}


void test_afstream( char* buf, long size, char* test )
{
	triLogPrint("Testing afstream for errors...\n");
	stream* bf = stream_afopen( "test.dat", STREAM_RDWR );
	if (bf==0)
		triLogPrint("Error opening test.dat!\n");
	test_byte_read( bf, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
	test_block_read( bf, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
	test_read_all( bf, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
	stream_close( bf );
	
	unlink("test.out2");
	bf = stream_afopen( "test.out2", STREAM_RDWR|STREAM_CREATE );
	if (bf==0)
		triLogPrint("Error opening test.out2!\n");
	test_byte_write( bf, buf, size );
	test_block_write( bf, buf, size );
	test_write_all( bf, buf, size );
	//test_byte_rdwr( bf, buf );
	triLogPrint("Finished test.\n\n");
	stream_close( bf );
}




void test_bufstream_copy( char* in, char* out, char* buf, int size )
{
	triLogPrint("Testing bufstream file copy for errors...\n");
	stream* bufs1 = stream_bufopen( STREAM_TYPE_FILE, in, STREAM_RDONLY );
	if (bufs1==0)
		triLogPrint("Error opening %s!\n", in);
	
	unlink(out);
	stream* bufs2 = stream_bufopen( STREAM_TYPE_FILE, out, STREAM_WRONLY|STREAM_CREATE );
	if (bufs2==0)
		triLogPrint("Error opening %s!\n", out);

	long read = 0;
	long wrote = 0;
	long i = 0;
	while (!stream_eos( bufs1 ))
	{
		long towrite = stream_read( bufs1, buf, size );
		read += towrite;
		wrote += stream_write( bufs2, buf, towrite );
		i++;
	}
	triLogPrint("Did %i read/write(s)\n", i );
	triLogPrint("Wrote %i bytes\n", wrote );
	triLogPrint("Read %i bytes\n", read );
	stream_close( bufs1 );
	stream_close( bufs2 );
	triLogPrint("Finished test.\n\n");
}



void test_memcpy_vfpu( unsigned char* buf, long size )
{
	triLogPrint("Testing memcpy_vfpu for errors...\n");
	unsigned char* buf2 = malloc(size+16);
	
	long tst;

	triLogPrint("Testing aligned memcpy...\n");
	memset(buf2,0,size);
	buf2[size] = 0xAB;
	memcpy_vfpu( buf2, buf, size );
	tst = size-1;
	while (tst>=0)
	{
		if (buf[tst]!=buf2[tst])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, &buf[tst], &buf2[tst], buf[tst], buf2[tst]);
		}
		tst--;
	}
	if (buf2[size] != 0xAB) triLogPrint("Buffer overrun detected!\n");
	
	triLogPrint("Testing word unaligned dst memcpy...\n");
	memset(buf2,0,size+4);
	buf2[size+4] = 0xAB;
	memcpy_vfpu( buf2+4, buf, size );
	tst = 0;
	while (tst<size)
	{
		if (buf[tst]!=buf2[tst+4])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, &buf[tst], &buf2[tst+4], buf[tst], buf2[tst+4]);
		}
		tst++;
	}
	if (buf2[4+size] != 0xAB) triLogPrint("Buffer overrun detected!\n");

	triLogPrint("Testing word unaligned src memcpy...\n");
	memset(buf2,0,size);
	buf2[size-16] = 0xAB;	
	memcpy_vfpu( buf2, buf+4, size-16 );
	tst = size-17;
	while (tst>=0)
	{
		if (buf[tst+4]!=buf2[tst])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, &buf[tst+4], &buf2[tst], buf[tst+4], buf2[tst]);
		}
		tst--;
	}
	if (buf2[size-16] != 0xAB) triLogPrint("Buffer overrun detected!\n");
	
	triLogPrint("Testing byte unaligned dst memcpy...\n");
	memset(buf2,0,size+1);
	buf2[1+size] = 0xAB;
	memcpy_vfpu( buf2+1, buf, size );
	tst = size-1;
	while (tst>=0)
	{
		if (buf[tst]!=buf2[tst+1])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, &buf[tst], &buf2[tst+1], buf[tst], buf2[tst+1]);
		}
		tst--;
	}
	if (buf2[1+size] != 0xAB) triLogPrint("Buffer overrun detected!\n");

	triLogPrint("Testing byte unaligned src memcpy...\n");
	memset(buf2,0,size);
	buf2[size-16] = 0xAB;
	memcpy_vfpu( buf2, buf+1, size-16 );
	tst = size-17;
	while (tst>=0)
	{
		if (buf[tst+1]!=buf2[tst])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, &buf[tst+1], &buf2[tst], buf[tst+1], buf2[tst]);
		}
		tst--;
	}
	if (buf2[size-16] != 0xAB) triLogPrint("Buffer overrun detected!\n");
	
	free(buf);
	triLogPrint("Finished test.\n\n");
}


void test_memcpy_vfpu_bench( char* buf, long size )
{
	size = 256*1024;
	triLogPrint("Testing memcpy_vfpu for speed...\n");
	char* buf2 = malloc( size+16 );
	
	long i = 0;
	u64 last_tick, end_tick;
	u32 tick_frequency = sceRtcGetTickResolution();

	triLogPrint("Testing memcpy...\n");
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2, buf, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("QWord aligned memcpy                : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2+1, buf, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Byte unaligned (dst) memcpy         : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2+4, buf, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (dst) memcpy         : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2, buf+1, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Byte unaligned (src) memcpy         : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2, buf+4, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src) memcpy         : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2+4, buf+4, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src&dst) memcpy     : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	

	triLogPrint("Testing memcpy_vfpu...\n");
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2, buf, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("QWord aligned memcpy_vfpu           : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2+1, buf, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Byte unaligned (dst) memcpy_vfpu    : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2+4, buf, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (dst) memcpy_vfpu    : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2, buf+1, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Byte unaligned (src) memcpy_vfpu    : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2, buf+4, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src) memcpy_vfpu    : %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2+4, buf+4, size );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src&dst) memcpy_vfpu: %.2fMB/s\n", (float)((size)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	free(buf);
	triLogPrint("Finished test.\n\n");
}


/* Simple thread */
int main(int argc, char **argv)
{
	SetupCallbacks();
	
	pspDebugScreenInit();
	
	triLogInit();

	char* buf;
	unsigned char* ubuf;
	stream* f = stream_fopen( "test.dat", STREAM_RDONLY );
	unsigned int size = stream_size( f ); //4048+16;
	buf = malloc( size );
	ubuf = (unsigned char*)buf;
	char* buf_test = malloc( size );
	stream_read( f, buf_test, size );
	stream_close( f );
	

	test_fstream( buf, size );
	test_afstream( buf,size, buf_test );
	test_bufstream( buf, size, buf_test );
	test_mstream( buf, size );

/*	test_bufstream_copy( "test.dat", "test.out", buf, size );

	unsigned int testsize = 2048;
	long i = 0;
	for (;i<testsize;i++)
		ubuf[i] = (i&0xFF);
	
	test_memcpy_vfpu( ubuf, testsize );
	test_memcpy_vfpu_bench( buf, testsize );
	*/
	free( buf );
	triLogPrint("Finished all tests.\n\n");
	sceKernelExitGame();
	return 0;
}
