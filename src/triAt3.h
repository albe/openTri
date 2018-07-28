/*
 * triAt3.h: Header for Atrac3 playback
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

#ifndef __TRIAT3_H__
#define __TRIAT3_H__

#include "triTypes.h"

/** @defgroup triAt3 Atrac3
 *  @{
 */

/**
 * Initialise the atrac3 playback
 *
 * @returns true on success
 *
 * @note Requires kernel mode
 */
triBool triAt3Init();

/**
 * Load an atrac3 file
 *
 * @param filename - Path of the file to load.
 *
 * @returns true on success
 */
triBool triAt3Load(const char *filename);

/**
 * Unload a previously loaded atrac3 file
 */
triVoid triAt3Free();

/**
 * Start playing a loaded atrac3 file
 */
triVoid triAt3Play();

/**
 * Stop playing a loaded atrac3 file
 */
triVoid triAt3Stop();

/**
 * Pause a loaded atrac3 file
 */
triVoid triAt3Pause();

/**
 * Set the volume of the atrac3 playback
 *
 * @param volume - The new volume. Accepted values 0 to 32768
 */
triVoid triAt3SetVol(triSInt volume);

/**
 * Get the volume of the atrac3 playback
 */
triSInt triAt3GetVol();

/**
 * Set the loop of the atrac3 playback
 *
 * @param loop - Set to 1 to loop, 0 to playback once
 */
triVoid triAt3SetLoop(triBool loop);

/**
 * Get the total number of samples played
 *
 * @returns Number of samples
 */
triU64 triAt3SamplesPlayed();

/**
 * A pointer to the current mixing buffer
 *
 * @returns A pointer to the current values in the mixing buffer
 */
triS16 *triAt3GetMixBuffer();

/** @} */

#endif // __TRIAT3_H__
