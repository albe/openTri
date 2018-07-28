#include <pspkernel.h>
#include "../../triImage.h"
#include "../../triGraphics.h"
#include "../../triTypes.h"
#include "../../triLog.h"
#include "../../triMemory.h"
#include "../../triFont.h"

PSP_MODULE_INFO("triFontTest", 0x0, 1, 1);
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
	triInit( GU_PSM_8888, 1 );
	//triEnable( TRI_VBLANK );
	
	// Colors
	enum colors
	{
		RED =	0xFF0000FF,
		GREEN =	0xFF00FF00,
		BLUE =	0xFFFF0000,
		WHITE =	0xFFFFFFFF
	};

	// Init font library
	triFontInit();

	// Make Debug font mono spaced at width 6
	triFontMakeMono( 0, 6 );
	
	// Load first font, 12 point, 128x128 texture, in VRAM
	triFont *verdana12 = triFontLoad("./verdana.ttf", 12, TRI_FONT_SIZE_POINTS, 128, TRI_FONT_VRAM);

    // Load second font, 20 pixels, 128x128 texture, in VRAM
	triFont *impact20 = triFontLoad("./impact.ttf", 20, TRI_FONT_SIZE_PIXELS, 128, TRI_FONT_VRAM);

	// Make sure the fonts are loaded
	if(!verdana12 || !impact20)
		sceKernelExitGame();

	//triFontSaveTRF( verdana12, "verdana8.trf" );
	while (isrunning)
	{
		triClear( 0 );

		// Activate our first font
		triFontActivate(verdana12);

		// Draw our character set using the first font
		triFontPrint(100, 50, RED, " .,!?:;0123456789");
		triFontPrint(100, 70, GREEN, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		triFontPrint(100, 90, BLUE, "abcdefghijklmnopqrstuvwxyz");
		triFontPrint(100, 110, WHITE, "@#$%^&*+-()[]{}<>/\\|~`\"");

		// Activate the internal debug font
		triFontActivate(0);
		triFontPrintf(2, 2, WHITE, "FPS: %.2f - FPS Max: %.2f - FPS Min: %.2f", triFps(), triFpsMax(), triFpsMin());

		// Activate our second font
		triFontActivate(impact20);

		// Draw our character set using the second font
		triFontPrint(100, 150, RED, " .,!?:;0123456789");
		triFontPrint(100, 170, GREEN, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		triFontPrint(100, 190, BLUE, "abcdefghijklmnopqrstuvwxyz");
		triFontPrint(100, 210, WHITE, "@#$%^&*+-()[]{}<>/\\|~`\"");

		triSwapbuffers();
	}

	// Unload our first font
	triFontUnload(verdana12);

	// Unload our second font
	triFontUnload(impact20);

	// Shutdown font library
	triFontShutdown();

	triClose();
	triMemoryShutdown();
	triLogShutdown();
	sceKernelExitGame();
	return 0;
}
