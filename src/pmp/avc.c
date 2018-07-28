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


#include "avc.h"
#include <string.h>
#include <pspsdk.h>
#include <pspmpeg.h>
#include <psputility_avmodules.h>
#include "mem64.h"


char *avc_static_init()
	{
	sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	sceUtilityLoadAvModule(PSP_AV_MODULE_MPEGBASE);
	return(0);
	}


void avc_safe_constructor(struct avc_struct *p)
	{
	p->mpeg_init = -1;

	p->mpeg_data = 0;

	p->mpeg_ringbuffer_construct = -1;

	p->mpeg_create = -1;

	p->mpeg_es = 0;
	
	p->mpeg_lli = 0;
	
	p->mpeg_format = -1;
	}


void avc_close(struct avc_struct *p)
	{
	if (p->mpeg_lli != 0) free_64(p->mpeg_lli);

	if (p->mpeg_es != 0) sceMpegFreeAvcEsBuf(&p->mpeg, p->mpeg_es);

	if (!(p->mpeg_create != 0)) sceMpegDelete(&p->mpeg);

	if (!(p->mpeg_ringbuffer_construct != 0)) sceMpegRingbufferDestruct(&p->mpeg_ringbuffer);

	if (!(p->mpeg_init != 0)) sceMpegFinish();

	if (p->mpeg_data != 0) free_64(p->mpeg_data);


	avc_safe_constructor(p);
	}


char *avc_open(struct avc_struct *p, unsigned int maximum_frame_size, int bufwidth, int format)
	{
	avc_safe_constructor(p);

	p->mpeg_format = format;
	p->mpeg_width = bufwidth;
	
	p->mpeg_init = sceMpegInit();
	if (p->mpeg_init != 0)
		{
		avc_close(p);
		return("avc_open: sceMpegInit failed");
		}


	int size = sceMpegQueryMemSize(0);
	if (size < 0)
		{
		avc_close(p);
		return("avc_open: sceMpegQueryMemSize failed");
		}


	p->mpeg_data = malloc_64(size);
	if (p->mpeg_data == 0)
		{
		avc_close(p);
		return("avc_open: malloc_64 failed on mpeg_data");
		}


	p->mpeg_ringbuffer_construct = sceMpegRingbufferConstruct(&p->mpeg_ringbuffer, 0, 0, 0, 0, 0);
	if (p->mpeg_ringbuffer_construct != 0)
		{
		avc_close(p);
		return("avc_open: sceMpegRingbufferConstruct failed");
		}


	p->mpeg_create = sceMpegCreate(&p->mpeg, p->mpeg_data, size, &p->mpeg_ringbuffer, p->mpeg_width, 0, 0);
	if (p->mpeg_create != 0)
		{
		avc_close(p);
		return("avc_open: sceMpegCreate failed");
		}


	SceMpegAvcMode avc_mode;
	avc_mode.iUnk0 = -1;
	avc_mode.iPixelFormat = p->mpeg_format;
	if (sceMpegAvcDecodeMode(&p->mpeg, &avc_mode) != 0)
		{
		avc_close(p);
		return("avc_open: sceMpegAvcDecodeMode failed");
		}
	p->mpeg_format = avc_mode.iPixelFormat;
	

	p->mpeg_es = sceMpegMallocAvcEsBuf(&p->mpeg);
	if (p->mpeg_es == 0)
		{
		avc_close(p);
		return("avc_open: sceMpegMallocAvcEsBuf failed");
		}
		


	unsigned int maximum_number_of_blocks = (maximum_frame_size + DMABLOCK - 1) / DMABLOCK;

	p->mpeg_lli = malloc_64(sizeof(struct SceMpegLLI) * maximum_number_of_blocks);
	if (p->mpeg_lli == 0)
		{
		avc_close(p);
		return("avc_open: malloc_64 failed on mpeg_lli");
		}


	memset(&p->mpeg_au, -1, sizeof(SceMpegAu));
	p->mpeg_au.iEsBuffer = 1;


	return(0);
	}


static void CopyAu2Me(struct avc_struct *p, void *source_buffer, int size)
	{
	void *destination_buffer = (void *) MEAVCBUF;

	unsigned int i = 0;

	while (1)
		{
		p->mpeg_lli[i].pSrc = source_buffer;
		p->mpeg_lli[i].pDst = destination_buffer;

		if (size > DMABLOCK)
			{
			p->mpeg_lli[i].iSize = DMABLOCK;
			p->mpeg_lli[i].Next  = &p->mpeg_lli[i + 1];

			source_buffer      += DMABLOCK;
			destination_buffer += DMABLOCK;
			size               -= DMABLOCK;
			i                  ++;
			}
		else
			{
			p->mpeg_lli[i].iSize = size;
			p->mpeg_lli[i].Next  = 0;

			break;
			}
		}
		
	sceMpegbase_BEA18F91(p->mpeg_lli);
	}


char *avc_get(struct avc_struct *p, void *source_buffer, int size, void *destination_buffer)
	{
	CopyAu2Me(p, source_buffer, size);

	p->mpeg_au.iAuSize = size;

	int unused;

	if (sceMpegAvcDecode(&p->mpeg, &p->mpeg_au, p->mpeg_width, &destination_buffer, &unused) != 0)
		{
		return("avc_get: sceMpegAvcDecode failed");
		}


	return(0);
	}
