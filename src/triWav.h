/*
 * triWav.h: Header for WAV playback
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

#ifndef __TRIWAV_H__
#define __TRIWAV_H__

#include "triTypes.h"

/** @defgroup triWav WAV
 *  @{
 */

/**
 * A WAV file struct
 */
typedef struct
{
        triU32 channels; /**<  Number of channels */
        triU32 sampleRate; /**<  Sample rate */
        triU32 sampleCount; /**<  Sample count */
        triU32 dataLength; /**<  Data length */
        triU32 rateRatio; /**<  Rate ratio (sampleRate / 44100 * 0x10000) */
        triU32 playPtr; /**<  Internal */
        triU32 playPtr_frac; /**<  Internal */
        triUInt loop; /**<  Loop flag */
        void *data; /**< A pointer to the actual WAV data */
        triUInt id; /**<  The ID of the WAV */
        triU32 bitPerSample; /**<  The bit rate of the WAV */
} triWav;

/**
 * Initialise the WAV playback
 *
 * @returns true on success.
 */
triBool triWavInit();

/**
 * Load a WAV file
 *
 * @param filename - Path of the file to load.
 *
 * @returns A pointer to a ::triWAV struct or NULL on error.
 */
triWav *triWavLoad(const triChar *filename);

/**
 * Unload a previously loaded WAV file
 */
triVoid triWavFree(triWav *theWav);

/**
 * Start playing a loaded WAV file
 *
 * @param theWav A pointer to a valid ::triWav struct.
 *
 * @returns < 0 on error, or 0 if no error.
 */
triBool triWavPlay(triWav *theWav);

/**
 * Stop playing a loaded WAV
 */
triVoid triWavStop(triWav *theWav);

/**
 * Stop playing all WAVs
 */
triVoid triWavStopAll();

/**
 * Set the loop of the WAV playback
 *
 * @param theWav - A pointer to a valid ::triWav struct.
 *
 * @param loop - Set to 1 to loop, 0 to playback once.
 */
triVoid triWavSetLoop(triWav *theWav, triUInt loop);

/** @} */

#endif // __TRIWAV_H__
