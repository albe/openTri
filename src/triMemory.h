/*
Copyright (C) 2000-2007 Tomas Jakobsson.

triMemory.h
*/

#ifndef	__TRIMEMORY_H__
#define	__TRIMEMORY_H__

#include <malloc.h>
#include "triTypes.h"

#ifdef _DEBUG_MEMORY

extern triBool	triMemoryInit		(void);
extern void		triMemoryShutdown	(void);
extern void*	triMemoryAlloc		(triU32 Size, const triChar* pName, const triU32 Line);
extern void		triMemoryFree		(void* pAddress, const triChar* pName, const triU32 Line);
extern triBool	triMemoryCheck		(void);
extern triU32	triMemoryGetUsage	(void);

#define triMalloc(Size)		triMemoryAlloc(Size, __FILE__, __LINE__)
#define triFree(pAddress)	triMemoryFree(pAddress, __FILE__, __LINE__)

#else // _DEBUG_MEMORY

#define triMemoryInit()		(void)1
#define triMemoryShutdown()  
#define triMemoryCheck()	(void)1
#define triMemoryGetUsage()	(void)0

#define triMalloc(Size)		malloc(Size)
#define triFree(pAddress)	free(pAddress)

#endif // _DEBUG_MEMORY

#endif // __TRIMEMORY_H__
