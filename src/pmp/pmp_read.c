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


#include "pmp_read.h"


void pmp_read_safe_constructor(struct pmp_read_struct *p)
	{
	pmp_file_safe_constructor(&p->file);

	p->f             = -1;
	p->buffer_0      = 0;
	p->buffer_1      = 0;
	p->packet_header = 0;
	}


void pmp_read_close(struct pmp_read_struct *p)
	{
    pmp_file_close(&p->file);

    if (!(p->f < 0))           sceIoClose(p->f);
    if (p->buffer_0      != 0) free_64(p->buffer_0);
    if (p->buffer_1      != 0) free_64(p->buffer_1);
    if (p->packet_header != 0) free_64(p->packet_header);


	pmp_read_safe_constructor(p);
	}


static char *fill_asynchronous_buffer(struct pmp_read_struct *reader, struct asynchronous_buffer *p, unsigned int first_packet, unsigned int first_packet_position, unsigned int seek)
	{
	p->first_packet     = first_packet;
	p->last_packet      = first_packet;
	p->packets_size     = reader->file.packet_index[first_packet] >> 1;
	p->packet_buffer[0] = p->buffer + (first_packet_position & (64 - 1));


	unsigned int number_of_packets = 1;


	if (seek == 0)
		{
		while (1)
			{
			if (p->last_packet + 1 == reader->file.header.video.number_of_frames)
				{
				break;
				}


			if (number_of_packets == maximum_number_of_packets)
				{
				break;
				}


			unsigned int next_packet_size = reader->file.packet_index[p->last_packet + 1] >> 1;

			if (p->packets_size + next_packet_size > reader->buffer_size)
				{
				break;
				}


			p->packet_buffer[number_of_packets]  = p->packet_buffer[0] + p->packets_size;
			p->packets_size                     += next_packet_size;
			p->last_packet                      ++;
			number_of_packets                   ++;
			}
		}


	p->first_packet_position = first_packet_position;
	p->next_packet_position  = first_packet_position + p->packets_size;




	if (sceIoLseek32(reader->f, p->first_packet_position, PSP_SEEK_SET) != p->first_packet_position)
		{
		return("fill_asynchronous_buffer: seek failed");
		}


	if (sceIoReadAsync(reader->f, p->packet_buffer[0], p->packets_size) < 0)
		{
		return("fill_asynchronous_buffer: read failed");
		}




	return(0);
	}


static char *wait_asynchronous_buffer(struct pmp_read_struct *reader, struct asynchronous_buffer *p)
	{
	long long result;

	if (sceIoWaitAsync(reader->f, &result) < 0)
		{
		return("wait_asynchronous_buffer: wait failed");
		}


	if (p->packets_size != result)
		{
		return("wait_asynchronous_buffer: read failed");
		}


	return(0);
	}


static char *fill_and_wait_asynchronous_buffer(struct pmp_read_struct *reader, struct asynchronous_buffer *p, unsigned int first_packet, unsigned int first_packet_position, unsigned int seek)
	{
	char *result = fill_asynchronous_buffer(reader, p, first_packet, first_packet_position, seek);
	if (result != 0)
		{
		return(result);
		}


	result = wait_asynchronous_buffer(reader, p);
	if (result != 0)
		{
		return(result);
		}


	return(0);
	}


static char *fill_next_asynchronous_buffer(struct pmp_read_struct *reader, unsigned int seek)
	{
	unsigned int first_packet          = reader->current_asynchronous_buffer->last_packet + 1;
	unsigned int first_packet_position = reader->current_asynchronous_buffer->next_packet_position;


	if (first_packet == reader->file.header.video.number_of_frames)
		{
		first_packet          = 0;
		first_packet_position = reader->file.packet_start;
		}


	char *result = fill_asynchronous_buffer(reader, reader->next_asynchronous_buffer, first_packet, first_packet_position, seek);
	if (result != 0)
		{
		return(result);
		}


	return(0);
	}


static char *fill_current_and_next_asynchronous_buffer(struct pmp_read_struct *reader, unsigned int first_packet, unsigned int first_packet_position, unsigned int seek)
	{
	char *result = fill_and_wait_asynchronous_buffer(reader, reader->current_asynchronous_buffer, first_packet, first_packet_position, seek);
	if (result != 0)
		{
		return(result);
		}


	result = fill_next_asynchronous_buffer(reader, seek);
	if (result != 0)
		{
		return(result);
		}


	return(0);
	}


static void swap_asynchronous_buffers(struct pmp_read_struct *reader)
	{
	struct asynchronous_buffer *swap    = reader->current_asynchronous_buffer;
	reader->current_asynchronous_buffer = reader->next_asynchronous_buffer;
	reader->next_asynchronous_buffer    = swap;
	}


static unsigned int get_packet_position(struct pmp_read_struct *reader, unsigned int packet)
	{
	unsigned int first_packet          = reader->current_asynchronous_buffer->first_packet;
	unsigned int first_packet_position = reader->current_asynchronous_buffer->first_packet_position;


	if (packet >= first_packet)
		{
		while (first_packet != packet)
			{
			first_packet_position += reader->file.packet_index[first_packet] >> 1;
			first_packet          ++;
			}
		}
	else
		{
		while (first_packet != packet)
			{
			first_packet          --;
			first_packet_position -= reader->file.packet_index[first_packet] >> 1;
			}
		}


	return(first_packet_position);
	}


char *pmp_read_open(struct pmp_read_struct *p, unsigned int padding, char *s)
	{
	pmp_read_safe_constructor(p);

	printf(">pmp_read_open( %p, %i, \"%s\" )\n", p, padding, s);


	char *result = pmp_file_open(&p->file, s);
	if (result != 0)
		{
		pmp_read_close(p);
		return(result);
		}


	p->f = sceIoOpen(s, PSP_O_RDONLY, 0777);
	if (p->f < 0)
		{
		pmp_read_close(p);
		return("pmp_read_open: can't open file");
		}


	if (sceIoChangeAsyncPriority(p->f, 0x10) < 0)
		{
		pmp_read_close(p);
		return("pmp_read_open: sceIoChangeAsyncPriority failed");
		}




	p->buffer_size = minimum_buffer_size;
	if (p->file.maximum_packet_size > p->buffer_size)
		{
		p->buffer_size = p->file.maximum_packet_size;
		}


	p->buffer_0 = malloc_64(p->buffer_size + padding + 64);
	if (p->buffer_0 == 0)
		{
		pmp_read_close(p);
		return("pmp_read_open: malloc_64 failed on buffer_0");
		}
	memset(p->buffer_0, 0, p->buffer_size + padding + 64);


	p->buffer_1 = malloc_64(p->buffer_size + padding + 64);
	if (p->buffer_1 == 0)
		{
		pmp_read_close(p);
		return("pmp_read_open: malloc_64 failed on buffer_1");
		}
	memset(p->buffer_1, 0, p->buffer_size + padding + 64);




	p->packet_header = malloc_64(sizeof(unsigned int) * (p->file.header.audio.maximum_number_of_frames * p->file.header.audio.number_of_streams + 3));
	if (p->packet_header == 0)
		{
		pmp_read_close(p);
		return("pmp_read_open: malloc_64 failed on packet_header");
		}




	p->asynchronous_buffer_0.buffer = p->buffer_0;
	p->asynchronous_buffer_1.buffer = p->buffer_1;

	p->current_asynchronous_buffer  = &p->asynchronous_buffer_0;
	p->next_asynchronous_buffer     = &p->asynchronous_buffer_1;


	result = fill_current_and_next_asynchronous_buffer(p, 0, p->file.packet_start, 0);
	if (result != 0)
		{
		pmp_read_close(p);
		return(result);
		}



	printf(">pmp_read_open done.\n");
	return(0);
	}


static char *get_packet(struct pmp_read_struct *reader, unsigned int packet, void **buffer)
	{
	if (packet >= reader->current_asynchronous_buffer->first_packet && packet <= reader->current_asynchronous_buffer->last_packet)
		{
		*buffer = reader->current_asynchronous_buffer->packet_buffer[packet - reader->current_asynchronous_buffer->first_packet];
		}
	else
		{
		char *result = wait_asynchronous_buffer(reader, reader->next_asynchronous_buffer);
		if (result != 0)
			{
			return(result);
			}


		if (packet >= reader->next_asynchronous_buffer->first_packet && packet <= reader->next_asynchronous_buffer->last_packet)
			{
			swap_asynchronous_buffers(reader);

			result = fill_next_asynchronous_buffer(reader, 0);
			if (result != 0)
				{
				return(result);
				}

			*buffer = reader->current_asynchronous_buffer->packet_buffer[packet - reader->current_asynchronous_buffer->first_packet];
			}
		else
			{
			unsigned int packet_position = get_packet_position(reader, packet);

			result = fill_current_and_next_asynchronous_buffer(reader, packet, packet_position, 1);
			if (result != 0)
				{
				return(result);
				}

			*buffer = reader->current_asynchronous_buffer->packet_buffer[0];
			}
		}


	return(0);
	}


char *pmp_read_get(struct pmp_read_struct *p, unsigned int packet, unsigned int audio_stream, struct pmp_read_output_struct *output)
	{
	void *buffer;

	char *result = get_packet(p, packet, &buffer);
	if (result != 0)
		{
		return(result);
		}


	unsigned char number_of_audio_frames = *((unsigned char *) buffer);
	output->number_of_audio_frames       = number_of_audio_frames;


	unsigned int size_of_unsigned_int_fields = sizeof(unsigned int) * (output->number_of_audio_frames * p->file.header.audio.number_of_streams + 3);

	memcpy(p->packet_header, buffer + 1, size_of_unsigned_int_fields);


	output->first_delay  = p->packet_header[0];
	output->last_delay   = p->packet_header[1];
	output->video_length = p->packet_header[2];
	output->audio_length = &p->packet_header[3];
	output->video_buffer = buffer + size_of_unsigned_int_fields + 1;
	output->audio_buffer = output->video_buffer + output->video_length;


	unsigned int skip = audio_stream * output->number_of_audio_frames;

	while (skip != 0)
		{
		output->audio_buffer += *output->audio_length;
		output->audio_length ++;
		skip                 --;
		}


	unsigned int i = 0;

	for (; i < output->number_of_audio_frames; i++)
		{
		if (output->audio_length[i] == 0)
			{
			output->audio_buffer = 0;
			}
		}


	return(0);
	}
