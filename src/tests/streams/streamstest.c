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



void *sceVfpuMemcpy_vnop(void *dst, const void *src, unsigned int n)
{
	unsigned char *d       = (unsigned char *)dst;
	const unsigned char *s = (const unsigned char *)src;

	while (n>0 && (((unsigned int)d) & 0x3F)!=0) {
		*d++ = *s++;
		n--;
	}
	if (n==0) {
		return (dst);
	} else if (n >= 64) {
		unsigned char *ud;
		ud = (unsigned char *)((unsigned int)d | 0x40000000);

		switch (((unsigned int)s) & 0xF) {
			case 0:
				__asm__ volatile (
					".set			push\n"					// save assembler option
					".set			noreorder\n"			// suppress reordering
					"srl			$8, %2, 7\n"			// t0   = (n>>7)
					"beq			$8, $0, 1f\n"			// if (t0==0) goto 1f
					"nop\n"									// waste delay slot
				"0:\n"
					"cache			0x1B,   0(%0)\n"		// Hit Writeback Invalidate(D)
					"lv.q			c000,   0(%1)\n"		// c000 = *(s +   0)
					"lv.q			c010,  16(%1)\n"		// c010 = *(s +  16)
					"lv.q			c020,  32(%1)\n"		// c020 = *(s +  32)
					"lv.q			c030,  48(%1)\n"		// c030 = *(s +  48)
					"lv.q			c100,  64(%1)\n"		// c100 = *(s +  64)
					"lv.q			c110,  80(%1)\n"		// c110 = *(s +  80)
					"lv.q			c120,  96(%1)\n"		// c120 = *(s +  96)
					"lv.q			c130, 112(%1)\n"		// c130 = *(s + 112)
					"cache			0x1B,  64(%0)\n"		// Hit Writeback Invalidate(D)
					"addiu			%0, %0, 128\n"			// d    = d + 128
					"addiu			%1, %1, 128\n"			// s    = s + 128
					"addiu			%2, %2, -128\n"			// n    = n - 128
					"srl			$8, %2, 7\n"			// t0   = (n>>7)
					"sync\n"								// stall until asynchronous writeback from Allegrex
					"sv.q			c000,   0(%3), wb\n"	// *(ud +   0) = c000
					"sv.q			c010,  16(%3), wb\n"	// *(ud +  16) = c010
					"sv.q			c020,  32(%3), wb\n"	// *(ud +  32) = c020
					"sv.q			c030,  48(%3), wb\n"	// *(ud +  48) = c030
					"sv.q			c100,  64(%3), wb\n"	// *(ud +  64) = c100
					"sv.q			c110,  80(%3), wb\n"	// *(ud +  80) = c110
					"sv.q			c120,  96(%3), wb\n"	// *(ud +  96) = c120
					"sv.q			c130, 112(%3), wb\n"	// *(ud + 112) = c130
					"addiu			%3, %3, 128\n"			// ud = ud + 128
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"bne			$8, $0, 0b\n"			// if (t0!=0) goto 0b
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)	(delay slot)
					"srl			$8, %2, 6\n"			// t0   = (n>>6)
					"beq			$8, $0, 9f\n"			// if (t0==0) goto 9f
					"nop\n"									// waste delay slot
				"1:\n"
					"cache			0x1B,  0(%0)\n"			// Hit Writeback Invalidate(D)
					"lv.q			c000,  0(%1)\n"			// c000 = *(s +  0)
					"lv.q			c010, 16(%1)\n"			// c010 = *(s + 16)
					"lv.q			c020, 32(%1)\n"			// c020 = *(s + 32)
					"lv.q			c030, 48(%1)\n"			// c030 = *(s + 48)
					"addiu			%0, %0, 64\n"			// d    = d + 64
					"addiu			%1, %1, 64\n"			// s    = s + 64
					"addiu			%2, %2, -64\n"			// n    = n - 64
					"sync\n"								// stall until asynchronous writeback from Allegrex
					"sv.q			c000,  0(%3), wb\n"		// *(ud +  0) = c000
					"sv.q			c010, 16(%3), wb\n"		// *(ud + 16) = c010
					"sv.q			c020, 32(%3), wb\n"		// *(ud + 32) = c020
					"sv.q			c030, 48(%3), wb\n"		// *(ud + 48) = c030
				"9:\n"
					"vflush\n"								// VFPU writebuffer flushing
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					".set			pop\n"					// restore assembler option
					: "+r"(d), "+r"(s), "+r"(n), "+r"(ud)
					:
					: "$8", "9", "memory"
				);
				break;

			case 4:
				__asm__ volatile (
					".set			push\n"					// save assembler option
					".set			noreorder\n"			// suppress reordering
				"0:\n"
					"addiu			%2, %2, -64\n"			// n    = n - 64
					"cache			0x1B,  0(%0)\n"			// Hit Writeback Invalidate(D)
					"lv.q			c100, -4(%1)\n"			// c100 = *(s -  4)
					"lv.q			c110, 12(%1)\n"			// c110 = *(s + 12)
					"lv.q			c120, 28(%1)\n"			// c120 = *(s + 28)
					"lv.q			c130, 44(%1)\n"			// c130 = *(s + 44)
					"lv.s			s033, 60(%1)\n"			// s033 = *(s + 60)
					"vmmov.t		e000, e101\n"			// e000 = e101
					"vmov.t			r003, r110\n"			// r003 = r110
					"vmov.t			c030, c131\n"			// c030 = c131
					"addiu			%0, %0, 64\n"			// d    = d + 64
					"addiu			%1, %1, 64\n"			// s    = s + 64
					"srl			$8, %2, 6\n"			// t0   = (n>>6)
					"sync\n"								// stall until asynchronous writeback from Allegrex
					"sv.q			c000,  0(%3), wb\n"		// *(ud +  0) = c000
					"sv.q			c010, 16(%3), wb\n"		// *(ud + 16) = c010
					"sv.q			c020, 32(%3), wb\n"		// *(ud + 32) = c020
					"sv.q			c030, 48(%3), wb\n"		// *(ud + 48) = c030
					"addiu			%3, %3, 64\n"			// ud = ud + 64
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"bne			$8, $0, 0b\n"			// if (t0!=0) goto 0b
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)	(delay slot)
				"9:\n"
					"vflush\n"								// VFPU writebuffer flushing
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					".set			pop\n"					// restore assembler option
					: "+r"(d), "+r"(s), "+r"(n), "+r"(ud)
					:
					: "$8", "memory"
				);
				break;

			case 8:
				__asm__ volatile (
					".set			push\n"					// save assembler option
					".set			noreorder\n"			// suppress reordering
				"0:\n"
					"cache			0x1B,  0(%0)\n"			// Hit Writeback Invalidate(D)
					"lv.s			s000,  0(%1)\n"			// s000 = *(s +  0)
					"lv.s			s001,  4(%1)\n"			// s001 = *(s +  4)
					"lv.q			c100,  8(%1)\n"			// c100 = *(s +  8)
					"lv.q			c110, 24(%1)\n"			// c110 = *(s + 24)
					"lv.q			c120, 40(%1)\n"			// c120 = *(s + 40)
					"lv.q			c130, 56(%1)\n"			// c130 = *(s + 56)
					"vmmov.p		e002, e100\n"			// e002 = e100
					"vmov.p			c010, c102\n"			// c010 = c102
					"vmov.p			c020, c112\n"			// c020 = c112
					"vmov.p			c022, c120\n"			// c022 = c120
					"vmov.p			c030, c122\n"			// c030 = c122
					"vmov.p			c032, c130\n"			// c032 = c130
					"addiu			%0, %0, 64\n"			// d    = d + 64
					"addiu			%1, %1, 64\n"			// s    = s + 64
					"addiu			%2, %2, -64\n"			// n    = n - 64
					"srl			$8, %2, 6\n"			// t0 = (n>>6)
					"sync\n"								// stall until asynchronous writeback from Allegrex
					"sv.q			c000,  0(%3), wb\n"		// *(ud +  0) = c000
					"sv.q			c010, 16(%3), wb\n"		// *(ud + 16) = c010
					"sv.q			c020, 32(%3), wb\n"		// *(ud + 32) = c020
					"sv.q			c030, 48(%3), wb\n"		// *(ud + 48) = c030
					"addiu			%3, %3, 64\n"			// ud = ud + 64
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"bne			$8, $0, 0b\n"			// if (t0!=0) goto 0b
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)	(delay slot)
				"9:\n"
					"vflush\n"								// VFPU writebuffer flushing
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					".set			pop\n"					// restore assembler option
					: "+r"(d), "+r"(s), "+r"(n), "+r"(ud)
					:
					: "$8", "memory"
				);
				break;

			case 12:
				__asm__ volatile (
					".set			push\n"					// save assembler option
					".set			noreorder\n"			// suppress reordering
				"0:\n"
					"cache			0x1B,  0(%0)\n"			// Hit Writeback Invalidate(D)
					"lv.s			s000,  0(%1)\n"			// s000 = *(s -  0)
					"lv.q			c100,  4(%1)\n"			// c100 = *(s +  4)
					"lv.q			c110, 20(%1)\n"			// c110 = *(s + 20)
					"lv.q			c120, 36(%1)\n"			// c120 = *(s + 36)
					"lv.q			c130, 52(%1)\n"			// c130 = *(s + 52)
					"vmmov.t		e001, e100\n"			// e001 = e100
					"vmov.t			r010, r103\n"			// r010 = r103
					"vmov.t			c031, c130\n"			// c031 = c130
					"addiu			%0, %0, 64\n"			// d    = d + 64
					"addiu			%1, %1, 64\n"			// s    = s + 64
					"addiu			%2, %2, -64\n"			// n    = n - 64
					"srl			$8, %2, 6\n"			// t0 = (n>>6)
					"sync\n"								// stall until asynchronous writeback from Allegrex
					"sv.q			c000,  0(%3), wb\n"		// *(ud +  0) = c000
					"sv.q			c010, 16(%3), wb\n"		// *(ud + 16) = c010
					"sv.q			c020, 32(%3), wb\n"		// *(ud + 32) = c020
					"sv.q			c030, 48(%3), wb\n"		// *(ud + 48) = c030
					"addiu			%3, %3, 64\n"			// ud = ud + 64
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"bne			$8, $0, 0b\n"			// if (t0!=0) goto 0b
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)	(delay slot)
				"9:\n"
					"vflush\n"								// VFPU writebuffer flushing
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					".set			pop\n"					// restore assembler option
					: "+r"(d), "+r"(s), "+r"(n), "+r"(ud)
					:
					: "$8", "memory"
				);
				break;
			default:
				__asm__ volatile (
					".set			push\n"					// save assembler option
					".set			noreorder\n"			// suppress reordering
				"0:\n"
					"cache			0x1B, 0(%0)\n"			// Hit Writeback Invalidate(D)
					"addiu			%0, %0, 64\n"			// d    = d + 64
					"addiu			%2, %2, -64\n"			// n    = n - 64
					"sync\n"								// stall until asynchronous writeback from Allegrex

					"lwr			$8,  0(%1)\n"			//
					"lwl			$8,  3(%1)\n"			// t0   = *(s + 0)
					"mtv			$8, s000\n"				// s000 = t0
					"lwr			$8,  4(%1)\n"			//
					"lwl			$8,  7(%1)\n"			// t0   = *(s + 4)
					"mtv			$8, s001\n"				// s001 = t0
					"lwr			$8,  8(%1)\n"			//
					"lwl			$8, 11(%1)\n"			// t0   = *(s + 8)
					"mtv			$8, s002\n"				// s002 = t0
					"lwr			$8, 12(%1)\n"			//
					"lwl			$8, 15(%1)\n"			// t0   = *(s + 12)
					"mtv			$8, s003\n"				// s003 = t0

					"lwr			$8, 16(%1)\n"			//
					"lwl			$8, 19(%1)\n"			// t0   = *(s + 0)
					"mtv			$8, s100\n"				// s000 = t0
					"lwr			$8, 20(%1)\n"			//
					"lwl			$8, 23(%1)\n"			// t0   = *(s + 4)
					"mtv			$8, s101\n"				// s001 = t0
					"lwr			$8, 24(%1)\n"			//
					"lwl			$8, 27(%1)\n"			// t0   = *(s + 8)
					"mtv			$8, s102\n"				// s002 = t0
					"lwr			$8, 28(%1)\n"			//
					"lwl			$8, 31(%1)\n"			// t0   = *(s + 12)
					"mtv			$8, s103\n"				// s003 = t0
					
					"lwr			$8, 32(%1)\n"			//
					"lwl			$8, 35(%1)\n"			// t0   = *(s + 0)
					"mtv			$8, s200\n"				// s000 = t0
					"lwr			$8, 36(%1)\n"			//
					"lwl			$8, 39(%1)\n"			// t0   = *(s + 4)
					"mtv			$8, s201\n"				// s001 = t0
					"lwr			$8, 40(%1)\n"			//
					"lwl			$8, 43(%1)\n"			// t0   = *(s + 8)
					"mtv			$8, s202\n"				// s002 = t0
					"lwr			$8, 44(%1)\n"			//
					"lwl			$8, 47(%1)\n"			// t0   = *(s + 12)
					"mtv			$8, s203\n"				// s003 = t0

					"lwr			$8, 48(%1)\n"			//
					"lwl			$8, 51(%1)\n"			// t0   = *(s + 0)
					"mtv			$8, s300\n"				// s000 = t0
					"lwr			$8, 52(%1)\n"			//
					"lwl			$8, 55(%1)\n"			// t0   = *(s + 4)
					"mtv			$8, s301\n"				// s001 = t0
					"lwr			$8, 56(%1)\n"			//
					"lwl			$8, 59(%1)\n"			// t0   = *(s + 8)
					"mtv			$8, s302\n"				// s002 = t0
					"lwr			$8, 60(%1)\n"			//
					"lwl			$8, 63(%1)\n"			// t0   = *(s + 12)
					"mtv			$8, s303\n"				// s003 = t0
					
					"srl			$8, %2, 6\n"			// t0   = (n>>6)
					"addiu			%1, %1, 64\n"			// s    = s  + 64
					"sv.q			c000,  0(%3), wb\n"		// *(ud +  0) = c000
					"sv.q			c100, 16(%3), wb\n"		// *(ud + 16) = c100
					"sv.q			c200, 32(%3), wb\n"		// *(ud + 32) = c200
					"sv.q			c300, 48(%3), wb\n"		// *(ud + 48) = c300
					"addiu			%3, %3, 64\n"			// ud   = ud + 64
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)
					"bne			$8, $0, 0b\n"			// if (t0!=0) goto 0b
					"vnop\n"								// VFPU bug workaround (sv.q,wb needs trailing 5 VNOPs)	(delay slot)

				"9:\n"
					"vflush\n"								// VFPU writebuffer flushing
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					"vnop\n"								// VFPU bug workaround (vflush needs trailing 4 VNOPs)
					".set			pop\n"					// restore assembler option
					: "+r"(d), "+r"(s), "+r"(n), "+r"(ud)
					:
					: "$8", "memory"
				);
				break;
		}
	}

	while (n>0) {
		*d++ = *s++;
		n--;
	}
	return (dst);
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
	test_byte_read( bufs, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
	test_block_read( bufs, buf );
	if (memcmp( buf, test, size )!=0)
		triLogPrint("Error reading. Data does not match.\n");
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


void test_afstream( char* buf, long size )
{
	triLogPrint("Testing afstream for errors...\n");
	stream* bf = stream_afopen( "test.dat", STREAM_RDWR );
	if (bf==0)
		triLogPrint("Error opening test.dat!\n");
	test_byte_read( bf, buf );
	test_block_read( bf, buf );
	test_read_all( bf, buf );
	stream_close( bf );
	
	unlink("test.out2");
	bf = stream_afopen( "test.out2", STREAM_RDWR|STREAM_CREATE );
	if (bf==0)
		triLogPrint("Error opening test.out2!\n");
	test_byte_write( bf, buf, size );
	test_block_write( bf, buf, size );
	test_write_all( bf, buf, size );
	test_byte_rdwr( bf, buf );
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
	buf2[64+16+15] = 0xAB;
	memcpy_vfpu( buf2, buf, 64+16+15 );
	tst = 64+16+14;
	while (tst>=0)
	{
		if (buf[tst]!=buf2[tst])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, buf, buf2, buf[tst], buf2[tst+1]);
		}
		tst--;
	}
	if (buf2[64+16+15] != 0xAB) triLogPrint("Buffer overrun detected!\n");
	
	triLogPrint("Testing word unaligned dst memcpy...\n");
	memcpy_vfpu( buf2+4, buf, 64+16+15 );
	buf2[4+64+16+15] = 0xAB;
	tst = 64+16+14;
	while (tst>=0)
	{
		if (buf[tst]!=buf2[tst+4])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, buf, buf2+4, buf[tst], buf2[tst+4]);
		}
		tst--;
	}
	if (buf2[4+64+16+15] != 0xAB) triLogPrint("Buffer overrun detected!\n");

	triLogPrint("Testing word unaligned src memcpy...\n");
	memcpy_vfpu( buf2, buf+4, 64+16+15 );
	buf2[64+16+15] = 0xAB;
	tst = 64+16+14;
	while (tst>=0)
	{
		if (buf[tst+4]!=buf2[tst])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, buf+4, buf2, buf[tst], buf2[tst+4]);
		}
		tst--;
	}
	if (buf2[64+16+15] != 0xAB) triLogPrint("Buffer overrun detected!\n");
	
	triLogPrint("Testing byte unaligned dst memcpy...\n");
	memcpy_vfpu( buf2+1, buf, 64+16+15 );
	tst = 64+16+14;
	buf2[1+64+16+15] = 0xAB;
	while (tst>=0)
	{
		if (buf[tst]!=buf2[tst+1])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, buf, buf2+1, buf[tst], buf2[tst+1]);
		}
		tst--;
	}
	if (buf2[1+64+16+15] != 0xAB) triLogPrint("Buffer overrun detected!\n");

	triLogPrint("Testing byte unaligned src memcpy...\n");
	memcpy_vfpu( buf2, buf+1, 64+16+15 );
	tst = 64+16+14;
	buf2[64+16+15] = 0xAB;
	while (tst>=0)
	{
		if (buf[tst+1]!=buf2[tst])
		{
			triLogPrint("Error with memcpy_vfpu at position: %i (%x, %x): %i != %i\n", tst, buf+1, buf2, buf[tst], buf2[tst+1]);
		}
		tst--;
	}
	if (buf2[64+16+15] != 0xAB) triLogPrint("Buffer overrun detected!\n");
	
	free(buf);
	triLogPrint("Finished test.\n\n");
}


void test_memcpy_vfpu_bench( char* buf, long size )
{
	size = 256*1024 + 16;
	triLogPrint("Testing memcpy_vfpu for speed...\n");
	char* buf2 = malloc( size );
	
	long i = 0;
	u64 last_tick, end_tick;
	u32 tick_frequency = sceRtcGetTickResolution();

	triLogPrint("Testing memcpy...\n");
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("QWord aligned memcpy                : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2+1, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Byte unaligned memcpy               : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy( buf2+4, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned memcpy               : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	

	triLogPrint("Testing memcpy_vfpu...\n");
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("QWord aligned memcpy_vfpu           : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2+1, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Byte unaligned memcpy_vfpu          : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2+4, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (dst) memcpy_vfpu    : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2, buf+4, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src) memcpy_vfpu    : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		memcpy_vfpu( buf2+4, buf+4, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src&dst) memcpy_vfpu: %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));



	triLogPrint("Testing sceVfpuMemcpy_vnop...\n");
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		sceVfpuMemcpy_vnop( buf2, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("QWord aligned sceVfpuMemcpy_vnop           : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		sceVfpuMemcpy_vnop( buf2+1, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Byte unaligned sceVfpuMemcpy_vnop          : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));
	
	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		sceVfpuMemcpy_vnop( buf2+4, buf, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (dst) sceVfpuMemcpy_vnop    : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		sceVfpuMemcpy_vnop( buf2, buf+4, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src) sceVfpuMemcpy_vnop    : %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));

	sceRtcGetCurrentTick(&last_tick);
	for (i=0;i<1024;i++)
	{
		sceVfpuMemcpy_vnop( buf2+4, buf+4, size-16 );
	}
	sceRtcGetCurrentTick(&end_tick);
	triLogPrint("Word unaligned (src&dst) sceVfpuMemcpy_vnop: %.2fMB/s\n", (float)((size-16)/1024.0f)/(float)((float)(end_tick-last_tick)/tick_frequency));


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
	stream* f = stream_fopen( "test.dat", STREAM_RDONLY );
	long size = stream_size( f ); //4048+16;
	buf = malloc( size );
	char* buf_test = malloc( size );
	stream_read( f, buf_test, size );
	stream_close( f );
	

	test_fstream( buf, size );
	//test_afstream( buf,size );		// currently creates an infinite loop somewhere!
	test_bufstream( buf, size, buf_test );
	test_mstream( buf, size );

	test_bufstream_copy( "test.dat", "test.out", buf, size );
	
	long i = 0;
	for (;i<size;i++)
		buf[i] = (i&0xFF);
	
	test_memcpy_vfpu( buf, size );
	test_memcpy_vfpu_bench( buf, size );
	
	free( buf );
	triLogPrint("Finished all tests.\n\n");
	sceKernelExitGame();
	return 0;
}
