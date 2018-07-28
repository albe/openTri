#include <pspkernel.h>
#include "../../triImage.h"
#include "../../triGraphics.h"
#include "../../triTypes.h"
#include "../../triLog.h"
#include "../../triMemory.h"
#include "../../triVAlloc.h"
#include "../../triFont.h"
#include "../../triInput.h"

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
	triFontInit();
	triMemoryInit();
	triInputInit();
	triInit( GU_PSM_8888, 1 );
	
	// Make Debug font mono spaced at width 7
	triFontSetMono( 0, 7 );
	
	/*
	triImage* triSprite = triImageLoadTga( "sprite.tga" );
	if (triSprite==0)
	{
		triLogError("Error loading sprite.tga!\n");
		sceKernelExitGame();
		return 0;
	}
	triS32 i = 0;
	for (;i<256;i++)
		triImagePaletteSet( triSprite, i, i, i, i, i );
	*/
	/*
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
	triSInt bigMode = 0;
	triFloat x = 0.f, y = 0.f;
	triFloat dx = 2.f, dy = 1.0f;
	
	triFloat angle = 0.0f;
	triEnable(TRI_VBLANK);	// Enable vsync
	while (isrunning)
	{
		if (bigMode == 0)
		{
			// Blit a moving background in spritemode
			triBltSprite( 0.f, 0.f, x, y, triBig );
			x += dx;
			y += dy;
			if (x>=(triBig->width-480) || x<=0) dx = -dx;
			if (y>=(triBig->height-272) || y<=0) dy = -dy;
		}
		else
		if (bigMode == 1)
		{  
			triClear( 0xFFFFFFFF );
			// Draw centered background with changing scale factor
			triDrawImageCenterScaled( 240, 141, sinf(angle) + 1.5f, triBig );
		}
		
		// Draw some shapes
		triDrawRect( 64, 64, 64, 64, 0xff000000 );
		triDrawRectRotate( 64, 64, 64, 64, 0xff0000ff, angle );
		triDrawRegPolyGrad( 420, 150, 32, 0xff0000ff, 0xffff0000, 5, 360.f-angle );
		triDrawStarGrad( 64, 230, 16, 32, 0xffffffff, 0xff00ffff, 5, angle );
		
		// Draw an antialiased circle...
		triEnable(TRI_AALINE);
		triDrawCircleOutline( 320, 230, 32, 0xff00ffff );
		triDisable(TRI_AALINE);	// Don't keep that enabled for filled primitives... makes everything go bogus
		triDrawCircle( 320, 230, 32, 0xff00ffff );
		
		// Draw some images
		triDrawImageRotate2( 196, 128, angle, triTri );
	 	triDrawImageAnimation( 280, 32, triAni );

	 	triImageBlend(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);
	 	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	 	//triDrawImage( 164, 164, 32, 32, 0, 0, 32, 32, triSprite );

		triImageAnimationUpdate( triAni );
		
		triFontActivate(0);
		triFontPrintf( 1, 1, 0xff000000, "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin() );
		triFontPrintf( 1,11, 0xff000000, "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad() );
		triFontPrintf( 1,21, 0xff000000, "VRAM: %iKb - largest: %iKb", triVMemavail()/1024, triVLargestblock()/1024 );
		triFontPrintf( 0, 0, 0xffffffff, "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin() );
		triFontPrintf( 0,10, 0xffffffff, "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad() );
		triFontPrintf( 0,20, 0xffffffff, "VRAM: %iKb - largest: %iKb", triVMemavail()/1024, triVLargestblock()/1024 );
		angle += 0.05f;
		if (angle>=360.f) angle-=360.f;
		
		triInputUpdate();
		if (triInputPressed(PSP_CTRL_CROSS))
		{
			bigMode = (bigMode + 1)%2;
		}
		
		triSwapbuffers();
	}

	triImageAnimationFree( triAni );
	triImageFree( triTri );
	triImageFree( triBig );
	triClose();
	triFontShutdown();
	triMemoryCheck();
	triMemoryShutdown();
	sceKernelExitGame();
	return 0;
}
