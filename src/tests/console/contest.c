#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include "../../triTypes.h"
#include "../../triLog.h"
#include "../../triConsole.h"
#include "../../triMemory.h"


PSP_MODULE_INFO("triConTest", 0x0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

static int isrunning = 1;

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	isrunning = 0;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread,
				     0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


/* Simple thread */
int main(int argc, char **argv)
{
	SetupCallbacks();
	
	triLogInit();
	triConsoleInit();
	
	CVARF( cl_fps, 60.0 );
	CVARS( cl_name, "Raphael" );
	CVARS_RD( sv_name, "triEngine" );
	
	triCVarRegister( &cl_fps );
	triCVarRegister( &cl_name );
	triCVarRegister( &sv_name );
	

	printf("\nPlease type 'tty' to connect to the ingame console.\n");
	printf("\nType '~.' to return to PSPLink shell.\n");
	printf("Inside the console, type 'exit' to quit the program.\n");
	printf("Type 'cmds' to get a list of available commands.\n");
	printf("Type 'cvars' to get a list of registered cvars.\n");
	
	triConsoleToggle();
	
	while (triConsoleVisible())
	{
		triConsoleUpdate();
		
		sceKernelDelayThread(250*1000);
	}

	triConsoleClose();
	triMemoryShutdown();
	sceKernelExitGame();
	return 0;
}
