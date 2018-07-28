/*
 * triConsole.h: Header for Ingame Console
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 Alexander Berl 'Raphael' <raphael@fx-world.org>
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
 
#ifndef __TRICONSOLE_H__
#define __TRICONSOLE_H__


#include "triTypes.h"


// A command function prototype
// Arguments can be parsed through triCmdArgc() and triCmdArgv(n)/triCmdArgs()
// Return must be 0 on success or a pointer to a string containing an error message
// See cmd_* functions for examples
typedef triChar* (*triCmdFunc)();

#define TRI_INVALID_VAL (float)1e64

#define TRI_CVAR_RDONLY 1
#define TRI_CVAR_ALLOC 2
#define TRI_CVAR_ARCHIVE 4

// Macros for easy cvar declarations:
// CVARF( cl_fpsmax, 60.0 );
// CVARS( cl_playername, "Barney" );
#define CVARS(x,s) triCVar x = { #x, s, TRI_INVALID_VAL, 0 };
#define CVARF(x,f) triCVar x = { #x, #f, (triFloat)f, 0 };
#define CVARS_RD(x,s) triCVar x = { #x, s, TRI_INVALID_VAL, TRI_CVAR_RDONLY };
#define CVARF_RD(x,f) triCVar x = { #x, #f, (triFloat)f, TRI_CVAR_RDONLY };

typedef struct triCVar {
	triChar*		name;
	triChar*		svalue;
	triFloat		fvalue;
	triU32			flags;
	
	struct triCVar* next;
	} triCVar;


//extern triCVar* triCVars;



typedef struct triCmd {
	triChar*		name;
	triCmdFunc		func;
	
	triS32			minargs;		// minimum number of arguments needed for function
	triChar*		help;			// argument list (optional arguments in [] brackets)
	triChar*		description;	// short description of functionality
	triChar*		details;		// detailed information on arguments
	
	struct triCmd* next;
	} triCmd;


/*
 * command aliases to allow a simple form of scripting as quake console does
 * ie. alias "myscript" "echo 'this is text from a script!'; cl_fpsmax 30.0; echo 'Set FPS max to 30.';"
 */
typedef struct triCmdAlias {
	triChar*		name;
	triChar*		cmd;

	struct triCmdAlias* next;
	} triCmdAlias;


/* keep private
extern triCmdAlias* trialiases;

extern triCmd* triCmds;
*/



#define CON_STDOUT 1		// Make console output to stdout


#define MAXCONSOLECHARS 16*1024
typedef struct triConsole {
	triChar			text[MAXCONSOLECHARS];
	triS32			current;		// offset to current line
	triS32			x;				// position in current line
	triS32			display;		// bottom line
	
	triS32			numlines;		// number of lines in console
	triS32			maxlines;		// maximal visible lines
	triS32			visible;
	} triConsole;


extern triConsole tricon;


/* 
 * triCVars - implementation following cvars from quake (tm)
 */
triFloat triatof( const triChar* str );

void triCVarRegister( triCVar* var );
void triCVarSet( const triChar* name, const triChar* value );
void triCVarSetf( const triChar* name, const triFloat value );
triChar* triCVarGet( const triChar* name );
triFloat triCVarGetf( const triChar* name );

triChar* triCVarComplete( const triChar* name );
// Execute a cvar command (set value or return value)
triS32 triCVarCmd();

// Find a cvar matching the name
triCVar* triCVarFind( const triChar* name );


/*
 * triAlias functionality
 */

// Register a new alias
triChar* triAliasRegister( const triChar* name, const triChar* cmd );
// Find an alias matching the name
triCmdAlias* triAliasFind( const triChar* name );
// Execute an alias command
triS32 triAliasCmd();

/*
 * triCmd functionality
 */

// command argument parsing
triS32 triCmdArgc();
triChar* triCmdArgv( triS32 n );
triChar* triCmdArgs();


// Complete a command
triChar* triCmdComplete( const triChar* name );
// Register a new command
triChar* triCmdRegister( const triChar* name, triCmdFunc func, const triS32 minargs, const triChar* help, const triChar* desc, const triChar* details );
triCmd* triCmdFind( const triChar* name );
// Execute a command
triS32 triCmdCmd();

// Execute the line containing cmds as if written to console
void triCmdExecute( triChar* line );
// Tokenize a line and make the tokens available through triCmdArgv(n). Return pointer to end of line
triChar* triCmdTokenize( triChar* line );



triChar* cmd_aliases();
triChar* cmd_exit();
triChar* cmd_cls();
triChar* cmd_cmds();
triChar* cmd_cvars();
triChar* cmd_echo();
triChar* cmd_wait();
triChar* cmd_alias();
triChar* cmd_exec();
triChar* cmd_eval();


/*
 * triConsole functionality
 */
void triConsoleInit();
void triConsoleClose();
void triConsoleToggle();
void triConsolePrint( const triChar* text );
void triConsolePrintf( const triChar* fmt, ... );
void triConsoleDraw();
void triConsoleClear();
void triConsoleResize( const triS32 lines );
void triConsoleUpdate();
triS32  triConsoleVisible();

#endif // __TRICONSOLE_H__
