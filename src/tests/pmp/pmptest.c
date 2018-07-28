#include <pspkernel.h>
#include <pspsdk.h>
#include <pspgu.h>
#include <stdio.h>
#include <malloc.h>
#include "../../pmp/pmp.h"


PSP_MODULE_INFO("triPMPTest", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
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
	
	char* result = pmp_init();
	if (result != 0)
	{
		fprintf(stderr, "ERROR: %s\n", result);
		sceKernelExitGame();
	}
	fprintf(stderr, "pmp_init() succeeded.\n");

	// playback can always be interrupted by pressing START
	result = pmp_play( "test.pmp", 1, GU_PSM_4444 );
	if (result != 0)
	{
		fprintf(stderr, "ERROR: %s\n", result);
		sceKernelExitGame();
	}
	
	int fd;
	
	while (isrunning && pmp_isplaying())
	{
		sceKernelDelayThread(1000*1000); // Wait 1 seconds.
		fd = sceIoOpen("ms0:/test.mp3", PSP_O_RDONLY, 0777);
		int filesize = sceIoLseek32(fd, 0, PSP_SEEK_END);
		sceIoLseek32(fd, 0, PSP_SEEK_SET);
		unsigned char *buffer = (unsigned char *)malloc(filesize);
		
		sceIoRead(fd, buffer, filesize);
		fprintf(stderr, "Done reading\n");
		sceIoClose(fd);
		
		fd = sceIoOpen("ms0:/test2.mp3", PSP_O_WRONLY|PSP_O_CREAT, 0777);
		sceIoWrite(fd, buffer, filesize);
		fprintf(stderr, "Done writing\n");
		sceIoClose(fd);
		
		free(buffer);
	}
	pmp_stop();

	sceKernelExitGame();
	return 0;
}
