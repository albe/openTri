#include <pspctrl.h>
#include <pspdebug.h>
#include <pspkernel.h>

#include "triTypes.h"

triVoid triError(const triChar *errmsg)
{
	SceCtrlData pad;
        pspDebugScreenInit();
	pspDebugScreenSetXY(0, 0);
	pspDebugScreenPrintf("%s\n\nPress START to exit.", errmsg);

	while(!(pad.Buttons & PSP_CTRL_START))
		sceCtrlPeekBufferPositive(&pad, 1);

		sceKernelExitGame();
}
