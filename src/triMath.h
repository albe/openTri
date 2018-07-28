/*
Copyright (C) 2000-2006 Tomas Jakobsson.

triMath.h
*/

#ifndef	__TRIMATH_H__
#define	__TRIMATH_H__

#include <math.h>
#include <time.h>
#include "triTypes.h"

#define TRI_PI				3.1415926535897932384626433832795028841971693993751058209749445923f
#define TRI_RAD_TO_DEG(X)	((X) * (180.0f / TRI_PI))
#define TRI_DEG_TO_RAD(X)	((X) * (TRI_PI / 180.0f))
#define TRI_EPSILON			0.03125f
#define	TRI_RAND_MAX		RAND_MAX
#define	TRI_RAND_MAX_HALF	RAND_MAX / 2.0f
#define TRI_NANMASK			(255<<23)
#define TRI_IS_NAN(X)		(((*(triS32*)&X)&BASE_MATH_NANMASK)==BASE_MATH_NANMASK)

/*
===========
triMathInit
===========
*/
static inline triBool triMathInit (void)
{
	srand (time (NULL));
	return TRUE;
}

/*
============
triMathBound
============
*/
static inline triFloat triMathBound (const triFloat Min, const triFloat Num, const triFloat Max)
{
	if ((Min) < (Max))
	{
		if		((Num) < (Min))	return (Min);
		else if ((Num) > (Max))	return (Max);
		else					return (Num);
	}
	else
	{
		if		((Num) < (Max))	return (Max);
		else if ((Num) > (Min))	return (Min);
		else					return (Num);
	}
}

/*
=============
triMathRandom
=============
*/
static inline triFloat triMathRandom (const triFloat Min, const triFloat Max)
{
	return (rand () * (((Max) - (Min)) * (1.0f / (triFloat)TRI_RAND_MAX)) + (Min));
}

/*
===========
triMathRInt
===========
*/
static inline triS32 triMathRInt (const triFloat X)
{
	return (triS32)((X) >= 0 ? (X) + 0.5 : (X) - 0.5);
}

/*
===========
triMathWrap
===========
*/
static inline triFloat triMathWrap (const triFloat Min, triFloat Num, const triFloat Max)
{
	triFloat	Diff;

	if ((Min) < (Max))
	{
		Diff	= (Max) - (Min);

		while ((Num) < (Min))	(Num)	+= Diff;
		while ((Num) > (Max))	(Num)	-= Diff;
	}
	else
	{
		Diff	= (Min) - (Max);

		while ((Num) < (Max))	(Num)	+= Diff;
		while ((Num) > (Min))	(Num)	-= Diff;
	}

	return	(Num);
}

/*
==============
triMathBetween
==============
*/
static inline triBool triMathBetween (const triFloat Min, const triFloat Num, const triFloat Max)
{
	if ((Min) < (Max))
	{
		if		((Num) < (Min))	return FALSE;
		else if	((Num) > (Max))	return FALSE;
		else					return TRUE;
	}
	else
	{
		if		((Num) > (Min))	return FALSE;
		else if	((Num) < (Max))	return FALSE;
		else					return TRUE;
	}
}

/*
============
triMathShift
============
*/
static inline void triMathShift (triFloat Num1, triFloat Num2)
{
	triFloat	Temp;

	Temp	= Num1;
	Num1	= Num2;
	Num2	= Temp;
}

/*
===============
triMathNextPow2
===============
*/
static inline triU32 triMathNextPow2 (triU32 In)
{
	triU32	Out;

	for (Out=2; Out<In; Out<<=1);

	return Out;
}

#endif // __TRI_MATH__
