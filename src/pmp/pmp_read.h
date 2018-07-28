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
basic layer for reading av frames
*/


#ifndef pmp_read_h
#define pmp_read_h


#include <string.h>
#include <pspiofilemgr.h>
#include "pmp_file.h"
#include "mem64.h"


#define minimum_buffer_size       262144/2
#define maximum_number_of_packets 1024


struct asynchronous_buffer
	{
	unsigned int  first_packet;
	unsigned int  last_packet;
	unsigned int  packets_size;
	unsigned int  first_packet_position;
	unsigned int  next_packet_position;
	void         *buffer;
	void         *packet_buffer[maximum_number_of_packets];
	};


struct pmp_read_struct
	{
	struct pmp_file_struct file;

	SceUID        f;
	unsigned int  buffer_size;
	void         *buffer_0;
	void         *buffer_1;
	
	unsigned int *packet_header;

	struct asynchronous_buffer asynchronous_buffer_0;
	struct asynchronous_buffer asynchronous_buffer_1;

	struct asynchronous_buffer *current_asynchronous_buffer;
	struct asynchronous_buffer *next_asynchronous_buffer;
	};


struct pmp_read_output_struct
	{
	unsigned int number_of_audio_frames;

	int first_delay;
	int last_delay;
	
	unsigned int  video_length;
	unsigned int *audio_length;

	void *video_buffer;
	void *audio_buffer;
	};


void pmp_read_safe_constructor(struct pmp_read_struct *p);
void pmp_read_close(struct pmp_read_struct *p);
char *pmp_read_open(struct pmp_read_struct *p, unsigned int padding, char *s);
char *pmp_read_get(struct pmp_read_struct *p, unsigned int packet, unsigned int audio_stream, struct pmp_read_output_struct *output);


#endif
