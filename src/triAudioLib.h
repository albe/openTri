/*
 * triAudioLib.h: Header for audio library (based on pspaudiolib)
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

#ifndef __TRIAUDIOLIB_H__
#define __TRIAUDIOLIB_H__

#include "triTypes.h"

/** @defgroup triAudioLib Audio Library
 *  @{
 */

#define TRI_NUM_AUDIO_CHANNELS	4
#define TRI_NUM_AUDIO_SAMPLES	1024
#define TRI_VOLUME_MAX		0x8000

typedef triVoid (* triAudioLibCallback)(triVoid *buf, triUInt reqn, triVoid *pdata);

typedef struct
{
	triSInt threadHandle;
        triSInt handle;
        triSInt volumeLeft;
        triSInt volumeRight;
        triAudioLibCallback callback;
        triVoid *data;
} triAudioLibChannelInfo;

/**
 * Initialise the Audio Library
 *
 * @param priority - Priority to use for the audio threads.
 *
 * @returns true on success.
 */
triBool triAudioLibInit(triSInt priority); // 0x12

/**
 * Shutdown the Audio Library
 */
triVoid triAudioLibShutdown();

/**
 * Set channel volume
 *
 * @param channel - The audio channel.
 *
 * @param left - Left volume.
 *
 * @param right - Right volume.
 */
triVoid triAudioLibSetVolume(triSInt channel, triSInt left, triSInt right);

/**
 * Setup a callback
 *
 * @param channel - The audio channel.
 *
 * @param callback - The callback function.
 *
 * @param pdata - The data to pass.
 */
triVoid triAudioLibSetChannelCallback(triSInt channel, triAudioLibCallback callback, triVoid *data);

/**
 * Panned Output
 *
 * @param channel - The audio channel.
 *
 * @param left - Left volume.
 *
 * @param right - Right volume.
 */
triSInt triAudioLibOutBlocking(triUInt channel, triUInt left, triUInt right, triVoid *data);

/** @} */

#endif // __TRIAUDIOLIB_H__
