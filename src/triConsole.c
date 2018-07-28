/*
 * triConsole.c: Code for Ingame Console
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
 
#include <pspdebug.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "triMemory.h"
#include "triConsole.h"
#include "triLog.h"

// Linked lists
triCVar* 		triCVars = 0;
triCmdAlias*	triAliases = 0;
triCmd*			triCmds = 0;
triConsole		tricon;


static	triS32		triWait = 0;


// hardcoded commands
triCmd triCmdList[] =	 {	{ "exit", cmd_exit, 0, 0, "Exit console", 0, 0 },
							{ "cls", cmd_cls, 0, 0, "Clear console screen", 0, 0 },
							{ "cmds", cmd_cmds, 0, 0, "Print all available commands", 0, 0 },
							{ "cvars", cmd_cvars, 0, 0, "Print all registered CVars", 0, 0 },
							{ "echo", cmd_echo, 1, "text", "Echo out text to console", 0, 0 },
							{ "wait", cmd_wait, 0, 0, "Wait until next frame", 0, 0 },
							{ "alias", cmd_alias, 1, "name [commands]", "Bind commands to an alias or return current binding", 0, 0 },
							{ "aliases", cmd_aliases, 0, 0, "Print all registered aliases", 0, 0 },
							{ "exec", cmd_exec, 1, "filename", "Execute a script file", 0, 0 },
							{ "eval", cmd_eval, 1, "term", "Evaluate a term.", 0, 0 },
							{ 0 } // List terminator
						};





static triChar* triNullstring = "";

#define MAXARGS 64
static triS32   triCmdCurArgc = 0;
static triChar* triCmdCurArgv[MAXARGS] = {0};
static triChar* triCmdCurArgs = 0;


// command argument parsing
inline triS32 triCmdArgc()
{
	return triCmdCurArgc;
}


inline triChar* triCmdArgv( triS32 n )
{
	if (n>triCmdCurArgc) return(0);
	
	return triCmdCurArgv[n];
}


inline triChar* triCmdArgs()
{
	return triCmdCurArgs;
}



void triCVarRegister( triCVar* var )
{
	if (var==0 || var->name==0) return;
	
	var->next = triCVars;
	triCVars = var;
}


// Convert an infix notation expression to PRN (polish reverse notation)
static triChar* infix2prn( triChar* expr )
{
	triChar opstack[128];
	static triChar result[1025];
	triS32 i = 0;
	triS32 op = 0;
	while (*expr)
	{
		if (*expr==';' || *expr=='\n')
			break;
		if (*expr==' ')
		{
			expr++;
			continue;
		}
		if (*expr=='(')
			opstack[op++] = *expr++;
		else
		if (*expr==')')
		{
			while (i<1024 && op>0 && opstack[--op]!='(')
			{
				result[i++] = opstack[op];
			}
			expr++;
		} else
		if (*expr=='+' || *expr=='-')
		{
			result[i++] = ' ';
			while(i<1024 && op>0 && (opstack[op-1]=='*' || opstack[op-1]=='/'))
			{
				result[i++] = opstack[--op];
			}
			opstack[op++] = *expr++;
		} else
		if (*expr=='*' || *expr=='/')
		{
			result[i++] = ' ';
			while(i<1024 && op>0 && (opstack[op-1]=='*' || opstack[op-1]=='/'))
			{
				result[i++] = opstack[--op];
			}
			opstack[op++] = *expr++;
		} else
			result[i++] = *expr++;
	}
	while (i<1024 && op>0)
		result[i++] = opstack[--op];
	
	result[i] = 0;
	return result;
}


// Parse str for a number or variable name and return it's length
static triS32 varlen( const triChar* str )
{
	if (str[0]=='\\') return 2;
	triS32 i = 0;
	while (str[i])
	{
		if (str[i]==';' || str[i]=='\n' || str[i]==' ' || str[i]=='(' || str[i]==')' || str[i]=='+' || str[i]=='-' || str[i]=='*' || str[i]=='/')
			break;
		i++;
	}
	return i;
}


float triatof( const triChar* str )
{
	if (*str=='\\')
		return (float)(str[1]);
	if (*str=='$')
	{
		triChar buf[64];
		str++;
		triS32 i = 0, max = varlen(str);
		if (max>63) max = 63;
		for (;i<max;i++)
			buf[i] = str[i];
		buf[i] = 0;
		//printf("%s %.2f\n", buf, triCVarGetf( buf ));
		return triCVarGetf( buf );
	}
	if (*str=='0' && (str[1]=='x' || str[1]=='X'))
	{
		triUInt i;
		sscanf( &str[2], "%x", &i );
		return (float)i;
	}
	
	if (*str>='0' && *str<='9')
	{
		float f;
		sscanf( str, "%f", &f );
		return f;
	}
	
	return (TRI_INVALID_VAL);
}


// Evaluate a term by converting to PRN first, then evaluating the variables and operators
// This allows for arithmetic on variable/command arguments, e.g.:
// cl_fpsmax "$cl_fpsmin + 10.0"
// echo "cvar (cl_fpsmax * 4.0 + 13) / 4 is "(($cl_fpsmax * 4.0 + 13) / 4)
static float evaluate( triChar* term )
{
	triChar* prn = infix2prn( term );

	float result[128];
	result[0] = 0.0f;
	triS32 r = 0;
	while (*prn && r<128)
	{
		if (*prn == ' ')
			prn++;
		else if (*prn == '+')
		{
			if (r<2) break;
			result[r-2] = result[r-2] + result[r-1];
			r -= 1;
			prn++;
		}
		else if (*prn == '-')
		{
			if (r<2) break;
			result[r-2] = result[r-2] - result[r-1];
			r -= 1;
			prn++;
		}
		else if (*prn == '*')
		{
			if (r<2) break;
			result[r-2] = result[r-2] * result[r-1];
			r -= 1;
			prn++;
		}
		else if (*prn == '/')
		{
			if (r<2) break;
			result[r-2] = result[r-2] / result[r-1];
			r -= 1;
			prn++;
		}
		else
		{
			result[r++] = triatof( prn );
			//triConsolePrintf("result[%i] = %.2f\n",r-1,result[r-1]);
			prn += varlen(prn);
		}
	}
	return result[0];
}


void triCVarSet( const triChar* name, const triChar* value )
{
	triCVar* res = triCVarFind( name );
	
	if (res==0) return;
	
	if (res->flags&TRI_CVAR_RDONLY)
	{
		triConsolePrintf("CVar is read-only.\n");
		return;
	}
	
	res->fvalue = evaluate( value );
	
	if (res->flags&TRI_CVAR_ALLOC)
		if (res->svalue) triFree(res->svalue);
	
	if (res->fvalue!=TRI_INVALID_VAL)
	{
		res->svalue = triMalloc( 32 );
		snprintf( res->svalue, 31, "%f", res->fvalue );
	}
	else
	{
		res->svalue = triMalloc( strlen(value)+1 );
		strcpy( res->svalue, value );
	}
	res->flags |= TRI_CVAR_ALLOC;
}


void triCVarSetf( const triChar* name, const float value )
{
	triCVar* res = triCVarFind( name );
	
	if (res==0) return;
	
	if (res->flags&TRI_CVAR_RDONLY)
	{
		triConsolePrintf("CVar is read-only.\n");
		return;
	}
	
	res->fvalue = value;
	if (res->flags&TRI_CVAR_ALLOC)
		if (res->svalue) triFree(res->svalue);

	res->svalue = triMalloc( 33 );
	snprintf( res->svalue, 33, "%f", value );
	res->flags |= TRI_CVAR_ALLOC;
}


triChar* triCVarGet( const triChar* name )
{
	triCVar* res = triCVarFind( name );
	
	if (res==0) return(0);
	
	return (res->svalue);
}


float triCVarGetf( const triChar* name )
{
	triCVar* res = triCVarFind( name );
	
	if (res==0) return(TRI_INVALID_VAL);
	
	return (res->fvalue);
}


triChar* triCVarComplete( const triChar* name )
{
	triCVar* list = triCVars;
	triS32 slen = strlen(name);
	while (list!=0)
	{
		if (strncmp(name, list->name, slen)==0)
			return (list->name);
		list = list->next;
	}
	return (0);
}


// execute a cvar command (set value or return value)
triS32 triCVarCmd()
{
	triCVar* var = triCVarFind( triCmdArgv(0) );
	
	if (var==0) return(0);
	
	if (triCmdArgc()<2)
		triConsolePrintf("%s \"%s\"\n", var->name, var->svalue);
	else
		triCVarSet( var->name, triCmdArgv(1) );
	return(1);
}


// Find a cvar matching the name
triCVar* triCVarFind( const triChar* name )
{
	triCVar* list = triCVars;
	
	while (list!=0)
	{
		if (strcmp(name, list->name)==0)
			return (list);
		list = list->next;
	}
	return (0);
}



triCmd* triCmdFind( const triChar* name )
{
	triS32 i = 0;
	while (triCmdList[i].name!=0)
	{
		if (strcmp(name,triCmdList[i].name)==0)
			return (&triCmdList[i]);
		i++;
	}
	
	triCmd* list = triCmds;
	
	while (list!=0)
	{
		if (strcmp(name,list->name)==0)
			return (list);
		list = list->next;
	}
	
	return(0);
}


// Complete a command
triChar* triCmdComplete( const triChar* name )
{
	triS32 i = 0;
	triS32 slen = strlen(name);
	while (triCmdList[i].name!=0)
	{
		if (strncmp(name,triCmdList[i].name,slen)==0)
			return (triCmdList[i].name);
		i++;
	}
	
	triCmd* list = triCmds;
	
	while (list!=0)
	{
		if (strncmp(name,list->name,slen)==0)
			return (list->name);
		list = list->next;
	}
	
	return(0);
}


// Register a new command
triChar* triCmdRegister( const triChar* name, triCmdFunc func, const triS32 minargs, const triChar* help, const triChar* desc, const triChar* details )
{
	if (name==0 || func==0) return("Invalid arguments.");
	if (triCmdFind( name ) || triCVarFind( name ) || triAliasFind( name )) return("Name already used.");
	
	triCmd* cmd = triMalloc(sizeof(triCmd));
	cmd->name = triMalloc(strlen(name)+1);
	strcpy( cmd->name, name );
	cmd->func = func;
	cmd->minargs = minargs;
	cmd->help = 0;
	if (help!=0)
	{
		cmd->help = triMalloc(strlen(help)+1);
		strcpy( cmd->help, help );
	}
	cmd->description = 0;
	if (desc!=0)
	{
		cmd->description = triMalloc(strlen(desc)+1);
		strcpy( cmd->description, desc );
	}
	cmd->details = 0;
	if (details!=0)
	{
		cmd->details = triMalloc(strlen(details)+1);
		strcpy( cmd->details, details );
	}
	cmd->next = triCmds;
	triCmds = cmd;
	
	return(0);
}


// Execute a command
triS32 triCmdCmd()
{
	triCmd* cmd = triCmdFind( triCmdArgv(0) );
	
	if (cmd==0) return(0);
	
	if (triCmdArgc()<cmd->minargs+1)
	{
		triConsolePrintf( "Usage: %s %s\n", cmd->name, cmd->help );
		if (cmd->description!=0)
			triConsolePrintf("%s\n", cmd->description);
		if (cmd->details!=0)
			triConsolePrintf("%s\n", cmd->details);
	}
	else
		if (cmd->func!=0)
		{
			triChar* ret = cmd->func();
			if (ret) triConsolePrintf("%s\n", ret);
		}
	
	return(1);
}


triChar* triAliasRegister( const triChar* name, const triChar* cmd )
{
	// Don't allow aliases with same name as commands or cvars
	if (triCmdFind( name ) || triCVarFind( name )) return("Name already used.");
	
	triCmdAlias* alias = triAliasFind( name );
	
	if (alias==0)
		alias = triMalloc(sizeof(triCmdAlias));
	else
	{
		triFree(alias->name);
		triFree(alias->cmd);
	}
	
	if (alias==0) return("Couldn't create alias.");
	
	alias->name = triMalloc(strlen(name)+1);
	strcpy( alias->name, name );
	alias->cmd = triMalloc(strlen(cmd)+1);
	strcpy( alias->cmd, cmd );
	
	alias->next = triAliases;
	triAliases = alias;
	
	return(0);
}


// Find an alias matching the name
triCmdAlias* triAliasFind( const triChar* name )
{
	triCmdAlias* list = triAliases;
	
	while (list!=0)
	{
		if (strcmp(name, list->name)==0)
			return (list);
		list = list->next;
	}
	return(0);
}


// Execute an alias
triS32 triAliasCmd()
{
	triCmdAlias* alias = triAliasFind( triCmdArgv(0) );
	
	if (alias==0)	return(0);

	if (triCmdArgc()<2)
	{
		triCmdExecute( alias->cmd );
		return (1);
	}
	
	// Allow aliases to to be fed with arguments, making things like 'alias @ echo;@ "aliased echo output"' possible
	triS32 len = strlen(alias->cmd)+1+strlen(triCmdArgs())+1;
	/*triS32 i = 1;
	for (;i<triCmdArgc();i++)
		len += strlen(triCmdArgv(i))+1;
	*/
	triChar* buffer = triMalloc(len);
	if (buffer==0)
		triCmdExecute( alias->cmd );
	else
	{
		sprintf( buffer, "%s %s", alias->cmd, triCmdArgs() );
		/*len = strlen(alias->cmd)+1;
		i = 1;
		for (;i<triCmdArgc();i++)
		{
			sprintf( buffer+len, "%s ", triCmdArgv(i) );
			len += strlen(triCmdArgv(i))+1;
		}
		buffer[len-1] = 0;*/
		triCmdExecute( buffer );
		triFree( buffer );
	}

	return(1);
}


// Create a new alias or change an existing one
triChar* cmd_alias()
{

	triCmdAlias* alias = triAliasFind( triCmdArgv(1) );
	
	if (alias==0)
	{
		if (triCmdArgc()<3)
			return("Alias not found.");
		return triAliasRegister( triCmdArgv(1), triCmdArgv(2) );
	}
	
	static triChar triReturnString[256];
	// Alias exists? We change it, or print the current commands
	if (triCmdArgc()<3)
	{
		snprintf( triReturnString, 256, "Current commands:\n\"%s\" \"%s\"", alias->name, alias->cmd );
		return triReturnString;
	}
	else
	{
		triFree( alias->cmd );
		alias->cmd = triMalloc(strlen(triCmdArgv(2))+1);
		strcpy( alias->cmd, triCmdArgv(2) );
	}
	
	return(0);
}


triChar* cmd_exit()
{
	triConsolePrint( "Exiting the console\n" );
	triConsoleToggle();
	return(0);
}

triChar* cmd_cls()
{
	triConsoleClear();
	return(0);
}


static inline void triCmdPrint( triCmd* cmd )
{
	if (cmd->help)
	{
		triChar scmd[32];
		snprintf( scmd, 32, "%s %s", cmd->name, cmd->help );
		triConsolePrintf( "%-31s - %s\n", scmd, cmd->description );
	}
	else
		triConsolePrintf( "%-31s - %s\n", cmd->name, cmd->description );
}

triChar* cmd_cmds()
{
	triS32 i = 0;
	while (triCmdList[i].name!=0)
	{
		triCmdPrint( &triCmdList[i] );
		i++;
	}
	
	triCmd* cmd = triCmds;
	while (cmd!=0)
	{
		triCmdPrint( cmd );
		cmd = cmd->next;
	}
	return(0);
}

triChar* cmd_cvars()
{
	triCVar* var = triCVars;
	while (var!=0)
	{
		triConsolePrintf("%s%s \"%s\"\n", (var->flags&TRI_CVAR_RDONLY)?"*":"",var->name, var->svalue);
		var = var->next;
	}
	return(0);
}

triChar* cmd_aliases()
{
	triCmdAlias* list = triAliases;
	
	while (list!=0)
	{
		triConsolePrintf("%s \"%s\"\n", list->name, list->cmd);
		list = list->next;
	}
	return(0);
}

triChar* cmd_echo()
{
	triS32 i;
	for (i=1;i<triCmdArgc();i++)
		triConsolePrintf( "%s ", triCmdArgv(i) );
	triConsolePrintf("\n");
	return(0);
}

triChar* cmd_wait()
{
	triWait = 1;
	return(0);
}

triChar* cmd_exec()
{
	triChar* file = triCmdArgv(1);
	
	FILE* fp = fopen( file, "r" );
	if (fp==0) return("Error opening file.");
	
	fseek(fp,0,SEEK_END);
	triS32 sz = ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	triChar* buffer = triMalloc(sz+1);
	if (buffer==0) return("Error allocating script buffer.");
	fread( buffer, 1, sz, fp );
	fclose(fp);
	
	triCmdExecute( buffer );
	triFree(buffer);
	return(0);
}


triChar* cmd_eval()
{
	triFloat val = evaluate(triCmdArgs());
	triConsolePrintf("%f\n", val);
	return (0);
}

// Execute the line containing cmds as if written to console
void triCmdExecute( triChar* line )
{
	triChar* buf = line;
	if (*buf=='>') buf++;
	while (buf && *buf)
	{
		buf = triCmdTokenize( buf );

		if (triCmdArgc()<1) continue;
		
		if (triCmdCmd()) continue;
		
		if (triCVarCmd()) continue;
		
		if (triAliasCmd()) continue;

		triConsolePrintf("Command \"%s\" not found.\n", triCmdArgv(0));
	}
}


// Tokenize the string line until either a ;, \n or \0 is found and return pointer to that position
// ; and spaces may be included in single arguments by encapsulating in quotes ""
// spaces may be included in single arguments by encapsulating in round brackets ()
// This allows things like "cl_backcolor (1.0 0.7 0.1)"
triChar* triCmdTokenize( triChar* line )
{
	// triFree all current command argument buffers
	if (triCmdCurArgs)
	{
		triFree(triCmdCurArgs);
		triCmdCurArgs = 0;
	}
	triS32 i = 0;
	for (;i<MAXARGS;i++)
		if (triCmdCurArgv[i])
		{
			triFree(triCmdCurArgv[i]);
			triCmdCurArgv[i] = 0;
		}

	i = 0;
	// strip whitespaces
	while (*line && *line<=' ') line++;
	triCmdCurArgc = 0;
	triS32 quotes = 0;
	triS32 brackets = 0;
	triChar* args = 0;
	
	while (1)
	{
		if (line[i]=='"') quotes++;
		if (line[i]=='(') brackets++;
		if (line[i]==')') if (brackets) brackets--;
		if ((!(quotes&1) && !brackets && line[i]==' ') ||
		    (!(quotes&1) && line[i]==';') ||
		    (line[i]=='\n') ||
		    (line[i]==0) ||
		    (!(quotes&1) && line[i]=='"'))
		{
			// Save arguments start
			if (triCmdCurArgc==1)
				args = line;

			if (line[0]=='$')
			{
				triChar buf[64];
				strncpy( buf, &line[1], i-1 );
				buf[i-1] = 0;
				triCVar* cvar = triCVarFind(buf);
				if (cvar!=0)
				{
					triCmdCurArgv[triCmdCurArgc] = triMalloc(strlen(cvar->svalue)+1);
					strcpy(triCmdCurArgv[triCmdCurArgc], cvar->svalue);
					goto DONEARGV;
				}
			}
			// Strip quotes
			triS32 len = i;
			if (line[0]=='"' && line[i]=='"')
			{
				i--;
				line++;
				len-=1;
			}
			triCmdCurArgv[triCmdCurArgc] = triMalloc(len+1);
			if (triCmdCurArgv[triCmdCurArgc]==0)
			{
				triLogError("ERROR ALLOCATING ARGV BUFFER!\n");
				return(0);
			}
			strncpy( triCmdCurArgv[triCmdCurArgc], line, len );
			triCmdCurArgv[triCmdCurArgc][len] = 0;
		DONEARGV:
			triCmdCurArgc++;
			if (line[i]==0 || line[i]=='\n')
				break;
			if (line[i]==';')
				break;
			i++;
			line += i;
			i = 0;
			while (*line && *line<=' ') line++;
			continue;
		}
		i++;
	}
	
	// Create args string
	if (triCmdCurArgc>1)
	{
		triS32 size = &line[i]-args;
		triCmdCurArgs = triMalloc(size+1);
		strncpy( triCmdCurArgs, args, size );
		triCmdCurArgs[size] = 0;
	}
	
	if (line[i]==0)
		return 0;
	return &line[i+1];
}


void triConsoleInit()
{
	memset( tricon.text, 0, MAXCONSOLECHARS );
	tricon.display = 0;
	tricon.x = 0;
	tricon.current = 0;
	tricon.visible = 0;
	tricon.maxlines = 32;
	tricon.numlines = 0;
	//pspDebugScreenInit();
}


void triConsoleClose()
{
	// triFree all current command argument buffers
	if (triCmdCurArgs)
	{
		triFree(triCmdCurArgs);
		triCmdCurArgs = 0;
	}
	triS32 i = 0;
	for (;i<MAXARGS;i++)
		if (triCmdCurArgv[i])
		{
			triFree(triCmdCurArgv[i]);
			triCmdCurArgv[i] = 0;
		}

	// triFree all aliases
	triCmdAlias* list = triAliases;
	while (list!=0)
	{
		triCmdAlias* next = list->next;
		triFree(list->name);
		triFree(list->cmd);
		triFree(list);
		list = next;
	}

	// triFree all alloc'd CVars
	triCVar* var = triCVars;
	while (var!=0)
	{
		triCVar* next = var->next;
		if (var->flags&TRI_CVAR_ALLOC)
			triFree( var->svalue );
		var = next;
	}
}


void triConsoleToggle()
{
	tricon.visible ^= 1;
	if (tricon.visible)
		triConsolePrint("\n>");
}

void triConsolePrint( const triChar* text )
{
	#ifdef CON_STDOUT
	printf( text );
	#else
	
	
	#endif
}

void triConsolePrintf( const triChar* fmt, ... )
{
	#ifdef CON_STDOUT
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	#else
	
	
	#endif
}

void triConsoleDraw()
{
	#ifndef CON_STDOUT
	// Only need to draw something if we don't output to stdout
	#endif
}

void triConsoleClear()
{
	#ifdef CON_STDOUT
	triS32 i = 0;
	for (;i<64;i++)
		printf("\n");
	printf(">");
	#else
	memset( tricon.text, 0, MAXCONSOLEtriCharS );
	tricon.display = 0;
	tricon.x = 0;
	tricon.current = 0;
	tricon.numlines = 0;
	#endif
}

void triConsoleResize( const triS32 lines )
{
	#ifndef CON_STDOUT
	
	#endif
}

static triS32 linepos = 0;
void triConsoleUpdate()
{
	if (!tricon.visible) return;

	static triChar line[1024];
	
	/*
	triS32 ch;
	while (1)
	{
		ch = getc( stdin );
		if (ch==EOF||ch==0)
			break;
		
		if (ch=='\t')
		{
			triChar* cmd = triCmdComplete(line);
		}
		else
		if (ch=='\n')
		{
			line[linepos] = 0;
			linepos = 0;
			if (strlen(line)>0)
				triCmdExecute( line );
			triConsolePrint(">");
		}
		else
			line[linepos++] = (triChar)ch;
	}
	*/
	gets( line );
	if (strlen(line)>0)
		triCmdExecute( line );
	triConsolePrint(">");
}

triS32  triConsoleVisible()
{
	return tricon.visible;
}
