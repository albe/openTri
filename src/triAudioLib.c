/*
 * triAudioLib.c: Code for audio library (based on pspaudiolib)
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
 
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <pspaudio.h>

#include "triAudioLib.h"
#include "triTypes.h"

static triSInt triAudioLibReady = 0;
static triS16 triAudioLibSoundBuffer[TRI_NUM_AUDIO_CHANNELS][2][TRI_NUM_AUDIO_SAMPLES][2];

static triAudioLibChannelInfo triAudioLibStatus[TRI_NUM_AUDIO_CHANNELS];

static volatile triSInt triAudioLibTerminate = 0;


triVoid triAudioLibSetVolume(triSInt channel, triSInt left, triSInt right)
{
	triAudioLibStatus[channel].volumeRight = right;
	triAudioLibStatus[channel].volumeLeft  = left;
}


triVoid triAudioLibSetChannelCallback(triSInt channel, triAudioLibCallback callback, triVoid *data)
{
	volatile triAudioLibChannelInfo *pci = &triAudioLibStatus[channel];
	
	if (callback == 0)
		pci->callback = 0;
	else
	{
		pci->callback = callback;
		sceKernelWakeupThread(pci->threadHandle);
	}
}


triSInt triAudioLibOutBlocking(triUInt channel, triUInt left, triUInt right, triVoid *data)
{
	if (!triAudioLibReady) return(-1);
	if (channel >= TRI_NUM_AUDIO_CHANNELS) return(-1);
	if (left > TRI_VOLUME_MAX) left = TRI_VOLUME_MAX;
	if (right > TRI_VOLUME_MAX) right = TRI_VOLUME_MAX;

	return sceAudioOutputPannedBlocking(triAudioLibStatus[channel].handle, left, right, data);
}


static triSInt triAudioLibChannelThread(triSInt args, triVoid *argp)
{
	volatile triSInt bufidx = 0;
	triSInt channel = *(triSInt *) argp;
	
	while (triAudioLibTerminate == 0)
	{
		triVoid *bufptr = &triAudioLibSoundBuffer[channel][bufidx];
		triAudioLibCallback callback;
		callback = triAudioLibStatus[channel].callback;

		if (callback)
		{
			callback(bufptr, TRI_NUM_AUDIO_SAMPLES, triAudioLibStatus[channel].data);
			triAudioLibOutBlocking(channel, triAudioLibStatus[channel].volumeLeft, triAudioLibStatus[channel].volumeRight, bufptr);
		} 
		else
		{
			sceKernelSleepThread();
		}

		bufidx = (bufidx ? 0:1);
	}

	sceKernelExitThread(0);

	return(0);
}

triBool triAudioLibInit(triSInt priority)
{
	triSInt i, ret;
	triSInt failed = 0;
	triChar str[32];

	triAudioLibTerminate = 0;
	triAudioLibReady = 0;
	
	for (i = 0; i < TRI_NUM_AUDIO_CHANNELS; i++)
	{
		triAudioLibStatus[i].handle = -1;
		triAudioLibStatus[i].threadHandle = -1;
		triAudioLibStatus[i].volumeRight = TRI_VOLUME_MAX;
		triAudioLibStatus[i].volumeLeft  = TRI_VOLUME_MAX;
		triAudioLibStatus[i].callback = 0;
		triAudioLibStatus[i].data = 0;
	}

	for (i = 0; i < TRI_NUM_AUDIO_CHANNELS; i++)
	{
		if ((triAudioLibStatus[i].handle = sceAudioChReserve(-1, TRI_NUM_AUDIO_SAMPLES, 0)) < 0)
		failed = 1;
	}
	
	if (failed)
	{
		for (i = 0; i < TRI_NUM_AUDIO_CHANNELS; i++)
		{
			if (triAudioLibStatus[i].handle != -1)
				sceAudioChRelease(triAudioLibStatus[i].handle);

			triAudioLibStatus[i].handle = -1;
		}

		return(0);
	}
	
	triAudioLibReady = 1;

	strcpy(str, "TriAudio0");
	
	for (i = 0; i < TRI_NUM_AUDIO_CHANNELS; i++)
	{
		str[8] = '0' + i;
		triAudioLibStatus[i].threadHandle = sceKernelCreateThread(str, (triVoid*)&triAudioLibChannelThread, priority, 0x10000, PSP_THREAD_ATTR_USER, NULL);
		
		if (triAudioLibStatus[i].threadHandle < 0)
		{
			triAudioLibStatus[i].threadHandle = -1;
			failed = 1;
			break;
		}

		ret = sceKernelStartThread(triAudioLibStatus[i].threadHandle, sizeof(i), &i);

		if (ret != 0)
		{
			failed = 1;
			break;
		}
	}

	if (failed)
	{
		triAudioLibTerminate = 1;

		for (i = 0; i < TRI_NUM_AUDIO_CHANNELS; i++)
		{
			if (triAudioLibStatus[i].threadHandle != -1)
			{
				sceKernelDeleteThread(triAudioLibStatus[i].threadHandle);
			}

			triAudioLibStatus[i].threadHandle = -1;
		}

		triAudioLibReady = 0;
		return(1);
	}
	return(1);
}

triVoid triAudioLibShutdown()
{
	triSInt i;
	triAudioLibReady = 0;
	triAudioLibTerminate = 1;

	for (i = 0; i < TRI_NUM_AUDIO_CHANNELS; i++)
	{
		if (triAudioLibStatus[i].threadHandle != -1)
		{
			sceKernelWaitThreadEnd(triAudioLibStatus[i].threadHandle,0);
			sceKernelDeleteThread(triAudioLibStatus[i].threadHandle);
		}

		triAudioLibStatus[i].threadHandle = -1;
	}

	for (i = 0; i < TRI_NUM_AUDIO_CHANNELS; i++)
	{
		if (triAudioLibStatus[i].handle != -1)
		{
			sceAudioChRelease(triAudioLibStatus[i].handle);
			triAudioLibStatus[i].handle = -1;
		}
	}
}
