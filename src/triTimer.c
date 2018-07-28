/*
 * triTimer.c: Code for timers
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


#include <time.h>
#include <psptypes.h>
#include <psprtc.h>

#include "triTimer.h"
#include "triTypes.h"
#include "triMemory.h"

triTimer* triTimerCreate()
{
	triTimer* timer = (triTimer*) triMalloc(sizeof(triTimer));
	
	if(!timer)
		return NULL;

        sceRtcGetCurrentTick(&timer->timeLastAsk);
	timer->tickResolution = sceRtcGetTickResolution();
	return timer;
}

triVoid triTimerUpdate(triTimer *timer)
{
	if (timer==0) return;
	sceRtcGetCurrentTick( &timer->timeNow );
	timer->deltaTime = (timer->timeNow - timer->timeLastAsk) / ((float) timer->tickResolution);
	timer->timeLastAsk = timer->timeNow;
}

triFloat triTimerGetDeltaTime(triTimer *timer)
{
	if (timer==0) return 0.f;
	return timer->deltaTime;
}

triFloat triTimerPeekDeltaTime(triTimer *timer)
{
	if (timer==0) return 0.f;
	sceRtcGetCurrentTick( &timer->timeNow );
	return (timer->timeNow - timer->timeLastAsk) / ((float) timer->tickResolution);
}

triVoid triTimerFree(triTimer *timer)
{
	if(timer)
		triFree(timer);
}
