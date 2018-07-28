/*
PMP Mod
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
av decoding in a ring buffer
*/


#ifndef pmp_decode_h
#define pmp_decode_h


#include <pspkernel.h>
#include <pspdisplay.h>
#include "pmp_read.h"
#include "audiodecoder.h"
#include "mem64.h"
#include "avc.h"

#define FF_INPUT_BUFFER_PADDING_SIZE 8

#define maximum_frame_buffers 16
#define number_of_free_video_frame_buffers 4


struct pmp_decode_buffer_struct
	{
	void *video_frame;
	void *audio_frame;

	unsigned int number_of_audio_frames;

	int first_delay;
	int last_delay;
	};


struct pmp_decode_struct
	{
	struct pmp_read_struct reader;

	struct avc_struct avc;

	int audio_decoder;


	void *video_frame_buffers[maximum_frame_buffers];
	void *audio_frame_buffers[maximum_frame_buffers];


	unsigned int video_decode_format;
	unsigned int video_frame_size;
	unsigned int video_frame_width;
	unsigned int video_frame_height;
	unsigned int video_buffer_width;
	void* current_video_frame;
	
	
	unsigned int audio_frame_size;
	unsigned int number_of_frame_buffers;


	struct pmp_decode_buffer_struct output_frame_buffers[maximum_frame_buffers];

	unsigned int last_buffer_number;
	unsigned int current_buffer_number;
	};


void pmp_decode_safe_constructor(struct pmp_decode_struct *p);
char *pmp_decode_open(struct pmp_decode_struct *p, char *s, int format);
void pmp_decode_close(struct pmp_decode_struct *p);
char *pmp_decode_get(struct pmp_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int decode_audio, unsigned int volume_boost);


#endif
