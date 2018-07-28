/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
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
.pmp header reading routines
*/


#include "pmp_file.h"


void pmp_file_safe_constructor(struct pmp_file_struct *p)
	{
	p->f            = 0;
	p->packet_index = 0;
	}


void pmp_file_close(struct pmp_file_struct *p)
	{
	if (p->f            != 0) fclose(p->f);
	if (p->packet_index != 0) free_64(p->packet_index);


	pmp_file_safe_constructor(p);
	}


char *pmp_file_open(struct pmp_file_struct *p, char *s)
	{
	pmp_file_safe_constructor(p);



	printf(">pmp_file_open( %p, \"%s\" )\n", p, s);
	
	p->f = fopen(s, "rb");
	if (p->f == 0)
		{
		pmp_file_close(p);
		return("pmp_file_open: can't open file");
		}


	if (fread(&p->header, 1, sizeof(struct pmp_header_struct), p->f) != sizeof(struct pmp_header_struct))
		{
		pmp_file_close(p);
		return("pmp_file_open: can't read header");
		}




	if (p->header.signature.pmpm    != 0x6d706d70)     { pmp_file_close(p); return("pmp_file_open: not a valid pmp file"); }
	if (p->header.signature.version != 1)              { pmp_file_close(p); return("pmp_file_open: invalid file version"); }




	if (p->header.video.format           != 1)         { pmp_file_close(p); return("pmp_file_open: invalid video format"); }
	if (p->header.video.number_of_frames == 0)         { pmp_file_close(p); return("pmp_file_open: video.number_of_frames == 0"); }
	if (p->header.video.width            == 0)         { pmp_file_close(p); return("pmp_file_open: video.width == 0"); }
	if (p->header.video.height           == 0)         { pmp_file_close(p); return("pmp_file_open: video.height == 0"); }
	if (p->header.video.scale            == 0)         { pmp_file_close(p); return("pmp_file_open: video.scale == 0"); }
	if (p->header.video.rate             == 0)         { pmp_file_close(p); return("pmp_file_open: video.rate == 0"); }


	if (p->header.video.rate  < p->header.video.scale) { pmp_file_close(p); return("pmp_file_open: video.rate < video.scale"); }
	if (p->header.video.scale > 0xffffff)              { pmp_file_close(p); return("pmp_file_open: video.scale > 0xffffff"); }


	if ((p->header.video.width  % 16) != 0)            { pmp_file_close(p); return("pmp_file_open: (video.width % 16) != 0"); }
	if ((p->header.video.height % 16) != 0)            { pmp_file_close(p); return("pmp_file_open: (video.height % 16) != 0"); }
	if (p->header.video.width         >  480)          { pmp_file_close(p); return("pmp_file_open: video.width > 480"); }
	if (p->header.video.height        >  272)          { pmp_file_close(p); return("pmp_file_open: video.height > 272"); }




	//if (p->header.audio.format                   != 0) { pmp_file_close(p); return("pmp_file_open: invalid audio format"); }
	if (p->header.audio.number_of_streams        == 0) { pmp_file_close(p); return("pmp_file_open: audio.number_of_streams == 0"); }
	if (p->header.audio.maximum_number_of_frames == 0) { pmp_file_close(p); return("pmp_file_open: audio.maximum_number_of_frames == 0"); }


	//if (p->header.audio.scale  != 1152)                { pmp_file_close(p); return("pmp_file_open: audio.scale != 1152"); }
	if ((p->header.audio.rate   != 44100) && (p->header.audio.rate   != 48000))               { pmp_file_close(p); return("pmp_file_open: audio.rate != 44100 or audio.rate != 48000"); }
	if (p->header.audio.stereo != 1)                   { pmp_file_close(p); return("pmp_file_open: audio.stereo != 1"); }




	p->packet_index = malloc_64(sizeof(unsigned int) * p->header.video.number_of_frames);
	if (p->packet_index == 0)
		{
		pmp_file_close(p);
		return("pmp_file_open: malloc_64 failed on packet_index");
		}


	if (fread(p->packet_index, 1, sizeof(unsigned int) * p->header.video.number_of_frames, p->f) != sizeof(unsigned int) * p->header.video.number_of_frames)
		{
		pmp_file_close(p);
		return("pmp_file_open: can't read packet index");
		}




	p->packet_start        = sizeof(struct pmp_header_struct) + sizeof(unsigned int) * p->header.video.number_of_frames;
	p->maximum_packet_size = p->packet_index[0] >> 1;

	unsigned int i           = 0;
	unsigned int packet_size = 0;

	for (; i < p->header.video.number_of_frames; i++)
		{
		if ((p->packet_index[i] >> 1) > p->maximum_packet_size)
			{
			p->maximum_packet_size = p->packet_index[i] >> 1;
			}

		packet_size += p->packet_index[i] >> 1;
		}




	if (fseek(p->f, 0, SEEK_END) != 0)
		{
		pmp_file_close(p);
		return("pmp_file_open: fseek failed on SEEK_END");
		}


	int tot_size = ftell(p->f);
	if (tot_size == -1)
		{
		pmp_file_close(p);
		return("pmp_file_open: ftell failed");
		}


	if (tot_size != p->packet_start + packet_size)
		{
		pmp_file_close(p);
		return("pmp_file_open: invalid tot_size");
		}




	fclose(p->f);
	p->f = 0;


	printf(">pmp_file_open done.\n");
	return(0);
	}
