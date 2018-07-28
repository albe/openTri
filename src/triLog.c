/*
Copyright (C) 2000-2007 Tomas Jakobsson.

triLog.c
*/

#ifdef _DEBUG_LOG

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "triTypes.h"
#include "triLog.h"

/*
==========
triLogInit
==========
*/
triBool triLogInit (void)
{
	unlink ("triLog.txt");
	unlink ("triErrorLog.txt");
	unlink ("triMemoryLog.txt");

	triLogPrint ("Initializing triLog\r\n");

	return TRUE;
}

/*
==============
triLogShutdown
==============
*/
void triLogShutdown (void)
{
	triLogPrint ("Shutdown triLog\r\n");
}

/*
===========
triLogPrint
===========
*/
void triLogPrint (const triChar* pMessage, ...)
{
	triChar	Message[256];
	va_list	ArgPtr;
	FILE*	pFile;

	va_start (ArgPtr, pMessage);
	vsnprintf (Message, sizeof (Message), pMessage, ArgPtr);
	va_end (ArgPtr);
#ifdef _DEBUG_STDOUT
	printf (Message);
#endif
	pFile = fopen ("triLog.txt", "a");

	fwrite (Message, strlen (Message), 1, pFile);

	fclose (pFile);
}

/*
===========
triLogError
===========
*/
void triLogError (const triChar* pMessage, ...)
{
	triChar	Message[256];
	va_list	ArgPtr;
	FILE*	pFile;

	va_start (ArgPtr, pMessage);
	vsnprintf (Message, sizeof (Message), pMessage, ArgPtr);
	va_end (ArgPtr);
#ifdef _DEBUG_STDOUT
	printf (Message);
#endif
	pFile = fopen ("triErrorLog.txt", "a");

	fwrite (Message, strlen (Message), 1, pFile);

	fclose (pFile);
}

/*
============
triLogMemory
============
*/
void triLogMemory (const triChar* pMessage, ...)
{
	triChar	Message[256];
	va_list	ArgPtr;
	FILE*	pFile;

	va_start (ArgPtr, pMessage);
	vsnprintf (Message, sizeof (Message), pMessage, ArgPtr);
	va_end (ArgPtr);

	//printf (Message);	// No stdout print of Memory loggs to avoid flooding ingame console

	pFile = fopen ("triMemoryLog.txt", "a");

	fwrite (Message, strlen (Message), 1, pFile);

	fclose (pFile);
}

#endif // _DEBUG_LOG
