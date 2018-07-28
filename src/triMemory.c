/*
Copyright (C) 2000-2006 Tomas Jakobsson.

triMemory.c
*/

#ifdef _DEBUG_MEMORY

#include <string.h>
#include <malloc.h>
#include "triTypes.h"
#include "triLog.h"

#define		MAX_AMOUNT	4096

typedef struct triMemoryBlock
{
	const triChar*	pName;
	void*			pAddress;
	triU32			Line;
	triU32			Size;
} triMemoryBlock;

typedef struct triMemory
{
	triMemoryBlock	Block[MAX_AMOUNT];
	triU32			PeakAlloced;
	triU32			CurAlloced;
	triU32			Alloced;
	triU32			Freed;
	triU32			Allocs;
	triU32			Frees;
} triMemory;

static triMemory Memory;

/*
=============
triMemoryInit
=============
*/
triBool triMemoryInit (void)
{
	memset (&Memory, 0, sizeof(triMemory));

	triLogPrint ("Initializing triMemory\r\n");

	return TRUE;
}

/*
=================
triMemoryShutdown
=================
*/
void triMemoryShutdown (void)
{
	triU32	i;

	triLogMemory ("\r\n\r\n");
	triLogMemory ("Amount of allocs: %i\r\n", Memory.Allocs);
	triLogMemory ("Amount of frees:  %i\r\n", Memory.Frees);
	triLogMemory ("Total amount of alloced memory: %7.2f MiB, %10.2f KiB, %9i B\r\n", Memory.Alloced / 1024.0f / 1024.0f, Memory.Alloced / 1024.0f, Memory.Alloced);
	triLogMemory ("Total amount of freed memory:   %7.2f MiB, %10.2f KiB, %9i B\r\n", Memory.Freed / 1024.0f / 1024.0f, Memory.Freed / 1024.0f, Memory.Freed);
	triLogMemory ("Amount of still alloced memory: %7.2f MiB, %10.2f KiB, %9i B\r\n", Memory.CurAlloced / 1024.0f / 1024.0f, Memory.CurAlloced / 1024.0f, Memory.CurAlloced);
	triLogMemory ("Peak of alloced memory:         %7.2f MiB, %10.2f KiB, %9i B\r\n", Memory.PeakAlloced / 1024.0f / 1024.0f, Memory.PeakAlloced / 1024.0f, Memory.PeakAlloced);

	for (i=0; i<MAX_AMOUNT; i++)
	{
		if (Memory.Block[i].pAddress)
		{
			triLogMemory ("Found alloced memory %4i at Address: %p Size: %9i File: %-24s Line: %4i\r\n", i, Memory.Block[i].pAddress, Memory.Block[i].Size,Memory.Block[i].pName, Memory.Block[i].Line);
		}
	}

	triLogPrint ("Shutdown triMemory\r\n");
}

/*
==============
triMemoryAlloc
==============
*/
void* triMemoryAlloc (triU32 Size, const triChar* pName, const triU32 Line)
{
	void*	pMemory;
	triU32	i;

	Size	= Size + 2;
	pMemory	= malloc (Size);

	for (i=0; i<MAX_AMOUNT; i++)
	{
		if (Memory.Block[i].pAddress == NULL)
		{
			Memory.Block[i].pAddress	= pMemory;
			Memory.Block[i].Line		= Line;
			Memory.Block[i].Size		= Size;
			Memory.Block[i].pName		= pName;

			triLogMemory ("malloc %4i at Address: %p Size: %9i File: %-24s Line: %4i\r\n", i, Memory.Block[i].pAddress, Memory.Block[i].Size, Memory.Block[i].pName, Memory.Block[i].Line);

			break;
		}
	}

	Memory.CurAlloced	+= Memory.Block[i].Size;
	Memory.Alloced		+= Memory.Block[i].Size;
	Memory.Allocs++;

	if (Memory.PeakAlloced < Memory.CurAlloced)
		Memory.PeakAlloced = Memory.CurAlloced;

	((triU8*)pMemory)[Size-2]	= 0xDE;
	((triU8*)pMemory)[Size-1]	= 0xAD;

	return pMemory;
}

/*
=============
triMemoryFree
=============
*/
void triMemoryFree (void* pAddress, const triChar* pName, const triU32 Line)
{
	triU32	i;

	for (i=0; i<MAX_AMOUNT; i++)
	{
		if (Memory.Block[i].pAddress && Memory.Block[i].pAddress == pAddress)
			break;
	}

	if (i == MAX_AMOUNT)
	{
		triLogMemory ("Tried to delete unalloced memory at Address: %p File: %-24s Line: %4i\r\n", pAddress, pName, Line);

		triLogError ("Tried to free unalloced memory\r\n");

		return;
	}

	triLogMemory ("free   %4i at Address: %p Size: %9i File: %-24s Line: %4i\r\n", i, Memory.Block[i].pAddress, Memory.Block[i].Size, pName, Line);

	Memory.CurAlloced	-= Memory.Block[i].Size;
	Memory.Freed		+= Memory.Block[i].Size;
	Memory.Frees++;

	memset (&Memory.Block[i], 0, sizeof(triMemoryBlock));

	free (pAddress);
}

/*
==============
triMemoryCheck
==============
*/
triBool triMemoryCheck (void)
{
	triU32	i;
	triBool	Corrupt;

	Corrupt	= FALSE;

	for (i=0; i<MAX_AMOUNT; i++)
	{
		if (Memory.Block[i].pAddress != NULL)
		{
			if (((triU8*)Memory.Block[i].pAddress)[Memory.Block[i].Size-2] != 0xDE ||
				((triU8*)Memory.Block[i].pAddress)[Memory.Block[i].Size-1] != 0xAD)
			{
				triLogMemory ("Corruption %4i at Address: %p Size: %9i File: %-24s Line: %4i\r\n", i, Memory.Block[i].pAddress, Memory.Block[i].Size, Memory.Block[i].pName, Memory.Block[i].Line);

				Corrupt	= TRUE;
			}
		}

		return Corrupt;
	}

	return FALSE;
}

/*
=================
triMemoryGetUsage
=================
*/
triU32 triMemoryGetUsage (void)
{
	return Memory.CurAlloced;
}

#endif // _DEBUG_MEMORY
