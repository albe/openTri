#include <pspkernel.h>
#include "../../triImage.h"
#include "../../triGraphics.h"
#include "../../triTypes.h"
#include "../../triLog.h"
#include "../../triMemory.h"
#include "../../triVAlloc.h"

PSP_MODULE_INFO("triGfxTest", 0x0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);
TRI_DLIST_SIZE_KB(64);

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
	triMemoryInit();
	triInit( GU_PSM_8888, 1 );
	pspDebugScreenInit();
	
	/*triImage* triSprite = triImageLoadTga( "sprite.tga" );
	if (triSprite==0)
	{
		triLogError("Error loading sprite.tga!\n");
		sceKernelExitGame();
		return 0;
	}
	triImageSaveTri( "sprite_gzip.tri", triSprite, TRI_IMG_FLAGS_GZIP );
	
	//triImageSaveTri( "sprite_rle.tri", triSprite, TRI_IMG_FLAGS_RLE );
	
	triImageSwizzle( triSprite );
	//triImageSaveTri( "sprite_swiz.tri", triSprite, TRI_IMG_FLAGS_SWIZZLE );
	triImageSaveTri( "sprite_swiz_rle.tri", triSprite, TRI_IMG_FLAGS_RLE );
	//triImageSaveTri( "sprite_swiz_rle_gzip.tri", triSprite, TRI_IMG_FLAGS_RLE|TRI_IMG_FLAGS_GZIP );
	triImageSaveTri( "sprite_swiz_gzip.tri", triSprite, TRI_IMG_FLAGS_GZIP );
	triImageFree( triSprite );
	*/
	
	triImage* triTri = triImageLoad( "sprite_gzip.tri", 0 );
	if (triTri==0)
	{
		triLogError("Error loading sprite_gzip.tri!\n");
		sceKernelExitGame();
		return 0;
	}
	triImageToVRAM( triTri );
	
	/*
	triImage* triAniImg = triImageLoadTga( "ani.tga" );
	if (triAniImg==0)
	{
		triLogError("Error loading ani.tga!\n");
		sceKernelExitGame();
		return 0;
	}*/
	triImageAnimation* triAni = triImageAnimationFromSheetTga( "ani.tga", 40, 40, 5, 1, 250 );
	//triImageAnimation* triAni = triImageAnimationFromSheet( triAniImg, 40, 40, 5, 1, 250 );
	if (triAni==0)
	{
		triLogError("Error creating triImageAnimation!\n");
		sceKernelExitGame();
		return 0;
	}
	triImageAnimationStart( triAni );
	
	triImage* triBig = triImageLoad( "wallpaper.png",0 );
	if (triBig==0)
	{
		triLogError("Error loading wallpaper.png!\n");
		sceKernelExitGame();
		return 0;
	}
	triLogPrint("Big Image info:\n");
	triLogPrint("width: %i\nheight: %i\n", triBig->width, triBig->height);
	triLogPrint("stride: %i\ntex_height: %i\n", triBig->stride, triBig->tex_height);
	triLogPrint("format: %X (%i bits)\n", triBig->format, triBig->bits);
	triLogPrint("size: %i\n", triBig->size);
	triLogPrint("swizzled: %i\n", triBig->swizzled);
	/*triImage* triBig = triImageLoadRaw( "wallpaper.raw" );
	if (triBig==0)
	{
		triLogError("Error loading wallpaper.raw!\n");
		sceKernelExitGame();
		return 0;
	}
	
	triBig->bits = 32;
	triBig->format = GU_PSM_8888;
	triBig->width = 864;
	triBig->height = 648;
	triBig->stride = 864;
	triBig->tex_height = 1024;*/
	
	triSpriteMode( 480, 272, 0 );
	triFloat x = 0.f, y = 0.f;
	triFloat dx = 2.f, dy = 1.0f;
	
	triFloat angle = 0.0f;
	triEnable(TRI_PSEUDO_FSAA);
	triEnable(TRI_VBLANK);
	while (isrunning)
	{
		triClear( 0xFF0000 );
		
		triDrawSprite( 0.f, 0.f, x, y, triBig );
		x += dx;
		y += dy;
		if (x>=(triBig->width-480) || x<=0) dx = -dx;
		if (y>=(triBig->height-272) || y<=0) dy = -dy;


		triDrawRectRotate( 64, 64, 64, 64, 0xff0000ff, angle );
		//triDrawRegPolyGrad( 64, 240, 32, 0xffff00ff, 0xff00ffff, 5, angle );
		//triDrawCircleGrad( 64, 240, 32, 0xffff00ff, 0xff00ffff );
		triDrawStarGrad( 64, 240, 16, 32, 0xffffffff, 0xff00ffff, 5, angle );
		triDrawImageRotate2( 196, 128, angle, triTri );
	 	triDrawImageAnimation( 280, 32, triAni );

		triImageAnimationUpdate( triAni );
		triSwapbuffers();
		pspDebugScreenSetOffset((triS32)vrelptr(triFramebuffer2));
		pspDebugScreenSetXY(0,0);
		pspDebugScreenPrintf( "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin() );
		pspDebugScreenSetXY(0,1);
		pspDebugScreenPrintf( "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad() );
		pspDebugScreenSetXY(0,2);
		pspDebugScreenPrintf( "VRAM: %iKb - largest: %iKb", triVMemavail()/1024, triVLargestblock()/1024 );
		angle += 0.05f;
	}

	triImageAnimationFree( triAni );
	triImageFree( triTri );
	triClose();
	triMemoryCheck();
	triMemoryShutdown();
	sceKernelExitGame();
	return 0;
}
