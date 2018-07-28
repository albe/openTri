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


#ifndef pmp_file_h
#define pmp_file_h


#include <stdio.h>
#include "mem64.h"


struct pmp_signature_struct
	{
	unsigned int pmpm;
	unsigned int version;
	};


struct pmp_video_struct
	{
	unsigned int format;
	unsigned int number_of_frames;
	unsigned int width;
	unsigned int height;
	unsigned int scale;
	unsigned int rate;
	};


struct pmp_audio_struct
	{
	unsigned int format;
	unsigned int number_of_streams;
	unsigned int maximum_number_of_frames;
	unsigned int scale;
	unsigned int rate;
	unsigned int stereo;
	};


struct pmp_header_struct
	{
	struct pmp_signature_struct signature;
	struct pmp_video_struct     video;
	struct pmp_audio_struct     audio;
	};


struct pmp_file_struct
	{
	FILE *f;

	struct pmp_header_struct header;

	unsigned int *packet_index;
	unsigned int  packet_start;
	unsigned int  maximum_packet_size;
	};


void pmp_file_safe_constructor(struct pmp_file_struct *p);
void pmp_file_close(struct pmp_file_struct *p);
char *pmp_file_open(struct pmp_file_struct *p, char *s);


#endif
