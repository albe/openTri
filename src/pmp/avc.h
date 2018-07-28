/*
Decoding AVC using sceMpeg Library
Copyright (c) 2006 by Sorin P. C. <magik@hypermagik.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
i've made only a little rearrangement over the magiK code posted here:
http://forums.ps2dev.org/viewtopic.php?t=5820
*/


#ifndef avc_h
#define avc_h

#include <psptypes.h>
#include <pspmpeg.h>

#define DMABLOCK 4095
#define MEAVCBUF 0x4a000


struct SceMpegLLI
	{
	ScePVoid pSrc;
	ScePVoid pDst;
	ScePVoid Next;
	SceInt32 iSize;
	};


struct avc_struct
	{
	int      mpeg_init;
	ScePVoid mpeg_data;
	int      mpeg_ringbuffer_construct;
	int      mpeg_create;
	int      mpeg_format;
	int      mpeg_width;

	SceMpegRingbuffer  mpeg_ringbuffer;
	SceMpeg            mpeg;
	ScePVoid           mpeg_es;
	struct SceMpegLLI *mpeg_lli;
	SceMpegAu          mpeg_au;
	};


SceInt32 sceMpegbase_BEA18F91(struct SceMpegLLI *p);

#ifdef __cplusplus
extern "C" {
#endif

char *avc_static_init();
void avc_safe_constructor(struct avc_struct *p);
void avc_close(struct avc_struct *p);
char *avc_open(struct avc_struct *p, unsigned int maximum_frame_size, int bufwidth, int format);
char *avc_get(struct avc_struct *p, void *source_buffer, int size, void *destination_buffer);

#ifdef __cplusplus
}
#endif

#endif
