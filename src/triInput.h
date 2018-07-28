/*
Copyright (C) 2000-2007 Tomas Jakobsson.

triInput.h
*/

#ifndef	__TRIINPUT_H__
#define	__TRIINPUT_H__

#include <pspctrl.h>
#include "triTypes.h"

extern triBool		triInputInit		(void);
extern void			triInputShutdown	(void);
extern void			triInputUpdate		(void);
extern triBool		triInputAny			(void);
extern triBool		triInputPressed		(const triU32 Button);
extern triBool		triInputHeld		(const triU32 Button);
extern triVec2f*	triInputGetStick	(void);

#endif // __TRIINPUT_H__
