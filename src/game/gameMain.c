#include <pspkernel.h>

#include "../tri3d.h"
#include "../triTypes.h"
#include "../triLog.h"
#include "../triMemory.h"
#include "../triModel.h"
#include "../triInput.h"
#include "../triGraphics.h"
#include "../triAt3.h"
#include "../triWav.h"
#include "../triNet.h"
#include "../triError.h"

PSP_MODULE_INFO("zeGame", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

static int gameRunning = 1;

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	gameRunning = 0;
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

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

/* Entry point */
int main(int argc, char **argv)
{
	if(!triAt3Init()) triError("Error starting atrac3 playback");
	
	if(!triNetInit()) triError("Error starting net");

	// Set up callbacks
	SetupCallbacks();
	
	// Game inits
	if(!triLogInit()) triError("Error starting Logging");
	triInit( GU_PSM_8888 );
	tri3dInit();
	if(!triInputInit()) triError("Error starting input");
	triModelManagerInit();
	if(!triWavInit()) triError("Error starting WAV playback");
	
	if(!gameLoadModels()) triError("Error loading models");
	
	while (gameRunning)
	{
		// Game state stuff
	}
	
	// Game shutdown
	triModelManagerFreeAll();
	triInputShutdown();
	triClose();
	tri3dClose();
	triMemoryShutdown();

	sceKernelExitGame();
	
	return 0;
}

	
