////////////////////////////////////////////////////////////
// WAV naming format is samplerate_bitrate_channels.wav
//
// ie. a 44khz, 16bit stereo file would be named 44_16_2.wav
//
///////////////////////////////////////////////////////////

#include <pspkernel.h>

#include "../../../triWav.h"
#include "../../../triLog.h"
#include "../../../triMemory.h"
#include "../../../triTypes.h"
#include "../../../triAudioLib.h"

PSP_MODULE_INFO("triWavTest", 0, 1, 1);
PSP_HEAP_SIZE_KB(20480);

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	
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

	triMemoryInit();

	triWavInit();
	
	triWav *wavFile = triWavLoad("44_16_2.wav");

	triWavPlay(wavFile);
	
	sceKernelDelayThread(1*1000*1000);

	triWavStop(wavFile);

	sceKernelDelayThread(1*1000*1000);
	
	triWavSetLoop(wavFile, 1);

	triWavPlay(wavFile);

	sceKernelDelayThread(5*1000*1000);
	
	triWavFree(wavFile);

	triMemoryShutdown();
	
	sceKernelExitGame();
	
	return 0;
}
