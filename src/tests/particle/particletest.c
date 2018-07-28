#include <stdlib.h>
#include <string.h>
#include <pspkernel.h>
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
#include "../../triFont.h"
#include "../../triTimer.h"


PSP_MODULE_INFO("triParticleTest", 0x0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
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

triVec4f	screenPos;
triCamera*	cam;

void renderParticle( triParticleSystem* sys, triParticle* p )
{
	// if we used this function for different particle systems, we could check for which system's particles
	// are currently going to be rendered by looking at sys
	// We don't care now
	
	screenPos = p->pos;
	screenPos.x *= 16.f;
	screenPos.y *= 16.f;
	screenPos.z *= 16.f;
	// First we transform the particle's position into screen coordinates, since we draw a 2D primitive
	triCameraProject( &screenPos, cam, &screenPos );
	screenPos.x *= 480.f;
	screenPos.y *= 272.f;
	triFloat scale = screenPos.z==0?p->size:p->size/screenPos.z;
	triDrawStar( screenPos.x, screenPos.y, 16.f*scale, 32.f*scale, triColor4f2RGBA8888( &p->col ), 5, p->pos.w );
}

/* Simple thread */
int main(int argc, char **argv)
{
	SetupCallbacks();
	
	triLogInit();
	triInit( GU_PSM_8888, 1 );
	tri3dInit();
	triInputInit();
	
	// FIXME: This is not the right way to do as it will cause the particle manager to try and free pointers from inside the array
	partSys = triMalloc( sizeof(triParticleSystem)*4 );
	partEm = triMalloc( sizeof(triParticleEmitter)*4 );
	
	triS32 PsID0 = triParticleManagerLoadScript( "watersprinkle.txt" );
	
	triParticleSystemConstructor( &partSys[0] );
	triParticleSystemConstructor( &partSys[1] );
	triParticleSystemConstructor( &partSys[2] );
	partSys[0].render = renderParticle;
	partSys[2].renderMode = GU_TRIANGLES;
	partSys[2].blendMode = TRI_BLEND_MODE_ALPHA;
	//triParticleSystemConstructor( &partSys[3] );
	//partSys[3].renderMode = GU_TRIANGLES;

	triParticleEmitterConstructor( &partEm[0], TRI_EMITTER_SPRINKLE );
	triParticleEmitterConstructor( &partEm[1], TRI_EMITTER_FIRE );
	triParticleEmitterConstructor( &partEm[2], TRI_EMITTER_SMOKE );
	//triParticleEmitterConstructor( &partEm[3], TRI_EMITTER_EXPLOSION );
	partEm[0].pos.z = -10.0f;
	partEm[0].pos.x = -5.0f;
	partEm[0].velRand.w = 1.f;
	partEm[0].max = 64;
	partEm[1].pos.z = -10.0f;
	partEm[1].pos.x = 5.0f;
	partEm[1].vel.y += 1.5f;
	partEm[1].binding = 0.0f;		// particles are bound to the emitter
	partEm[1].loosen = 0.85f;		// particles loosen from the emitter with age
	partEm[1].life = 0.8f;
	partEm[1].growth = -0.7f/partEm[1].life;
	partEm[1].max = 512;
	partEm[1].rate = partEm[1].max/(partEm[1].life*1.2);
	partEm[1].lastpos = partEm[1].pos;
	partEm[1].vortexRange = 0.6f;
	partEm[1].vortexDirRand = 2.5f;
	partEm[2].pos.z -= 3.0;
	partEm[2].pos.x += 2.0;
	partEm[2].maxVortex = 16;
	partEm[2].rateVortex = 2;
	partEm[2].max = 256;
	partEm[2].life = 10.f;
	partEm[2].lifetime = 0;
	partEm[2].vortexDir = 0.f;
	partEm[2].vortexDirRand = 1.2f;
	partEm[2].vortexRange = 0.5f;
	partEm[2].rate = partEm[2].max/(partEm[2].life);
	//partEm[3].pos.z -= 5.0;
	
	partSys[1].textureID = triTextureLoad( "sprite.tga" );
	partSys[2].textureID = triTextureLoad( "smoke32.tga" );
	
	//triParticleManagerAdd( &partSys[0], &partEm[0] );
	triParticleManagerAdd( &partSys[1], &partEm[1] );
	triParticleManagerAdd( &partSys[2], &partEm[2] );
	//triParticleManagerAdd( &partSys[3], &partEm[3] );
	
	cam = triCameraCreate( 0.0f, 0.0f, -10.0f );

	// Do additional setup in immediate mode
	triBegin();
	tri3dPerspective( 45.0f );
	sceGuFrontFace(GU_CW);
	triClear( 0x330000 );
	triEnable(TRI_VBLANK);
	triEnd();
	//triSync();
	triTimer* frametimer = triTimerCreate();
	triTimer* demotimer = triTimerCreate();
	triS32 vblank = 0;
	
	#define SPEED 0.05f
	
	while (isrunning)
	{
		triBegin();
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
		if (triInputPressed (PSP_CTRL_SELECT))
		{
			vblank ^= 1;
			if (vblank)
				triEnable(TRI_VBLANK);
			else
				triDisable(TRI_VBLANK);
		}
		triCameraUpdateMatrix( cam );
		sceGumMatrixMode	(GU_MODEL);
		sceGumLoadIdentity	();
		triTimerUpdate(frametimer);
		triParticleManagerUpdateRender( cam, triTimerGetDeltaTime(frametimer) );
		
		partEm[1].pos.z = sinf( 30*triTimerPeekDeltaTime(demotimer)*SCE_PI/180.0f )*5.0f - 10.0f;
		partEm[1].pos.x = cosf( 30*triTimerPeekDeltaTime(demotimer)*SCE_PI/180.0f )*5.0f;
		
		triFontActivate(0);
		triFontPrintf( 0, 0, 0xFFFFFFFF, "CAM: dir <%.3f, %.3f, %.3f> pos <%.3f, %.3f, %.3f>\n", cam->dir.x, cam->dir.y, cam->dir.z, cam->pos.x, cam->pos.y, cam->pos.z);
		triFontPrintf( 0,10, 0xFFFFFFFF, "FPS: %.2f - CPU: %.0f GPU: %.0f\n", triFps(), triCPULoad(), triGPULoad());
		/*if (partEm[3].age>=partEm[3].lifetime && (rand()%100<5))
		{
			triParticleEmitterConstructor( &partEm[3], TRI_EMITTER_EXPLOSION );
			partEm[3].pos.x = 3.0f * (rand()%65537 - 32768)/32768.0f;
			partEm[3].pos.y = 3.0f * (rand()%65537 - 32768)/32768.0f;
			partEm[3].pos.z = -15.0f + 3.0f * (rand()%65537 - 32768)/32768.0f;
		}*/
		triEnd();
		triSwapbuffers();
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
