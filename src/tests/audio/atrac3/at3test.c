#include <pspkernel.h>
#include <pspctrl.h>
#include <pspaudio.h>
#include <pspdebug.h>
#include "../../../triAt3.h"
#include "../../../triLog.h"
#include "../../../triMemory.h"
#include "../../../triTypes.h"

PSP_MODULE_INFO("triAt3Test", PSP_MODULE_USER, 1, 1);
PSP_HEAP_SIZE_KB(20480);

int running = 1;

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	running = 0;
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

triSInt loop = 0; // Looping flag

triVoid drawText()
{
	pspDebugScreenSetXY(0, 0);
	pspDebugScreenPrintf("R/L TRIGGER to increase/decrease volume. Current volume: %05d", triAt3GetVol());
	pspDebugScreenSetXY(0, 1);
	pspDebugScreenPrintf("START to loop/not loop. Currently looping: %i", loop);
	pspDebugScreenSetXY(0, 2);
	pspDebugScreenPrintf("CIRCLE to start playing");
	pspDebugScreenSetXY(0, 3);
	pspDebugScreenPrintf("CROSS to stop playing");
	pspDebugScreenSetXY(0, 4);
	pspDebugScreenPrintf("SQUARE to pause playing");
}

int main(int argc, char **argv)
{
	pspDebugScreenInit();
	
	triLogInit();

	triAt3Init();
	
	SetupCallbacks();
	
	triAt3Load("test.at3");
	
	triAt3SetLoop(loop); // Don't loop
	
	SceCtrlData pad;
	triSInt lastpad = 0;
	
	while (running)
	{
		drawText();

		sceCtrlPeekBufferPositive(&pad, 1);
     
		if(pad.Buttons != lastpad)
		{
			lastpad = pad.Buttons;
     
			if(pad.Buttons & PSP_CTRL_CROSS)
				triAt3Stop(); // Stop playing
                                       
			if(pad.Buttons & PSP_CTRL_CIRCLE)
				triAt3Play(); // Start playing
                                       
			if(pad.Buttons & PSP_CTRL_SQUARE)
				triAt3Pause(); // Pause playing
                                       
			if(pad.Buttons & PSP_CTRL_START)
			{
				loop ^= 1;
				triAt3SetLoop(loop); // Change loop
			}
		}
                  
		if(pad.Buttons & PSP_CTRL_RTRIGGER)
		{
			if(triAt3GetVol() < PSP_AUDIO_VOLUME_MAX)
				triAt3SetVol(triAt3GetVol() + 1); // Increase volume
		}
                  
		if(pad.Buttons & PSP_CTRL_LTRIGGER)
		{
			if(triAt3GetVol() > 0)
				triAt3SetVol(triAt3GetVol() - 1); // Decrease volume
		}
	}
         
	triAt3Free();
	
	sceKernelExitGame();

	return 0;
}
