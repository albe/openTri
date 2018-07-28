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
this play the file (av output and basic functions - pause, seek ... )
*/


#include <pspgu.h>
#include "pmp_play.h"


void pmp_play_safe_constructor(struct pmp_play_struct *p)
{
	p->audio_reserved = -1;

	p->semaphore_can_get   = -1;
	p->semaphore_can_put   = -1;
	p->semaphore_can_show  = -1;
	p->semaphore_show_done = -1;

	p->output_thread = -1;
	p->decode_thread = -1;
	p->show_thread   = -1;

	p->playing = 0;
	p->finished = 0;
	p->return_request = 0;
	p->return_result  = 0;
	pmp_decode_safe_constructor(&p->decoder);
}


void pmp_play_close(struct pmp_play_struct *p)
{
	//sceAudioSetFrequency(44100);
	if (!(p->audio_reserved < 0)) sceAudioChRelease(p->audio_reserved);

	if (!(p->semaphore_can_get   < 0)) sceKernelDeleteSema(p->semaphore_can_get);
	if (!(p->semaphore_can_put   < 0)) sceKernelDeleteSema(p->semaphore_can_put);
	if (!(p->semaphore_can_show  < 0)) sceKernelDeleteSema(p->semaphore_can_show);
	if (!(p->semaphore_show_done < 0)) sceKernelDeleteSema(p->semaphore_show_done);

	if (!(p->output_thread < 0)) sceKernelDeleteThread(p->output_thread);
	if (!(p->decode_thread < 0)) sceKernelDeleteThread(p->decode_thread);
	if (!(p->show_thread   < 0)) sceKernelDeleteThread(p->show_thread);

	pmp_decode_close(&p->decoder);

	pmp_play_safe_constructor(p);
}


static int pmp_wait(volatile struct pmp_play_struct *p, SceUID s, char *e)
{
	SceUInt t = 1000000;
	

	while (1)
	{
		int result = sceKernelWaitSema(s, 1, &t);

		if (result == SCE_KERNEL_ERROR_OK)
		{
			break;
		}
		else if (result == SCE_KERNEL_ERROR_WAIT_TIMEOUT)
		{
			sceKernelDelayThread(1);

			if (p->return_request == 1)
				return(0);
		}
		else
		{
			p->return_result  = e;
			p->return_request = 1;
			return(0);
		}
	}


	return(1);
}



static int pmp_decode_thread(SceSize input_length, void *input)
{
	volatile struct pmp_play_struct *p = *((void **) input);
	
	int current_video_frame = 0;
	printf("starting decode thread.\n");
	printf("number of frames: %i\n", p->decoder.reader.file.header.video.number_of_frames);
	
	while (p->return_request == 0 && current_video_frame != p->decoder.reader.file.header.video.number_of_frames)
	{
		if (pmp_wait(p, p->semaphore_can_put, "pmp_play_start: sceKernelWaitSema failed on semaphore_can_put") == 0)
		{
			break;
		}

		p->decoder.last_buffer_number = p->decoder.current_buffer_number;
		char *result = pmp_decode_get((struct pmp_decode_struct *) &p->decoder, current_video_frame, p->audio_stream, 1, p->volume_boost/*, p->subtitle, p->subtitle_format*/);
		if (result != 0)
		{
			p->return_result  = result;
			p->return_request = 1;
			break;
		}

		/* // Uncomment and run decode_thread only for a speed benchmark
		p->decoder.current_video_frame = p->decoder.output_frame_buffers[p->decoder.last_buffer_number].video_frame;
		if (p->decoder.video_buffer_width==512)
	{
			sceDisplaySetFrameBuf(p->decoder.current_video_frame, 512, p->decoder.video_decode_format, PSP_DISPLAY_SETBUF_NEXTFRAME);
		}*/
		
		
		if (sceKernelSignalSema(p->semaphore_can_get, 1) < 0)
		{
			p->return_result  = "pmp_play_start: sceKernelSignalSema failed on semaphore_can_get";
			p->return_request = 1;
			break;
		}
		
		current_video_frame++;
	}
	
	printf("current frame: %i\n", current_video_frame);
	if (p->return_request == 0)
	{
		p->finished = 1;
		p->return_result = "finished decoding";
	}
	
	printf("exiting decode thread: %s\n", p->return_result);
	return(0);
}


static void pmp_input(volatile struct pmp_play_struct *p, SceCtrlData *previous_controller)
{
	//scePowerTick(0);

	SceCtrlData controller;
	sceCtrlPeekBufferPositive(&controller, 1);
	if (((controller.Buttons & PSP_CTRL_START) == 0) && (previous_controller->Buttons & PSP_CTRL_START))
	{
		p->return_request = 1;
		p->return_result = "exit: manual";
	}

	*previous_controller = controller;
}


static int __attribute__((aligned(16))) dlist[512];

static int pmp_output_thread(SceSize input_length, void *input)
{
	volatile struct pmp_play_struct *p = *((void **) input);
	printf("starting output thread.\n");

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	SceCtrlData previous_controller;
	sceCtrlPeekBufferPositive(&previous_controller, 1);


    unsigned int first_video_frame     = 1;
	unsigned int current_buffer_number = 0;
	void* framebuffer = 0;

	if (p->show && p->decoder.video_buffer_width!=512)
	{
		sceGuInit();
		sceGuStart(GU_DIRECT,dlist);
		sceGuDrawBuffer( p->decoder.video_decode_format, (void*)0, 512 );
		sceGuDispBuffer(512, 512, (void*)(512*272*(p->decoder.video_decode_format==3?4:2)), 512);
	
		sceGuOffset(2048 - (480/2), 2048 - (272/2));
		sceGuViewport(2048, 2048, 480, 272);

		sceGuFinish();
		sceGuSync(0,0);
		printf("Initialized GU to mode %i (%i)\n", p->decoder.video_decode_format, p->decoder.video_buffer_width);
	}
	
	while (p->return_request == 0 && (p->finished == 0 || current_buffer_number != p->decoder.last_buffer_number) )
	{
		volatile struct pmp_decode_buffer_struct *current_buffer = &p->decoder.output_frame_buffers[current_buffer_number];


		if (pmp_wait(p, p->semaphore_can_get, "pmp_output_thread: sceKernelWaitSema failed on semaphore_can_get") == 0)
		{
			break;
		}


		p->decoder.current_video_frame = current_buffer->video_frame;
		if (p->show)
		{
			if (p->decoder.video_buffer_width==512)
			{
				//sceDisplayWaitVblankStart();
				sceDisplaySetFrameBuf(p->decoder.current_video_frame, 512, p->decoder.video_decode_format, PSP_DISPLAY_SETBUF_NEXTFRAME);
			}
			else
			{
				// TODO: Render frame fullscreen
				sceGuStart(GU_DIRECT,dlist);
				sceGuCopyImage( p->decoder.video_decode_format, 0, 0, p->decoder.video_frame_width, p->decoder.video_frame_height, p->decoder.video_buffer_width, p->decoder.current_video_frame,
				                0, 0, 512, (void*)(((unsigned int)framebuffer)+0x4000000 + ((480-p->decoder.video_frame_width) + 512*(272-p->decoder.video_frame_height))*(p->decoder.video_decode_format==3?4:2)) );
			}
		}
		
		current_buffer->first_delay -= 500;
		sceKernelDelayThread(current_buffer->first_delay < 1 ? 1 : current_buffer->first_delay);
		sceAudioOutputBlocking(0, PSP_AUDIO_VOLUME_MAX, current_buffer->audio_frame);

		
		pmp_input(p, &previous_controller);

		int i = 1;
		for (; i < current_buffer->number_of_audio_frames; i++)
		{
			sceAudioOutputBlocking(0, PSP_AUDIO_VOLUME_MAX, current_buffer->audio_frame + p->decoder.audio_frame_size * i);
		}

		current_buffer->last_delay -= 500;
		sceKernelDelayThread(current_buffer->last_delay < 1 ? 1 : current_buffer->last_delay);

		if (p->show && p->decoder.video_buffer_width!=512)
		{
			sceGuTexSync();
			sceGuFinish();
			sceGuSync(0,0);
			//sceDisplayWaitVblankStart();
			sceGuSwapBuffers();
		}


		current_buffer_number = (current_buffer_number + 1) % p->decoder.number_of_frame_buffers;


		if (first_video_frame == 1)
		{
			first_video_frame = 0;
		}
		else
		{
			if (sceKernelSignalSema(p->semaphore_can_put, 1) < 0)
			{
				p->return_result  = "pmp_output_thread: sceKernelSignalSema failed on semaphore_can_put";
				p->return_request = 1;
				break;
			}
		}
	}
	
	p->return_request = 1;
	printf("exiting output thread: %s\n", p->return_result);
	
	sceKernelWaitThreadEnd(p->decode_thread, 0);
	pmp_play_close(p);
	return(0);
}



char *pmp_play_start(volatile struct pmp_play_struct *p)
{
	p->playing = 1;
	sceKernelStartThread(p->decode_thread, 4, &p);	
	sceKernelStartThread(p->output_thread, 4, &p);
	return(0);
}



char *pmp_play_waitend(struct pmp_play_struct *p)
{
	if (p->decode_thread>=0) sceKernelWaitThreadEnd(p->decode_thread, 0);
	if (p->output_thread>=0) sceKernelWaitThreadEnd(p->output_thread, 0);
	return(0);
}


char *pmp_play_open(struct pmp_play_struct *p, char *s, int show, int format)
{
	pmp_play_safe_constructor(p);
	p->subtitle = 0;
	p->subtitle_count = 0;
	p->show = show;

	printf(">pmp_play_open( %p, \"%s\", %i, %i)\n", p, s, show, format);
	char *result = pmp_decode_open(&p->decoder, s, format);
	if (result != 0)
	{
		pmp_play_close(p);
		return(result);
	}

	/*
	//modify by cooleyes 2006/12/11
	//#define video_directory "ms0:/PSP/VIDEO/"
	char video_directory[512];
	char video_filename[512];
	memset(video_directory, 0, 512);
	memset(video_filename, 0, 512);
	char* divchar = strrchr(s, '/');
	if ( divchar == NULL) {
		strncpy(video_directory, "ms0:/PSP/VIDEO/", 512);
		strncpy(video_filename, s, 512);
	}
	else {
		strncpy(video_directory, s, divchar-s+1);
		strncpy(video_filename, divchar+1, 512); 
	}
	if (pmp_sub_parse_search( video_directory, video_filename, p->decoder.reader.file.header.video.rate, p->decoder.reader.file.header.video.scale, &p->subtitle_count)==0) p->subtitle = 1;
	//if (pmp_sub_parse_search( video_directory, s, p->decoder.reader.file.header.video.rate, p->decoder.reader.file.header.video.scale, &p->subtitle_count)==0) p->subtitle = 1;
	//modify end 
	*/
	

	/*if (sceAudioSetFrequency(p->decoder.reader.file.header.audio.rate) < 0)
	{
		pmp_play_close(p);
		return("pmp_play_open: sceAudioSetFrequency failed");
	}*/
	
	p->audio_reserved = sceAudioChReserve(0, p->decoder.reader.file.header.audio.scale, PSP_AUDIO_FORMAT_STEREO);
	if (p->audio_reserved < 0)
	{
		pmp_play_close(p);
		return("pmp_play_open: sceAudioChReserve failed");
	}



	p->semaphore_can_get = sceKernelCreateSema("can_get", 0, 0, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_get < 0)
	{
		pmp_play_close(p);
		return("pmp_play_open: sceKernelCreateSema failed on semaphore_can_get");
	}


	p->semaphore_can_put = sceKernelCreateSema("can_put", 0, p->decoder.number_of_frame_buffers, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_put < 0)
	{
		pmp_play_close(p);
		return("pmp_play_open: sceKernelCreateSema failed on semaphore_can_put");
	}



	p->output_thread = sceKernelCreateThread("output", pmp_output_thread, 0x8, 0x10000, 0, 0);
	if (p->output_thread < 0)
	{
		pmp_play_close(p);
		return("pmp_play_open: sceKernelCreateThread failed on output_thread");
	}

	p->decode_thread = sceKernelCreateThread("decode", pmp_decode_thread, 0x8, 0x10000, 0, 0);
	if (p->decode_thread < 0)
	{
		pmp_play_close(p);
		return("pmp_play_open: sceKernelCreateThread failed on decode_thread");
	}



	p->return_request = 0;
	p->return_result  = 0;

	p->audio_stream     = 0;
	p->volume_boost     = 0;


	printf(">pmp_play_open done.\n");
	return(0);
}
