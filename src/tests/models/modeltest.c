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
#include "../../triModel.h"
#include "../../triInput.h"
#include "../../triCamera.h"
#include "../../triVMath_vfpu.h"
#include "../../triTexman.h"
#include "../../triVAlloc.h"
#include "../../triTimer.h"
#include "../../triFont.h"


PSP_MODULE_INFO("triModelTest", 0x0, 1, 1);
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


/* Simple thread */
int main(int argc, char **argv)
{
	SetupCallbacks();
	
	triLogInit();
	triFontInit();
	triInit( GU_PSM_5551, 1 );
	tri3dInit();
	triInputInit();
	triModelManagerInit();
	
	triFontMakeMono( 0, 7 );
	
	triModel Model[3];

/*
	// Convert textures to .tri format
	triImage* img;
	img = triImageLoadTga( "corvette.tga" );
	triImageSaveTri( "corvette.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );

	img = triImageLoadTga( "pickup.tga" );
	triImageSaveTri( "pickup.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );

	img = triImageLoadTga( "van.tga" );
	triImageSaveTri( "van.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );

	img = triImageLoadTga( "tire.tga" );
	triImageSaveTri( "tire.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );
*/
/*
	// Load old .trim mesh file and save as new .trim Model file
	Model[0].numParts		= 5;
	Model[0].parts			= triMalloc (sizeof(triModelPart) * Model[0].numParts);
	memset (Model[0].parts, 0, sizeof(triModelPart) * Model[0].numParts);

	triVec3Set ((triVec3f*)&Model[0].pos, 0.0f, -1.55f, 0.0f);

	Model[0].parts[0].mesh	= triMeshLoadTrim ( "corvette.trim_old", "corvette.tri" );
	Model[0].parts[1].mesh	= triMeshLoadTrim ( "tire_left.trim_old", "tire.tri" );
	Model[0].parts[2].mesh	= triMeshLoadTrim ( "tire_right.trim_old", "tire.tri" );
	Model[0].parts[3].mesh	= Model[0].parts[1].mesh;
	Model[0].parts[4].mesh	= Model[0].parts[2].mesh;

	triVec3Set ((triVec3f*)&Model[0].parts[0].pos,  0.0f, 2.4f,  0.00f);
	triVec3Set ((triVec3f*)&Model[0].parts[1].pos,  1.6f, 1.0f,  1.90f);
	triVec3Set ((triVec3f*)&Model[0].parts[2].pos, -1.6f, 1.0f,  1.90f);
	triVec3Set ((triVec3f*)&Model[0].parts[3].pos,  1.6f, 1.0f, -2.00f);
	triVec3Set ((triVec3f*)&Model[0].parts[4].pos, -1.6f, 1.0f, -2.00f);

//////////////////////////////////////////////////////////////////////////

	Model[1].numParts		= 5;
	Model[1].parts			= triMalloc (sizeof(triModelPart) * Model[1].numParts);
	memset (Model[1].parts, 0, sizeof(triModelPart) * Model[1].numParts);

	triVec3Set ((triVec3f*)&Model[1].pos, 0.0f, -1.55f, 0.0f);

	Model[1].parts[0].mesh	= triMeshLoadTrim ( "pickup.trim_old", "pickup.tri" );
	Model[1].parts[1].mesh	= Model[0].parts[1].mesh;
	Model[1].parts[2].mesh	= Model[0].parts[2].mesh;
	Model[1].parts[3].mesh	= Model[0].parts[1].mesh;
	Model[1].parts[4].mesh	= Model[0].parts[2].mesh;

	triVec3Set ((triVec3f*)&Model[1].parts[0].pos,  0.0f, 2.4f,  0.00f);
	triVec3Set ((triVec3f*)&Model[1].parts[1].pos,  1.6f, 1.0f,  2.35f);
	triVec3Set ((triVec3f*)&Model[1].parts[2].pos, -1.6f, 1.0f,  2.35f);
	triVec3Set ((triVec3f*)&Model[1].parts[3].pos,  1.6f, 1.0f, -1.95f);
	triVec3Set ((triVec3f*)&Model[1].parts[4].pos, -1.6f, 1.0f, -1.95f);

//////////////////////////////////////////////////////////////////////////

	Model[2].numParts		= 5;
	Model[2].parts			= triMalloc (sizeof(triModelPart) * Model[2].numParts);
	memset (Model[2].parts, 0, sizeof(triModelPart) * Model[2].numParts);

	triVec3Set ((triVec3f*)&Model[2].pos, 0.0f, -1.55f, 0.0f);

	Model[2].parts[0].mesh	= triMeshLoadTrim ( "van.trim_old", "van.tri" );
	Model[2].parts[1].mesh	= Model[0].parts[1].mesh;
	Model[2].parts[2].mesh	= Model[0].parts[2].mesh;
	Model[2].parts[3].mesh	= Model[0].parts[1].mesh;
	Model[2].parts[4].mesh	= Model[0].parts[2].mesh;

	triVec3Set ((triVec3f*)&Model[2].parts[0].pos,  0.0f, 2.4f,  0.00f);
	triVec3Set ((triVec3f*)&Model[2].parts[1].pos,  1.6f, 1.0f,  2.05f);
	triVec3Set ((triVec3f*)&Model[2].parts[2].pos, -1.6f, 1.0f,  2.05f);
	triVec3Set ((triVec3f*)&Model[2].parts[3].pos,  1.6f, 1.0f, -1.70f);
	triVec3Set ((triVec3f*)&Model[2].parts[4].pos, -1.6f, 1.0f, -1.70f);

//////////////////////////////////////////////////////////////////////////

	Model[0].next	= &Model[1];
	Model[1].next	= &Model[2];

//////////////////////////////////////////////////////////////////////////
*/
//	triModelsSaveTrim( "cars.trim", Model, 1, TRI_MESH_FLAGS_GZIP );
//	triModelsSaveTrim( "cars2.trim", Model, 1, TRI_MESH_FLAGS_GZIP|TRI_MESH_FLAGS_SAVE_IMAGE );
//	triModelsSaveTrim( "cars3.trim", Model, 3, TRI_MESH_FLAGS_GZIP|TRI_MESH_FLAGS_SAVE_IMAGE );
//	triModelsSaveTrim( "corvette.trim", &Model[0], 1, TRI_MESH_FLAGS_GZIP|TRI_MESH_FLAGS_SAVE_IMAGE );
//	triModelsSaveTrim( "pickup.trim", &Model[1], 1, TRI_MESH_FLAGS_GZIP|TRI_MESH_FLAGS_SAVE_IMAGE );
//	triModelsSaveTrim( "van.trim", &Model[2], 1, TRI_MESH_FLAGS_GZIP|TRI_MESH_FLAGS_SAVE_IMAGE );

	// Load .trim Model file
	triModel* pModel;
	triS32 nModels;
	pModel = triModelsLoadTrim( "cars3.trim", &nModels );
	triLogPrint("Loaded %i Model(s) from cars3.trim...\n", nModels);
	Model[0] = *(pModel);
	Model[1] = *(pModel->next);
	Model[2] = *(pModel->next->next);
	
	//triMeshOptimize( Model[0].parts[1].mesh, TRI_VERTFASTUVNF_FORMAT );
	// Comment this line to test speed improvement of optimized mesh
	//triModelOptimize( &Model[0], TRI_VERTFASTUVNF_FORMAT );

	
	// Uncomment this line to test automatic mipmap generation (might take a while)
	/*triTimer* mmTimer = triTimerCreate();
	triTextureBuildMipmaps( Model[0].parts[0].mesh->texID, 4 );
	triLogPrint("Mipmap generation took %.3fs\n", triTimerPeekDeltaTime(mmTimer));
	triTimerFree(mmTimer);*/
	
	
/*	triImage* img;	
	img = triTextureGet( Model[0].parts[0].mesh->texID );
	triImageSaveTri( "corvette_mip.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );

	triTextureBuildMipmaps( Model[1].parts[0].mesh->texID, 4 );
	img = triTextureGet( Model[1].parts[0].mesh->texID );
	triImageSaveTri( "pickup_mip.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );
	
	triTextureBuildMipmaps( Model[2].parts[0].mesh->texID, 4 );
	img = triTextureGet( Model[2].parts[0].mesh->texID );
	triImageSaveTri( "van_mip.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );

	triTextureBuildMipmaps( Model[0].parts[1].mesh->texID, 3 );
	img = triTextureGet( Model[0].parts[1].mesh->texID );
	triImageSaveTri( "tire_mip.tri", img, TRI_IMG_FLAGS_GZIP|TRI_IMG_FLAGS_SWIZZLE );
	triImageFree( img );
	*/
	
	/*pModel = triModelsLoadTrim( "corvette.trim", &nModels );
	triLogPrint("Loaded %i Model(s) from cars.trim...\n", nModels);
	Model[0] = *(pModel);

	pModel = triModelsLoadTrim( "pickup.trim", &nModels );
	triLogPrint("Loaded %i Model(s) from cars.trim...\n", nModels);
	Model[1] = *(pModel);

	pModel = triModelsLoadTrim( "van.trim", &nModels );
	triLogPrint("Loaded %i Model(s) from cars.trim...\n", nModels);
	Model[2] = *(pModel);*/

	triCamera* cam = triCameraCreate( 0.0f, 0.0f, -10.0f );

	triFloat	angle	= 0.0f;

	triFloat	surface[4][8] = { { 0.0f, 0.4f, 0.1f, 0.8f, 1.0f, 0.3f, -0.3f, -0.1f } };
	triSInt		sIdx[4] = { 0, 2, 3, 1 };
	
	triU32		i;
	triVec3f	Rot;
	triVec2f*	pStick	= triInputGetStick ();

	tri3dPerspective( 45.0f );
	//sceGuFrontFace		(GU_CW);
	triClear( 0xFF0000 );

	//sceGuEnable(GU_LIGHTING);
	//sceGuEnable(GU_LIGHT0);

	triFloat val = 0.0f;
/*
	#define LIGHT_DISTANCE 5.0f
	ScePspFVector3 pos = { 1 * LIGHT_DISTANCE, 3.0, 0 * LIGHT_DISTANCE };
	sceGuLight(0,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos);
	sceGuLightColor(0,GU_DIFFUSE,0x22220000);
	sceGuLightColor(0,GU_SPECULAR,0xffffffff);
	sceGuLightAtt(0,0.0f,1.0f,0.0f);
	sceGuSpecular(12.0f);
	sceGuAmbient(0x00222222);
*/
	
	triU32 frames = 0;
	triS32 model = 0;
	triS32 enableAA = 1;
	triEnable(TRI_PSEUDO_FSAA);

	while (isrunning)
	{
		tri3dClear( 1,0,1 );
		triInputUpdate ();

		if (triInputPressed (PSP_CTRL_LTRIGGER))
		{
			if (--model < 0) model = 2;
		}
		if (triInputPressed (PSP_CTRL_RTRIGGER))
		{
			if (++model > 2) model = 0;
		}
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
		
		if (triInputHeld (PSP_CTRL_TRIANGLE))
		{
			triCameraMove( cam, 0.0f, 0.0f, -1.0f );
		}
		if (triInputHeld (PSP_CTRL_CROSS))
		{
			triCameraMove( cam, 0.0f, 0.0f, 1.0f );
		}
		if (triInputHeld (PSP_CTRL_SQUARE))
		{
			triCameraMove( cam, 0.0f, -1.0f, 0.0f );
		}
		if (triInputHeld (PSP_CTRL_CIRCLE))
		{
			triCameraMove( cam, 0.0f, 1.0f, 0.0f );
		}
		
		if (triInputPressed (PSP_CTRL_START))
		{
			enableAA = (enableAA+1)%2;
			if (enableAA)
				triEnable(TRI_PSEUDO_FSAA);
			else
				triDisable(TRI_PSEUDO_FSAA);
		}
/*
		ScePspFVector3 pos = { cosf(val * (GU_PI/180)) * LIGHT_DISTANCE, 3.0, sinf(val * (GU_PI/180)) * LIGHT_DISTANCE };
		sceGuLight(0,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos);
		sceGuLightColor(0,GU_DIFFUSE,0x22220000);
		sceGuLightColor(0,GU_SPECULAR,0xffffffff);
		sceGuLightAtt(0,0.0f,1.0f,0.0f);
		sceGuSpecular(12.0f);
		sceGuAmbient(0x00222222);
		val += 1.0f;
		if (val>=360.0f) val -= 360.0f;
*/
		Model[model].parts[1].rot.y	=  pStick->x * 45.0f;
		Model[model].parts[2].rot.y	=  pStick->x * 45.0f;
		Model[model].parts[3].rot.y	= -pStick->x * 22.5f;
		Model[model].parts[4].rot.y	= -pStick->x * 22.5f;
		Model[model].parts[1].rot.x	+= 5.0f;
		Model[model].parts[2].rot.x	+= 5.0f;
		Model[model].parts[3].rot.x	+= 5.0f;
		Model[model].parts[4].rot.x	+= 5.0f;

		{
			triCameraUpdateMatrix( cam );
			sceGumMatrixMode	(GU_MODEL);
			sceGumLoadIdentity	();
		}

		//triModelRender( &Model[model] );
		/*
		sceGumPushMatrix ();

		triVec3Set (&Rot, SCE_DEG_TO_RAD (Model[model].rot.x), SCE_DEG_TO_RAD (Model[model].rot.y), SCE_DEG_TO_RAD (Model[model].rot.z));

		sceGumTranslate ((ScePspFVector3*)&Model[model].pos);
		sceGumRotateXYZ ((ScePspFVector3*)&Rot);

		sceGuDisable(GU_BLEND);
		sceGuEnable(GU_TEXTURE_2D);
		for (i=0; i<Model[model].numParts; i++)
		{
			sceGumPushMatrix ();

			triVec3Set (&Rot, SCE_DEG_TO_RAD (Model[model].parts[i].rot.x), SCE_DEG_TO_RAD (Model[model].parts[i].rot.y), SCE_DEG_TO_RAD (Model[model].parts[i].rot.z));

			if (i>0)
			{
				triVec3f bump = { 0.0f, 0.1f*surface[0][sIdx[i-1]], 0.0f };
				sIdx[i-1] = (sIdx[i-1] + 1) % 8;
				sceGumTranslate((ScePspFVector3*)&bump);
			}

			sceGumTranslate ((ScePspFVector3*)&Model[model].parts[i].pos);
			sceGumRotateZYX ((ScePspFVector3*)&Rot);

			triTextureBind( Model[model].parts[i].mesh->texID );
			sceGumDrawArray (GU_TRIANGLES, Model[model].parts[i].mesh->vertFormat | GU_TRANSFORM_3D, Model[model].parts[i].mesh->numVerts, 0, Model[model].parts[i].mesh->verts);

			sceGumPopMatrix ();
		}

		sceGumPopMatrix ();
		*/
		frames++;
		/*
		triFontActivate(0);
		triFontPrintf( 0,0, 0xFFFFFFFF, "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin() );
		triFontPrintf( 0,10, 0xFFFFFFFF, "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad() );
		triFontPrintf( 0,20, 0xFFFFFFFF, "AA mode: %s", enableAA==0?"off":"on" );
		triFontPrintf( 0,30, 0xFFFFFFFF, "VRAM: %i - largest: %i", triVMemavail()/1024, triVLargestblock()/1024 );
		*/
		triSwapbuffers();
	}
	
	triLogPrint( "FPS MAX: %.2f - MIN: %.2f\n", triFpsMax(), triFpsMin() );
	
	triModelManagerFreeAll();
	//triMeshFree( Model.parts[0].mesh );

	//triFree ( Model.parts );

	triFree( cam );
	triInputShutdown();
	triClose();
	tri3dClose();
	triMemoryShutdown();
	sceKernelExitGame();

	return 0;
}
