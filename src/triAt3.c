/*
 * triAt3.c: Code for Atrac3 playback
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

// Includes
#include <pspkernel.h>
#include <pspsdk.h>
#include <string.h>
#include <pspaudio.h>
#include <pspaudiocodec.h>
#include <psputility.h>
#include <stdio.h>
#include <malloc.h>

#include "triAt3.h"
#include "triTypes.h"
#include "triMemory.h"
#include "triLog.h"

// Constants
#define AT3_CURRENT_BUFFER 6
#define AT3_POS_INPUT_BUFFER 7
#define AT3_TEMPORARY_BUFFER 8
#define AT3_INITIAL_BUFFER 59
#define AT3_LENGTH_BUFFER 60

#define AT3_SAMPLES 1024
#define AT3_THREAD_PRIORITY 16 - 2

#define AT3_TYPE_ATRAC3 0x1001

// -------------------------------------------------------
// Global Variables
static SceUID AT3_ThreadID;
static SceUID AT3_Channel;
static triU32 AT3_Codec_Buffer[65] __attribute__((aligned(64)));
static triS16 AT3_Mix_Buffer[AT3_SAMPLES * 2] __attribute__((aligned(64)));
static triU8 AT3_Buffer[384] __attribute__((aligned(64)));
static triSInt AT3_Playing;
static triSInt AT3_Loaded = 0;
static triSInt AT3_Loop = 1;
static triSInt AT3_Datas_Start;
static triSInt AT3_align = 0;
static triUInt AT3_pos = 0;
static triUInt AT3_length = 0;
static triSInt AT3_Volume = PSP_AUDIO_VOLUME_MAX;
static triU64 AT3_Samples_Played;
static triVoid (*AT3_CallBack)(triS16 *Audio_Buffer, triSInt Buffer_Size);
static triUChar *Tune;

triVoid triAt3Play()
{
	AT3_Playing = 1;
}

triVoid triAt3SetLoop(triBool loop)
{
	AT3_Loop = loop;
}

triVoid triAt3SetVol(triSInt volume)
{
	AT3_Volume = volume;
}

triSInt triAt3GetVol()
{
	return AT3_Volume;
}

triU64 pgeAudioAt3SamplesPlayed()
{
	return(AT3_Samples_Played);
}

triS16 *pgeAudioAt3GetMixBuffer()
{
	return((triS16 *) ((triSInt) AT3_Mix_Buffer | 0x40000000));
}

triSInt AT3_Thread(SceSize args, ScePVoid argp)
{
	while(AT3_Loaded)
	{
		if(AT3_Playing)
		{
			if(AT3_CallBack)
				AT3_CallBack((triS16 *) ((triSInt) AT3_Mix_Buffer | 0x40000000), AT3_SAMPLES);

			AT3_Samples_Played += sceAudioOutputBlocking(AT3_Channel, AT3_Volume, (triVoid *) ((triSInt) AT3_Mix_Buffer | 0x40000000));
			sceAudiocodecDecode(AT3_Codec_Buffer, AT3_TYPE_ATRAC3);
			
			AT3_pos += AT3_Codec_Buffer[AT3_POS_INPUT_BUFFER];
			memcpy( (void*)AT3_Buffer, (void*)AT3_pos, AT3_align );
			if (AT3_align==192)
			{
				memcpy( (void*)(AT3_Buffer+192), (void*)AT3_Buffer, 192 );
			}
			//AT3_Codec_Buffer[AT3_POS_INPUT_BUFFER] = AT3_pos;
			
			if(AT3_pos >= ((AT3_length - AT3_Datas_Start) + AT3_Codec_Buffer[AT3_INITIAL_BUFFER]))
			{
				AT3_pos = AT3_Codec_Buffer[AT3_INITIAL_BUFFER] + AT3_Datas_Start;
				if(!AT3_Loop)
					AT3_Playing = 0;
			}
		}
		else
			memset(AT3_Mix_Buffer, 0, AT3_SAMPLES * 2 * 2);
			
		sceKernelDcacheWritebackInvalidateAll();
		sceKernelDelayThread(10);
	}

	return(0);
}

// -------------------------------------------------------
// Initialize the AT3 replay
static triSInt AT3_FileInit(triVoid *File_Mem, triSInt File_Length, triVoid (*Mixing_CallBack)(triS16 *AT3_Audio_Buffer, triSInt AT3_Buffer_Size))
{
	triUInt *dwFile_Mem;
	triUChar *tmpFile_Mem;
	typedef struct
	{
		unsigned short		wFormatTag;         // Format category
		unsigned short		wChannels;          // Number of channels
		unsigned long		dwSamplesPerSec;    // Sampling rate
		unsigned long		dwAvgBytesPerSec;   // For buffer estimation
		unsigned short		wBlockAlign;        // Data block size
		unsigned short		wBitsPerSample;		// Sample size
	} WAVFMTCHUNK;
	WAVFMTCHUNK fmt;
	memset( &fmt, 0, sizeof(WAVFMTCHUNK) );
					
	AT3_Playing = 0;
	AT3_CallBack = Mixing_CallBack;
	
	dwFile_Mem = (triUInt *) File_Mem;
	
	if(*dwFile_Mem++ == 'FFIR')
	{
		dwFile_Mem++;

		if(*dwFile_Mem++ == 'EVAW')
		{
			tmpFile_Mem = (triUChar *) (dwFile_Mem + 2);
			AT3_align = 384;
			while(*dwFile_Mem != 'atad')
			{
				if (*dwFile_Mem++ == ' tmf')
				{
					memcpy( &fmt, (dwFile_Mem+1), sizeof(WAVFMTCHUNK) );
					AT3_align = fmt.wBlockAlign;
					//printf("Fmt:\nFormat: %x\nChannels: %i\nRate: %i\nBPS: %i\nAlign: %i\nBits: %i\n", fmt.wFormatTag, fmt.wChannels, fmt.dwSamplesPerSec, fmt.dwAvgBytesPerSec, fmt.wBlockAlign, fmt.wBitsPerSample );
				}
				
				dwFile_Mem = (triUInt *) (tmpFile_Mem + *dwFile_Mem);
				tmpFile_Mem = (triUChar *) (dwFile_Mem + 2);
			}
			dwFile_Mem++;
			
			AT3_Datas_Start = (triSInt) tmpFile_Mem - (triSInt) File_Mem;
			memset(AT3_Codec_Buffer, 0, sizeof(AT3_Codec_Buffer));
			memset(AT3_Mix_Buffer, 0, AT3_SAMPLES * 2 * 2);
			
			AT3_Codec_Buffer[26] = 0x20;
			AT3_Codec_Buffer[43] = 0x1001;
			memcpy( (void*)AT3_Buffer, (void*)(File_Mem + AT3_Datas_Start), AT3_align );
			if (AT3_align==192)
			{
				memcpy( (void*)(AT3_Buffer+192), (void*)AT3_Buffer, 192 );
			}
			AT3_pos = (triUInt)(File_Mem + AT3_Datas_Start);
			AT3_Codec_Buffer[AT3_CURRENT_BUFFER] = (triSInt)AT3_Buffer; //(File_Mem + AT3_Datas_Start);
			AT3_Codec_Buffer[AT3_TEMPORARY_BUFFER] = (triSInt) AT3_Mix_Buffer;
			
			if (AT3_align==304)
				AT3_Codec_Buffer[10] = 6;
			else
				AT3_Codec_Buffer[10] = 4;
			AT3_Codec_Buffer[44] = 2;
			AT3_length = File_Length;
			AT3_Codec_Buffer[AT3_INITIAL_BUFFER] = (triSInt) File_Mem;
			AT3_Codec_Buffer[AT3_LENGTH_BUFFER] = 384;

			if(sceAudiocodecCheckNeedMem(AT3_Codec_Buffer, AT3_TYPE_ATRAC3) < 0) return(0);
			if(sceAudiocodecGetEDRAM(AT3_Codec_Buffer, AT3_TYPE_ATRAC3) < 0) return(0);
			if(sceAudiocodecInit(AT3_Codec_Buffer, AT3_TYPE_ATRAC3) < 0) return(0);

			AT3_Channel = sceAudioChReserve(0, AT3_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
		
			if(AT3_Channel < 0) return(0);
		
			AT3_ThreadID = sceKernelCreateThread("TriAtrac3", (triVoid *) AT3_Thread, AT3_THREAD_PRIORITY, (1024 * 4), PSP_THREAD_ATTR_USER, NULL);
		
			if(AT3_ThreadID < 0) return(0);
		
			sceKernelStartThread(AT3_ThreadID, 0, NULL);
		
			return(1);
		}
	}
	return(0);
}

triVoid triAt3Pause()
{
	AT3_Playing = 0;
}

triVoid triAt3Stop()
{
	AT3_Playing = 0;
	AT3_Codec_Buffer[AT3_CURRENT_BUFFER] = AT3_Codec_Buffer[AT3_INITIAL_BUFFER] + AT3_Datas_Start;
}

triVoid triAt3Free()
{
	triAt3Stop();
	AT3_Loaded = 0;
	
	sceKernelDelayThread(50000);
	sceAudioChRelease(AT3_Channel);
	sceAudiocodecReleaseEDRAM(AT3_Codec_Buffer);
	sceKernelWaitThreadEnd(AT3_ThreadID, NULL);
	sceKernelTerminateDeleteThread(AT3_ThreadID);
	
	if(Tune)
		triFree(Tune);
}

static triVoid Mixing_CallBack(triS16 *Audio_Buffer, triSInt Buffer_Size)
{

}

triBool triAt3Load(const triChar *filename)
{
	triLogPrint("triAt3: Opening file %s\r\n", filename);
	
	FILE *pFile = fopen(filename , "rb");
	
	if(pFile == NULL)
	{
		triLogError("triAt3: File not found %s\r\n", filename);
		return(0);
	}
	
	triS32 lSize;
	
	fseek(pFile , 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);
	
	Tune = (triUChar*) triMalloc(sizeof(triUChar)*lSize);
	memset(Tune, 0, lSize);
	
	if(!Tune)
	{
		triLogError("triAt3: Malloc of 'Tune' failed\r\n");
		return(0);
	}
	
	fread(Tune, 1, lSize, pFile);
	fclose(pFile);
	
	AT3_Loaded = 1;
	
	if(AT3_FileInit(Tune, lSize, &Mixing_CallBack))
		return(1);
	
	triLogError("triAt3: Error loading file\r\n");

	return(0);
}

triBool triAt3Init()
{	
	int modID = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	
	if(modID < 0)
	{
		triLogError("triAt3: Init unsuccessful\r\n");
		return(0);
	}
		
	triLogPrint("triAt3: Init successful\r\n");
	
	return(1);
}
