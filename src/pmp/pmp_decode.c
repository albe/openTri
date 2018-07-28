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


#include "pmp_decode.h"


void pmp_decode_safe_constructor(struct pmp_decode_struct *p)
	{
	pmp_read_safe_constructor(&p->reader);

	avc_safe_constructor(&p->avc);

	p->video_frame_width = 0;
	p->video_frame_height = 0;
	p->video_buffer_width = 0;
	p->video_frame_size = 0;
	p->current_video_frame = 0;
	p->video_decode_format = -1;
	
	int i = 0;
	for (; i < maximum_frame_buffers; i++)
		{
		p->video_frame_buffers[i] = 0;
		p->audio_frame_buffers[i] = 0;
		}
	}


void pmp_decode_close(struct pmp_decode_struct *p)
	{
	pmp_read_close(&p->reader);
	
	avc_close(&p->avc);

	if (p->audio_decoder == 0 )
		audio_decoder_close();

	int i = 0;
	for (; i < maximum_frame_buffers; i++)
		{
		if (p->video_frame_buffers[i] != 0) free_64(p->video_frame_buffers[i]);
		if (p->audio_frame_buffers[i] != 0) free_64(p->audio_frame_buffers[i]);
		}


	pmp_decode_safe_constructor(p);
	}



// Return next power of 2
static unsigned int next_pow2(unsigned int w)
{
	w -= 1;		// To not change already power of 2 values
	w |= (w >> 1);
	w |= (w >> 2);
	w |= (w >> 4);
	w |= (w >> 8);
	w |= (w >> 16);
	return(w+1);
}


char *pmp_decode_open(struct pmp_decode_struct *p, char *s, int format)
	{
	pmp_decode_safe_constructor(p);

	printf(">pmp_decode_open( %p, \"%s\", %i )\n", p, s, format);

	char *result = pmp_read_open(&p->reader, FF_INPUT_BUFFER_PADDING_SIZE, s);
	if (result != 0)
		{
		pmp_decode_close(p);
		return(result);
		}

	p->audio_frame_size = (p->reader.file.header.audio.scale << 1) << p->reader.file.header.audio.stereo;
	
	if (format==-1) format=3;
	p->video_frame_width = p->reader.file.header.video.width;
	p->video_frame_height = p->reader.file.header.video.height;
	p->video_buffer_width = next_pow2(p->reader.file.header.video.width);
	p->video_frame_size = p->video_buffer_width*p->video_frame_height*((format==3)?4:2);

	result = avc_open(&p->avc, p->reader.file.maximum_packet_size, p->video_buffer_width, format);
	if (result != 0)
		{
		pmp_decode_close(p);
		return(result);
		}
	p->video_decode_format = p->avc.mpeg_format;
	
	printf(">video width: %i\n>video height: %i\n>frame size: %i\n>format: %i\n", p->video_frame_width, p->video_frame_height, p->video_frame_size, p->video_decode_format);
	
	if ( p->reader.file.header.audio.format == 0 )
		p->audio_decoder = audio_decoder_open(0x1002, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 0);
	else if ( p->reader.file.header.audio.format == 1 )
		p->audio_decoder = audio_decoder_open(0x1003, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 0);
	else if ( p->reader.file.header.audio.format == 2 )
		p->audio_decoder = audio_decoder_open(0x1001, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 384);
	else if (p->reader.file.header.audio.format == 21)
		p->audio_decoder = audio_decoder_open(0x1001, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 304);
	else if (p->reader.file.header.audio.format == 22)
		p->audio_decoder = audio_decoder_open(0x1001, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 192);
	else
		p->audio_decoder = -1;
	
	if (p->audio_decoder == -1)
		{
		pmp_decode_close(p);
		return("pmp_decode_open: audio_decoder_open failed");
		}


	p->number_of_frame_buffers = 0;

	int i = 0;
	for (; i < maximum_frame_buffers; i++)
		{
		p->video_frame_buffers[i] = malloc_64( p->video_frame_size );

		if (p->video_frame_buffers[i] == 0)
			{
			break;
			}

		memset(p->video_frame_buffers[i], 0, p->video_frame_size);

		p->number_of_frame_buffers ++;
		}


	if (p->number_of_frame_buffers < (number_of_free_video_frame_buffers + 4))
		{
		pmp_decode_close(p);
		return("pmp_decode_open: number_of_frame_buffers < 4");
		}


	p->number_of_frame_buffers -= number_of_free_video_frame_buffers;

	i = 0;
	for (; i < number_of_free_video_frame_buffers; i++)
		{
		free_64(p->video_frame_buffers[p->number_of_frame_buffers + i]);

		p->video_frame_buffers[p->number_of_frame_buffers + i] = 0;
		}


	i = 0;
	for (; i <= p->number_of_frame_buffers; i++)
		{
		// p->audio_frame_buffers[p->number_of_frame_buffers] is a null buffer

		p->audio_frame_buffers[i] = malloc_64(p->audio_frame_size * p->reader.file.header.audio.maximum_number_of_frames);

		if (p->audio_frame_buffers[i] == 0)
			{
			pmp_decode_close(p);
			return("pmp_decode_open: malloc_64 failed on audio_frame_buffers");
			}

		memset(p->audio_frame_buffers[i], 0, p->audio_frame_size * p->reader.file.header.audio.maximum_number_of_frames);
		}


	p->current_buffer_number = 0;
	p->current_video_frame = p->video_frame_buffers[0];
	
	printf(">pmp_decode_open done.\n");
	return(0);
	}


static void boost_volume(short *audio_buffer, unsigned int number_of_samples, unsigned int volume_boost)
	{
	if (volume_boost != 0)
		{
		while (number_of_samples--)
			{
			int sample = *audio_buffer;
			sample <<= volume_boost;


			if (sample > 32767)
				{
				*audio_buffer++ = 32767;
				}
			else if (sample < -32768)
				{
				*audio_buffer++ = -32768;
				}
			else
				{
				*audio_buffer++ = sample;
				}
			}
		}
	}


char *pmp_decode_get(struct pmp_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int decode_audio, unsigned int volume_boost/*, unsigned int show_subtitle, unsigned int subtitle_format*/)
	{
	struct pmp_read_output_struct packet;


	char *result = pmp_read_get(&p->reader, frame_number, audio_stream, &packet);
	if (result != 0)
		{
		return(result);
		}


	sceKernelDcacheWritebackInvalidateAll();
	void* frame_buffer = p->video_frame_buffers[p->current_buffer_number];
	if (p->video_buffer_width==512 && p->video_frame_width<=480 && p->video_frame_height<=272)
	{
		// Center screen
		frame_buffer += (((272-p->video_frame_height)>>1)*p->video_buffer_width + ((480-p->video_frame_width)>>1))*(p->video_decode_format==3?4:2);
	}

	result = avc_get(&p->avc, packet.video_buffer, packet.video_length, frame_buffer);
	if (result != 0)
		{
		printf("pmp_decode_get: avc_get failed!\n");
		return(result);
		}


	p->output_frame_buffers[p->current_buffer_number].number_of_audio_frames = packet.number_of_audio_frames;
	p->output_frame_buffers[p->current_buffer_number].first_delay            = packet.first_delay;
	p->output_frame_buffers[p->current_buffer_number].last_delay             = packet.last_delay;
	p->output_frame_buffers[p->current_buffer_number].video_frame            = p->video_frame_buffers[p->current_buffer_number];



	char *audio_result = 0;


	if (packet.audio_buffer == 0 || decode_audio == 0)
		{
		p->output_frame_buffers[p->current_buffer_number].audio_frame = p->audio_frame_buffers[p->number_of_frame_buffers];
		}
	else
		{
		p->output_frame_buffers[p->current_buffer_number].audio_frame = p->audio_frame_buffers[p->current_buffer_number];


		void *audio_buffer = packet.audio_buffer;


		int i = 0;
		for (; i < packet.number_of_audio_frames; i++)
			{
			int audio_length = packet.audio_length[i];
			int audio_output_length;
			
			if (audio_decoder_decode(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, &audio_output_length, audio_buffer, audio_length) != audio_length)
				{
				audio_result = "pmp_decode_get: audio_decoder_decode failed";
				break;
				}

			audio_buffer += audio_length;


			if (audio_output_length != p->audio_frame_size)
				{
				if (audio_output_length == 0)
					{
					}
				else if (audio_output_length < p->audio_frame_size)
					{
					audio_result = "pmp_decode_get: audio_output_length < audio_frame_size";
					break;
					}
				else if (audio_output_length > p->audio_frame_size)
					{
					audio_result = "pmp_decode_get: audio_output_length > audio_frame_size (severe error)";
					break;
					}
				}
			else
				{
				boost_volume(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, p->reader.file.header.audio.scale << p->reader.file.header.audio.stereo, volume_boost);
				}
			}
		}


	p->current_buffer_number = (p->current_buffer_number + 1) % p->number_of_frame_buffers;
	sceKernelDcacheWritebackInvalidateAll();


	return(audio_result);
	}
