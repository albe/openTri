/*
 * triTimer.h: Header for timers
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

#ifndef __TRITIMER_H__
#define __TRITIMER_H__

#include "triTypes.h"

/** @defgroup triTimer Timer
 *  @{
 */

/**
 * A timer struct
 */
typedef struct
{
	triFloat deltaTime; /**<  The delta time */
	triU64 timeNow; /**<  Current time */
	triU64 timeLastAsk; /**<  Time of last update */
	triU64 totalTime; /**<  Time passed since creation */
	triUInt tickResolution; /**<  Tick resolution */
} triTimer;

/**
 * Create a timer
 *
 * @returns A pointer to a ::triTimer struct.
 */
triTimer* triTimerCreate();

/**
 * Update a timer
 *
 * Should be called once at the start of each iteration of the loop
 *
 * @param timer - A pointer to a valid ::triTimer struct
 */
triVoid triTimerUpdate(triTimer *timer);

/**
 * Get the delta time of a timer
 *
 * @param timer - A pointer to a valid ::triTimer struct
 *
 * @returns The delta time member of the timer
 */
triFloat triTimerGetDeltaTime(triTimer *timer);

/**
 * Get the delta time of a timer without changing its status
 *
 * @param timer - A pointer to a valid ::triTimer struct
 *
 * @returns The delta time member of the timer
 */
triFloat triTimerPeekDeltaTime(triTimer *timer);

/**
 * Free a timer
 *
 * @param timer - A pointer to a valid ::triTimer struct
 */
triVoid triTimerFree(triTimer *timer);

/** @} */

#endif // __TRITIMER_H__
