#include <stdlib.h>
#include <string.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <psprtc.h>
#include <math.h>
#include "../../triImage.h"
#include "../../triGraphics.h"
#include "../../tri3d.h"
#include "../../triTypes.h"
#include "../../triLog.h"
#include "../../triMemory.h"
#include "../../triInput.h"
#include "../../triCamera.h"
#include "../../triVMath_vfpu.h"
#include "../../triTexman.h"
#include "../../triVAlloc.h"
#include "../../triParticle.h"
#include "../../triTimer.h"


PSP_MODULE_INFO("triParticleTest", 0x0, 1, 1);
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

#define SCE_PI				3.1415926535897932384626433832795028841971693993751058209749445923f
#define SCE_RAD_TO_DEG(X)	((X) * (180.0f / SCE_PI))
#define SCE_DEG_TO_RAD(X)	((X) * (SCE_PI / 180.0f))

// Need to be aligned on 16bytes and this isn't possible on stack :(
triVec4f	triXAxis	= { 1.0f, 0.0f, 0.0f, 0.0f };
triVec4f	triYAxis	= { 0.0f, 1.0f, 0.0f, 0.0f };
triVec4f	triZAxis	= { 0.0f, 0.0f, 1.0f, 0.0f };

triParticleSystem* partSys;
triParticleEmitter* partEm;


/* Simple thread */
int main(int argc, char **argv)
{
	SetupCallbacks();
	
	pspDebugScreenInit();
	
	triLogInit();
	triInit( GU_PSM_8888, 1 );
	tri3dInit();
	triInputInit();
	
	// FIXME: This is not the right way to do as it will cause the particle manager to try and free pointers from inside the array
	partSys = triMalloc( sizeof(triParticleSystem)*4 );
	partEm = triMalloc( sizeof(triParticleEmitter)*4 );
	
	triS32 PsID0 = triParticleManagerLoadScript( "watersprinkle.txt" );
	
	
	//triParticleSystemConstructor( &partSys[0] );
	triParticleSystemConstructor( &partSys[1] );
	triParticleSystemConstructor( &partSys[2] );
	partSys[2].renderMode = GU_TRIANGLES;
	partSys[2].blendMode = TRI_BLEND_MODE_ALPHA_SUB;
	//triParticleSystemConstructor( &partSys[3] );
	//partSys[3].renderMode = GU_TRIANGLES;

	//triParticleEmitterConstructor( &partEm[0], TRI_EMITTER_SPRINKLE );
	triParticleEmitterConstructor( &partEm[1], TRI_EMITTER_FIRE );
	triParticleEmitterConstructor( &partEm[2], TRI_EMITTER_SMOKE );
	//triParticleEmitterConstructor( &partEm[3], TRI_EMITTER_EXPLOSION );

	partEm[1].pos.z = -10.0f;
	partEm[1].pos.x = 5.0f;
	partEm[1].binding = 0.5f;		// particles are bound to the emitter
	partEm[1].loosen = 0.95f;		// particles loosen from the emitter with age
	partEm[1].life = 2.0f;
	partEm[1].growth = -0.7f/3.0f;
	partEm[1].max = 512;
	partEm[1].rate = partEm[1].max/(partEm[1].life*1.2);
	partEm[1].lastpos = partEm[1].pos;
	partEm[2].pos.z -= 3.0;
	partEm[2].pos.x += 2.0;
	//partEm[3].pos.z -= 5.0;
	
	partSys[1].textureID = triTextureLoad( "sprite.tga" );
	partSys[2].textureID = triTextureLoad( "smoke.tga" );
	
	
	//triParticleManagerAdd( &partSys[0], &partEm[0] );
	triParticleManagerAdd( &partSys[1], &partEm[1] );
	triParticleManagerAdd( &partSys[2], &partEm[2] );
	//triParticleManagerAdd( &partSys[3], &partEm[3] );
	
	triCamera* cam = triCameraCreate( 0.0f, 0.0f, -6.0f );


	tri3dPerspective( 45.0f );
	sceGuFrontFace(GU_CW);
	triClear( 0x330000 );
	triSync();
	triEnable(TRI_VBLANK);
	
	triTimer* frametimer = triTimerCreate();
	triTimer* demotimer = triTimerCreate();
	
	#define SPEED 0.05f
	
	while (isrunning)
	{
		tri3dClear( 1,0,1 );

		triInputUpdate ();

		if (triInputHeld (PSP_CTRL_UP))
		{
			triCameraRotate( cam, SCE_DEG_TO_RAD(3.2f), &triXAxis );
		}
		if (triInputHeld (PSP_CTRL_DOWN))
		{
			triCameraRotate( cam, SCE_DEG_TO_RAD(-3.2f), &triXAxis );
		}
		if (triInputHeld (PSP_CTRL_LEFT))
		{
			triCameraRotate( cam, SCE_DEG_TO_RAD(3.2f), &triYAxis );
		}
		if (triInputHeld (PSP_CTRL_RIGHT))
		{
			triCameraRotate( cam, SCE_DEG_TO_RAD(-3.2f), &triYAxis );
		}
		if (triInputHeld (PSP_CTRL_LTRIGGER))
		{
			triCameraMove( cam, 0.0f, 0.0f, SPEED );
		}
		if (triInputHeld (PSP_CTRL_RTRIGGER))
		{
			triCameraMove( cam, 0.0f, 0.0f, -SPEED );
		}

		triCameraUpdateMatrix( cam );
		sceGumMatrixMode	(GU_MODEL);
		sceGumLoadIdentity	();
		triTimerUpdate(frametimer);
		triParticleManagerUpdateRender( cam, triTimerGetDeltaTime(frametimer) );

		//triVblank();
		
		partEm[1].pos.z = sinf( 30*triTimerPeekDeltaTime(demotimer)*SCE_PI/180.0f )*5.0f - 10.0f;
		partEm[1].pos.x = cosf( 30*triTimerPeekDeltaTime(demotimer)*SCE_PI/180.0f )*5.0f;
		
		
		/*if (partEm[3].age>=partEm[3].lifetime && (rand()%100<5))
		{
			triParticleEmitterConstructor( &partEm[3], TRI_EMITTER_EXPLOSION );
			partEm[3].pos.x = 3.0f * (rand()%65537 - 32768)/32768.0f;
			partEm[3].pos.y = 3.0f * (rand()%65537 - 32768)/32768.0f;
			partEm[3].pos.z = -15.0f + 3.0f * (rand()%65537 - 32768)/32768.0f;
		}*/
		pspDebugScreenSetOffset((triS32)vrelptr(triFramebuffer));
		triSwapbuffers();
		
		pspDebugScreenSetXY(0,0);
		pspDebugScreenPrintf( "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin() );
		pspDebugScreenSetXY(0,1);
		pspDebugScreenPrintf( "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad() );
	}


	triTextureUnload( partSys[1].textureID );
	triTextureUnload( partSys[2].textureID );
	triParticleManagerDestroy();
	triFree( partSys );
	triFree( partEm );
	
	triTimerFree(frametimer);
	triTimerFree(demotimer);
	triFree( cam );
	triInputShutdown();
	triClose();
	tri3dClose();
	triMemoryShutdown();
	sceKernelExitGame();

	return 0;
}
