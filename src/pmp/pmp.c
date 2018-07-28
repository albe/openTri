/*
PMP Mod - mini lib
Copyright (C) 2006 jonny
Copyright (C) 2007 Raphael <raphael@fx-world.org>

Homepage: http://jonny.leffe.dnsalias.com
          http://wordpress.fx-world.org
E-mail:   jonny@leffe.dnsalias.com

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
this lib should be used to play .pmp files
*/


#include <pspsdk.h>
#include "pmp.h"
#include "pmp_play.h"
#include "avc.h"

char* pmp_init()
{
	return avc_static_init();
}

static struct pmp_play_struct p;
char *pmp_play(char *s, int show, int format)
	{
	char *result = pmp_play_open(&p, s, show, format);
	if (result == 0)
		{
		result = pmp_play_start(&p);
		}

		if (result != 0)
			pmp_play_close(&p);
	
	return(result);
	}

void* pmp_get_frame(int* format, int* width, int* height, int* vbw )
{
	*format = p.decoder.video_decode_format;
	*width = p.decoder.video_frame_width;
	*height = p.decoder.video_frame_height;
	*vbw = p.decoder.video_buffer_width;
	return (p.decoder.current_video_frame);
}

void pmp_stop()
{
	p.return_request = 1;
	p.return_result = "pmp_stop()";
	
	pmp_play_waitend(&p);
	//pmp_play_close(&p);
}

int pmp_isplaying()
{
	return (p.playing != 0);
}
