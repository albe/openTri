/*
Copyright (C) 2000-2007 Tomas Jakobsson.

triInput.c
*/

#include <string.h>
#include <pspctrl.h>
#include "triTypes.h"
#include "triLog.h"

typedef struct triInput
{
	SceCtrlData	Input;
	triVec2f	Stick;
	triU32		Pressed;
	triU32		Held;
} triInput;

static triInput	Input;

/*
========
AddInput
========
*/
static void AddInput (const triU32 Button)
{
	if ((Input.Input.Buttons & Button) != 0)	// Button is down
	{
		if ((Input.Held & Button) == 0)			// Held is not TRUE
		{
			Input.Pressed	|= Button;			// so set both Pressed
			Input.Held		|= Button;			// and Held to TRUE
		}
	}
	else										// Button is up
	{
		Input.Pressed	&= ~Button;				// so set Presses
		Input.Held		&= ~Button;				// and Held to FALSE
	}
}

/*
============
triInputInit
============
*/
triBool triInputInit (void)
{
	triLogPrint ("Initializing triInput\r\n");

	memset (&Input, 0, sizeof(triInput));

	sceCtrlSetSamplingCycle (0);

	sceCtrlSetSamplingMode (PSP_CTRL_MODE_ANALOG);

	return TRUE;
}

/*
================
triInputShutdown
================
*/
void triInputShutdown (void)
{
	triLogPrint ("Shutdown triInput\r\n");
}

/*
==============
triInputUpdate
==============
*/
void triInputUpdate (void)
{
	sceCtrlPeekBufferPositive (&Input.Input, 1);

	if (Input.Input.Lx > 103 && Input.Input.Lx < 151)	Input.Stick.x	= 0.0f;
	else												Input.Stick.x	= (Input.Input.Lx - 128.0f) / 128.0f;

	if (Input.Input.Ly > 103 && Input.Input.Ly < 151)	Input.Stick.y	= 0.0f;
	else												Input.Stick.y	= (Input.Input.Ly - 128.0f) / 128.0f;

	AddInput (PSP_CTRL_SELECT);
	AddInput (PSP_CTRL_START);
	AddInput (PSP_CTRL_UP);
	AddInput (PSP_CTRL_RIGHT);
	AddInput (PSP_CTRL_DOWN);
	AddInput (PSP_CTRL_LEFT);
	AddInput (PSP_CTRL_LTRIGGER);
	AddInput (PSP_CTRL_RTRIGGER);
	AddInput (PSP_CTRL_TRIANGLE);
	AddInput (PSP_CTRL_CIRCLE);
	AddInput (PSP_CTRL_CROSS);
	AddInput (PSP_CTRL_SQUARE);
}

/*
===============
triInputPressed
===============
*/
triBool triInputPressed (const triU32 Button)
{
	if (((Input.Pressed & Button) != 0))	// Pressed is TRUE
	{
		Input.Pressed	&= ~Button;			// so set it to FALSE
		return TRUE;						// and return TRUE
	}

	return FALSE;
}

/*
============
triInputHeld
============
*/
triBool triInputHeld (const triU32 Button)
{
	return	((Input.Held & Button) != 0);
}

/*
===========
triInputAny
===========
*/
triBool triInputAny (void)
{
	if (triInputPressed (PSP_CTRL_SELECT)	||
		triInputPressed (PSP_CTRL_START)	||
		triInputPressed (PSP_CTRL_UP)		||
		triInputPressed (PSP_CTRL_RIGHT)	||
		triInputPressed (PSP_CTRL_DOWN)		||
		triInputPressed (PSP_CTRL_LEFT)		||
		triInputPressed (PSP_CTRL_LTRIGGER)	||
		triInputPressed (PSP_CTRL_RTRIGGER)	||
		triInputPressed (PSP_CTRL_TRIANGLE)	||
		triInputPressed (PSP_CTRL_CIRCLE)	||
		triInputPressed (PSP_CTRL_CROSS)	||
		triInputPressed (PSP_CTRL_SQUARE))
		return TRUE;

	return FALSE;
}

/*
================
triInputGetStick
================
*/
triVec2f* triInputGetStick (void)
{
	return &Input.Stick;
}
