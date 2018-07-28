/*
Copyright (C) 2000-2007 Tomas Jakobsson.

triLog.h
*/

#ifndef	__TRILOG_H__
#define	__TRILOG_H__

#include "triTypes.h"

#define _DEBUG_STDOUT

#ifdef _DEBUG_LOG

extern triBool	triLogInit		(void);
extern void		triLogShutdown	(void);
extern void		triLogPrint		(const triChar* pMessage, ...);
extern void		triLogError		(const triChar* pMessage, ...);
extern void		triLogMemory	(const triChar* pMessage, ...);

#else // _DEBUG_LOG

#define	triLogInit() 
#define	triLogShutdown() 
#define	triLogPrint(...) 
#define	triLogError(...) 
#define	triLogMemory(...) 

#endif // _DEBUG_LOG

#endif // __TRILOG_H__
