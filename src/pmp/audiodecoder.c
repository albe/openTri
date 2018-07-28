/* 
 *	Copyright (C) 2006 cooleyes
 *	eyes.cooleyes@gmail.com 
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <stddef.h>
#include <stdio.h>
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspaudiocodec.h>
#include <malloc.h>
#include <string.h>


typedef int (*audio_decoder_decode_func)(void *data, int *data_size, uint8_t *buf, int buf_size);

static unsigned long audio_codec_buffer[65] __attribute__((aligned(64)));
static short audio_mix_buffer[1152 * 2] __attribute__((aligned(64)));
static unsigned char audio_data_buffer[384] __attribute__((aligned(64)));

static int audio_type;
static int audio_samplerate;
static int audio_samplecount;
static int audio_blockalign;
static audio_decoder_decode_func decode_func = 0;

int atrac3_decoder_decode(void *data, int *data_size,
            uint8_t *buf, int buf_size) {
    
    if (buf_size < audio_blockalign) {
       	return buf_size;
    }
    if ( audio_blockalign == 384 )
    	memcpy(audio_data_buffer, buf, 384);
    else if ( audio_blockalign == 304 )
    	memcpy(audio_data_buffer, buf, 304);
    else if ( audio_blockalign == 192 ) {
    	memcpy(audio_data_buffer, buf, 192);
    	memcpy(audio_data_buffer+192, audio_data_buffer, 192);
    }
    else
    	return buf_size;
    	
    audio_codec_buffer[6] = (int)(audio_data_buffer);
    audio_codec_buffer[8] = (int)(audio_mix_buffer);
    
    if ( sceAudiocodecDecode(audio_codec_buffer, 0x1001) < 0 )
    	memset(audio_mix_buffer, 0, 0x1000);
    
    memcpy(data, audio_mix_buffer, 0x1000);
    *data_size = 0x1000;
    
    return buf_size;
}

int mp3_decoder_decode(void *data, int *data_size,
            uint8_t *buf, int buf_size) {
    	
    audio_codec_buffer[6] = (int)(buf);
    audio_codec_buffer[8] = (int)(audio_mix_buffer);
    audio_codec_buffer[7] = audio_codec_buffer[10] = buf_size;
    audio_codec_buffer[9] = 0x1200;
    
    if ( sceAudiocodecDecode(audio_codec_buffer, 0x1002) < 0 )
    	memset(audio_mix_buffer, 0, 0x1200);
    
    memcpy(data, audio_mix_buffer, 0x1200);
    *data_size = 0x1200;
    
    return buf_size;
}

int aac_decoder_decode(void *data, int *data_size,
            uint8_t *buf, int buf_size) {
    	
    audio_codec_buffer[6] = (int)(buf);
    audio_codec_buffer[8] = (int)(audio_mix_buffer);
    audio_codec_buffer[7] = buf_size;
    audio_codec_buffer[9] = 0x1000;
    
    if ( sceAudiocodecDecode(audio_codec_buffer, 0x1003) < 0 )
    	memset(audio_mix_buffer, 0, 0x1000);
    
    memcpy(data, audio_mix_buffer, 0x1000);
    *data_size = 0x1000;
    
    return buf_size;
}

int audio_decoder_open(int type, int samplerate, int samplecount, int blockalign) {
	memset(audio_codec_buffer, 0, sizeof(audio_codec_buffer));
	if ( type == 0x1001 ) {
		audio_codec_buffer[26] = 0x20;
		if ( sceAudiocodecCheckNeedMem(audio_codec_buffer, 0x1001) < 0 ) 
			return -1;
		if ( sceAudiocodecGetEDRAM(audio_codec_buffer, 0x1001) < 0 ) 
			return -1;
		audio_codec_buffer[43] = 0x1001;
		audio_codec_buffer[6] = (int)(audio_data_buffer);
		audio_codec_buffer[8] = (int)(audio_mix_buffer);
		audio_codec_buffer[44] = 2;
		audio_codec_buffer[59] = (int)(audio_data_buffer);
		audio_codec_buffer[60] = 384;
		
		if ( blockalign == 384 ) {
			audio_codec_buffer[10] = 4;
		}
		else if ( blockalign == 304 ) {
			audio_codec_buffer[10] = 6;
		}
		else if ( blockalign == 192 ) {
			audio_codec_buffer[10] = 4;
		}	
		else {
			sceAudiocodecReleaseEDRAM(audio_codec_buffer);
			return -1;
		}
    
		if ( sceAudiocodecInit(audio_codec_buffer, 0x1001) < 0 ) {
			sceAudiocodecReleaseEDRAM(audio_codec_buffer);
			return -1;
		}
		decode_func = atrac3_decoder_decode;
	}
	else if ( type == 0x1002 ) {
		if ( sceAudiocodecCheckNeedMem(audio_codec_buffer, 0x1002) < 0 ) 
			return -1;
		if ( sceAudiocodecGetEDRAM(audio_codec_buffer, 0x1002) < 0 ) 
			return -1;
		if ( sceAudiocodecInit(audio_codec_buffer, 0x1002) < 0 ) {
			sceAudiocodecReleaseEDRAM(audio_codec_buffer);
			return -1;
		}
		decode_func = mp3_decoder_decode;
	}
	else if ( type == 0x1003 ) {
		if ( sceAudiocodecCheckNeedMem(audio_codec_buffer, 0x1003) < 0 ) 
			return -1;
		if ( sceAudiocodecGetEDRAM(audio_codec_buffer, 0x1003) < 0 ) 
			return -1;
		audio_codec_buffer[10] = samplerate;
		if ( sceAudiocodecInit(audio_codec_buffer, 0x1003) < 0 ) {
			sceAudiocodecReleaseEDRAM(audio_codec_buffer);
			return -1;
		}	
		decode_func = aac_decoder_decode;
	}
	else
		return -1;
	audio_type = type;
	audio_samplerate = samplerate;
	audio_samplecount = samplecount;
	audio_blockalign = blockalign;
	return 0;
}

int audio_decoder_close() {
    sceAudiocodecReleaseEDRAM(audio_codec_buffer);
    return 0;
}

int audio_decoder_decode(void *data, int *data_size,
            uint8_t *buf, int buf_size) {
    return decode_func(data, data_size, buf, buf_size);
}

