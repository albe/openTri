/*
 * triWav.c: Code for WAV playback
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 InsertWittyName <tias_dp@hotmail.com>
 *
 * $Id: $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <pspaudio.h>
#include <stdio.h>
#include <string.h>

#include "triRefcount.h"
#include "triWav.h"
#include "triTypes.h"
#include "triMemory.h"
#include "triLog.h"
#include "triAudioLib.h"

#define SND_MAXSLOT 16

static triWav wavout_snd_wavinfo[SND_MAXSLOT];
static triSInt wavout_snd_playing[SND_MAXSLOT];
static triSInt wavout_snd_id[SND_MAXSLOT];

static triS16 *samples;
static triU32 req;
static triUInt id = 0;

static triVoid wavout_snd_callback(triVoid *_buf, triUInt _reqn, triVoid *pdata)
{
	triSInt i,slot;
	triWav *wi;
	triU32 ptr, frac;
	triS16 *buf = _buf;
	
	samples = _buf;
	req = _reqn;
	
	for (i = 0; i < _reqn; i++)
	{
		triSInt outr = 0, outl = 0;
		
		for (slot = 0; slot < SND_MAXSLOT; slot++)
		{
			if (!wavout_snd_playing[slot]) continue;
			
			wi = &wavout_snd_wavinfo[slot];
			frac = wi->playPtr_frac + wi->rateRatio;
			wi->playPtr = ptr = wi->playPtr + (frac>>16);
			wi->playPtr_frac = (frac & 0xffff);

			if (ptr >= wi->sampleCount)
			{
				if (wi->loop)
				{
					wi->playPtr = 0;
					wi->playPtr_frac = 0;
					ptr = 0;
				}
				else
				{
					wavout_snd_playing[slot] = 0;
					break;
				}
			}
			
			triS16 *src16 = (triS16 *)wi->data;
			unsigned char *src8 = (unsigned char *)wi->data;

			if (wi->channels == 1)
			{
				if(wi->bitPerSample == 8)
				{
					outl += (src8[ptr] * 256) - 32768;
					outr += (src8[ptr] * 256) - 32768;
				}
				else
				{
					outl += src16[ptr];
					outr += src16[ptr];
				}
			}
			else
			{
				if(wi->bitPerSample == 8)
				{
					outl += (src8[ptr*2] * 256) - 32768;
					outr += (src8[ptr*2+1] * 256) - 32768;
				}
				else
				{
					outl += src16[ptr*2];
					outr += src16[ptr*2+1];
				}
			}
		}
		
		if (outl < -32768)
			outl = -32768;
		else if (outl > 32767)
			outl = 32767;
			
		if (outr < -32768)
			outr = -32768;
		else if (outr > 32767)
			outr = 32767;

		*(buf++) = outl;
		*(buf++) = outr;
	}
}

triBool triWavInit()
{
	triSInt i;

	triAudioLibInit(0x12);

	triAudioLibSetChannelCallback(0, wavout_snd_callback, 0);

	for (i = 0; i < SND_MAXSLOT; i++)
	{
		wavout_snd_playing[i] = 0;
	}

	triLogPrint("triWav: Init successful\r\n");

	return(1);
}

triVoid triWavStop(triWav *theWav)
{
	triSInt i;
	
	for (i = 0; i < SND_MAXSLOT; i++)
	{
		if(theWav->id == wavout_snd_id[i])
			wavout_snd_playing[i] = 0;
	}
}

triVoid triWavStopAll()
{
	triSInt i;
	
	for (i = 0; i < SND_MAXSLOT; i++)
		wavout_snd_playing[i] = 0;
}

triVoid triWavSetLoop(triWav *theWav, triUInt loop)
{
	theWav->loop = loop;
}

triBool triWavPlay(triWav *theWav)
{
	triSInt i;
	triWav *wid;

	for ( i = 0; i < SND_MAXSLOT; i++ ) if ( wavout_snd_playing[i] == 0 ) break;
	
	if ( i == SND_MAXSLOT ) return(0);
	
	wid = &wavout_snd_wavinfo[i];
	wid->channels = theWav->channels;
	wid->sampleRate = theWav->sampleRate;
	wid->sampleCount = theWav->sampleCount;
	wid->dataLength = theWav->dataLength;
	wid->data = theWav->data;
	wid->rateRatio = theWav->rateRatio;
	wid->playPtr = 0;
	wid->playPtr_frac = 0;
	wid->loop = theWav->loop;
	wid->id = theWav->id;
	wavout_snd_playing[i] = 1;
	wavout_snd_id[i] = theWav->id;
	wid->bitPerSample = theWav->bitPerSample;

	return(1);
}


inline triS16 readS16( triChar* file, triU32 addr )
{
	return (triS16)((file[addr]) | (file[addr+1]<<8));
}

inline triU16 readU16( triChar* file, triU32 addr )
{
	return (triU16)((file[addr]) | (file[addr+1]<<8));
}

inline triS32 readS32( triChar* file, triU32 addr )
{
	return (triS32)((file[addr]) | (file[addr+1]<<8) | (file[addr+2]<<16) | (file[addr+3]<<24));
}

inline triU32 readU32( triChar* file, triU32 addr )
{
	return (triU32)((file[addr]) | (file[addr+1]<<8) | (file[addr+2]<<16) | (file[addr+3]<<24));
}

triWav *triWavLoad(const triChar *filename)
{
	triLogPrint("triWav: Loading %s\r\n", filename);
	
	triUInt filelen;
	triU32 channels;
	triU32 samplerate;
	triU32 blocksize;
	triU32 bitpersample;
	triU32 datalength;
	triU32 samplecount;
	triChar *wavfile;
	triWav *theWav;
	
	if ((theWav=triRefcountRetain( filename ))!=0)
		return theWav;
	
	FILE *pFile = fopen(filename , "rb");
	
	if(pFile == NULL)
	{
		triLogError("triWav: File not found %s\r\n", filename);
		return NULL;
	}
	
	triS32 lSize;

	fseek(pFile , 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);
	
	theWav = triMalloc(lSize + sizeof(triWav));
	triRefcountCreate( filename, theWav );
	wavfile = (triChar*)(theWav) + sizeof(triWav);
	
	filelen = fread(wavfile, 1, lSize, pFile);
	
	fclose(pFile);
	
	if (memcmp(wavfile, "RIFF", 4) != 0)
	{
		triLogError("triWav: format error not RIFF\r\n");
		triFree(theWav);
		return NULL;
	}
	
	/*if (memcmp(wavfile + 8, "WAVEfmt \x10\x00\x00\x00\x01\x00", 14) != 0)
	{
		triLogError("triWav: format error no WAVEfmt string\r\n");
		triFree(theWav);
		return NULL;
	}*/
	
	channels = *(triS16 *)(wavfile + 0x16);
	samplerate = *(triS32 *)(wavfile + 0x18);
	blocksize = *(triS16 *)(wavfile + 0x20);
	bitpersample = *(triS16 *)(wavfile + 0x22);

	triSInt i;
	
	for(i = 0; memcmp(wavfile + 0x24 + i, "data", 4) != 0; i++)
	{
		if (i == filelen-0x2c)
		{
			triLogError("triWav: no data chunk found\r\n");
			triFree(theWav);
			return NULL;
		}
	}

	datalength = readU32(wavfile, 0x28+i); //*(triU32 *)(wavfile + 0x28 + i);
	
	if (datalength + 0x2c + i > filelen)
	{
		triLogError("triWav: size in data chunk is invalid (%s:0x%08X)\r\n",filename,0x28+i);
		datalength = filelen - i - 0x2c;
	}
	
	if (channels != 2 && channels != 1)
	{
		triLogError("triWav: not Mono or Stereo sample\r\n");
		triFree(theWav);
		return NULL;
	}
	
	if (samplerate > 100000 || samplerate < 2000)
	{
		triLogError("triWav: sample rate is wrong\r\n");
		triFree(theWav);
		return NULL;
	}
	
	/*if (blocksize != channels*2)
	{
		triLogError("triWav: BLOCKSIZE MISMATCH\r\n");
		triFree(theWav);
		return NULL;
	}*/
	
	/*if (bitpersample != 16 || bitpersample != 8)
	{
		triLogError("triWav: Bits Per Sample Error\r\n");
		triFree(theWav);
		return NULL;
	}*/

	if (channels == 2)
	{
		samplecount = datalength/(bitpersample/4);
	}
	else
	{
		samplecount = datalength/((bitpersample/4)/2);
	}
	
	if (samplecount <= 0)
	{
		triLogError("triWav: no samples\r\n");
		triFree(theWav);
		return NULL;
	}
	
	theWav->channels = channels;
	theWav->sampleRate = samplerate;
	theWav->sampleCount = samplecount;
	theWav->dataLength = datalength;
	theWav->data = wavfile + 0x2c + i;
	theWav->rateRatio = (samplerate*0x4000)/11025;
	theWav->playPtr = 0;
	theWav->playPtr_frac= 0;
	theWav->loop = 0;
	id++;
	theWav->id = id;
	theWav->bitPerSample = bitpersample;

	return theWav;
}

triVoid triWavFree(triWav *theWav)
{
	if (triRefcountRelease( theWav )!=0) return;
	if(theWav != NULL)
		triFree(theWav);
}
