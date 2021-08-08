#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <pspkernel.h>
#include <pspthreadman.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <psprtc.h>
#include <psppower.h>
#include <pspdisplay.h>
#include <pspsdk.h>
#include <psputility.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>

#include "main.h"
//#define VID_CAPTURE 1

#if _PSP_FW_VERSION >= 200
PSP_MODULE_INFO("Battle Arena", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);
//PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(20480);
#else
PSP_MODULE_INFO("Battle Arena", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
#endif

unsigned int __attribute__((aligned(16))) gulist[162144];
ScePspFMatrix4 projection;
void *drawBuffer;
int exitRequest = 0;
Image *splash;
intraFont *iFont[6];

#define SCR_WIDTH 480
#define SCR_HEIGHT 272

int running()
{
	return !exitRequest;
}

int exitCallback(int arg1, int arg2, void *common)
{
	exitRequest = 1;
	return 0;
}

int callbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

int setupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

void drawSplash()
{
	if(!splash) splash=loadPng("data/splashscreen.png");
	if(!splash) {
		printf("Splash not found.\n");
		return;
	}
	sceKernelDcacheWritebackAll();
	sceGuStart(GU_DIRECT,gulist);
	sceGuCopyImage(GU_PSM_8888, 0, 0, splash->imageWidth, splash->imageHeight, splash->textureWidth, splash->data, 0, 0, 512, (Color *)((unsigned int)drawBuffer+0x4000000));
	sceGuFinish();
	sceGuSync(0,0);
	drawBuffer=sceGuSwapBuffers();
	pspDebugScreenSetOffset((int)drawBuffer);
	int i;
	for(i=0;i<60;i++) sceDisplayWaitVblankStart();
	freeImage(splash);
	splash=0;
}

void init()
{
	sceGuStart(GU_DIRECT,gulist);

	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CCW);
	sceGuShadeModel(GU_SMOOTH);
	//sceGuEnable(GU_LINE_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_CLIP_PLANES);
	//sceGuDisable(GU_CLIP_PLANES);

	sceGuTexMode(GU_PSM_8888,0,0,0);
	//sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuTexFunc(GU_TFX_ADD,GU_TCC_RGB);
	//sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexEnvColor(0xffff00);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexScale(1.0f,1.0f);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(45.0f,16.0f/9.0f,2.0f,4000.0f);
	gumLoadIdentity(&projection);
	gumPerspective(&projection,45.0f,16.0f/9.0f,2.0f,4000.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	sceGuDepthRange(65535,0);
	sceGuDepthFunc(GU_GEQUAL);

	sceGuDisable(GU_TEXTURE_2D);
	sceGuDisable(GU_BLEND);
	//sceGuEnable(GU_DEPTH_TEST);
	sceGuDisable(GU_DEPTH_TEST);
	//sceGuDisable(GU_CULL_FACE);
	sceGuDisable(GU_ALPHA_TEST);
	//sceGuDisable(GU_CLIP_PLANES);

	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuEnable(GU_LIGHT1);
	sceGuEnable(GU_LIGHT2);
	//ScePspFVector3 key={-1,-1,-1},fill={1,0,-1},back={0,-1,1};
	ScePspFVector3 key={1,-1,1},fill={-1,0,1},back={1,-1,0};
	sceGuLight( 0, GU_DIRECTIONAL, GU_DIFFUSE_AND_SPECULAR, &key);
	sceGuLightColor( 0, GU_DIFFUSE, 0xffffffff);
	sceGuLightColor( 0, GU_SPECULAR, 0xffffffff);
	sceGuLightAtt(0,0.0f,1.0f,0.0f);
	sceGuLight( 1, GU_DIRECTIONAL, GU_DIFFUSE_AND_SPECULAR, &fill);
	sceGuLightColor( 1, GU_DIFFUSE, 0xff202020);
	sceGuLightColor( 1, GU_SPECULAR, 0xff000000);
	sceGuLightAtt(1,0.0f,1.0f,0.0f);
	sceGuLight( 2, GU_DIRECTIONAL, GU_DIFFUSE_AND_SPECULAR, &back);
	sceGuLightColor( 1, GU_DIFFUSE, 0xff808080);
	sceGuLightColor( 1, GU_SPECULAR, 0xff000000);
	sceGuLightAtt(2,0.0f,1.0f,0.0f);
	sceGuSpecular(12.0f);
	sceGuAmbient(0xff404040);
	sceGuAmbientColor(0xffffffff);
	sceGuFinish();
	sceGuSync(0,0);
}

int app_main(SceSize args,void *argp)
{
    int done;
	SceCtrlData pad;
    SceCtrlData oldpad;
    oldpad.Buttons=0;
    oldpad.Lx=-1;
    oldpad.Ly=-1;
	srand(time(NULL));

	// Register callbacks.
	setupCallbacks();

	pspDebugScreenInit();
	//pspDebugScreenPrintf("Welcome.\n");

    /* Initialize GU */
	sceGuInit();
	resetVRam();

	sceGuStart(GU_DIRECT,gulist);
	sceGuDrawBuffer(GU_PSM_8888,0,512);
	sceGuDispBuffer(480,272,(void *)0x88000,512);
	sceGuDepthBuffer((void *)0x110000,512);
	sceGuFinish();
	sceGuSync(0,0);
	sceGuDisplay(GU_TRUE);
	pspDebugScreenSetOffset((int)drawBuffer);
//	pspDebugScreenPrintf("cleared the screen.\n");

    init();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	drawSplash();
//	drawSplash();
//	pspDebugScreenPrintf("drew the splash.\n");

	Image *title=loadPng("data/title.png");
	sceKernelDcacheWritebackAll();

	if(title) {
		int i;
		for(i=254;i>=0;i-=2) {
			sceGuStart(GU_DIRECT,gulist);
			drawSprite(0,0,480,272,title,0,0);
			drawFilledRect(0,0,480,272,i<<24);
			sceGuFinish();
			sceGuSync(0,0);
			drawBuffer=sceGuSwapBuffers();
			pspDebugScreenSetOffset((int)drawBuffer);
			if(!running()) return 0;
		}
	}
	//int err=0;

	u64 oldTick;
	u64 newTick;
	sceRtcGetCurrentTick(&newTick);

//	printf("Displayed the title (%dx%d; %dx%d)\n",title->imageWidth,title->imageHeight,title->textureWidth,title->textureHeight);

	intraFontInit();
	// FONT_HEADLINE,FONT_BODY,FONT_BODYHIGHLIGHT,FONT_MESSAGE,FONT_SMALL,FONT_SMALLHIGHLIGHT
	iFont[FONT_HEADLINE]=intraFontLoad("flash0:/font/ltn4.pgf",0);	// regular bold
	iFont[FONT_BODY]=intraFontLoad("flash0:/font/ltn0.pgf",0);	// regular
	iFont[FONT_BODYHIGHLIGHT]=intraFontLoad("flash0:/font/ltn2.pgf",0);	// italic
	iFont[FONT_MESSAGE]=intraFontLoad("flash0:/font/ltn1.pgf",0);	// serif
	iFont[FONT_SMALL]=intraFontLoad("flash0:/font/ltn8.pgf",0);	// small
	iFont[FONT_SMALLHIGHLIGHT]=intraFontLoad("flash0:/font/ltn11.pgf",0);	// small italic
	
	//initFastFont();
	initSound();

	//newGame();
	startTitle();

	if(title) {
		int i;
		for(i=0;i<=272;i+=4) {
			sceGuStart(GU_DIRECT,gulist);
			drawSprite(0,0,480,272,title,0,0);
			drawFilledRect(0,0,480,i,255<<24);
			sceGuFinish();
			sceGuSync(0,0);
			drawBuffer=sceGuSwapBuffers();
			pspDebugScreenSetOffset((int)drawBuffer);
			if(!running()) return 0;
		}
		freeImage(title);
		title=0;
	}

	printf("event loop\n");
    done = 0;
	sceCtrlReadBufferPositive(&pad,1);
	sceRtcGetCurrentTick(&newTick);


    while (running())
    {
		int changedButtons;

		oldpad.Buttons=pad.Buttons;
		oldpad.Lx=pad.Lx;
		oldpad.Ly=pad.Ly;
		sceCtrlReadBufferPositive(&pad,1);
		changedButtons=pad.Buttons^oldpad.Buttons;

#define handleJoyChange(from,to) if(changedButtons&from) done+=handleJoy(to,(pad.Buttons&from)==0?1:0)
		handleJoyChange(PSP_CTRL_UP,BT_UP);
		handleJoyChange(PSP_CTRL_DOWN,BT_DOWN);
		handleJoyChange(PSP_CTRL_LEFT,BT_LEFT);
		handleJoyChange(PSP_CTRL_RIGHT,BT_RIGHT);
		handleJoyChange(PSP_CTRL_TRIANGLE,BT_TRIANGLE);
		handleJoyChange(PSP_CTRL_SQUARE,BT_SQUARE);
		handleJoyChange(PSP_CTRL_CIRCLE,BT_CIRCLE);
		handleJoyChange(PSP_CTRL_CROSS,BT_CROSS);
		handleJoyChange(PSP_CTRL_LTRIGGER,BT_LTRIGGER);
		handleJoyChange(PSP_CTRL_RTRIGGER,BT_RTRIGGER);
		handleJoyChange(PSP_CTRL_START,BT_START);
		handleJoyChange(PSP_CTRL_SELECT,BT_SELECT);
		handleJoyChange(PSP_CTRL_HOLD,BT_HOLD);
		if(pad.Buttons & PSP_CTRL_SELECT && changedButtons & PSP_CTRL_SELECT) {
			char buf[64];
			static int cap=1;
			sprintf(buf,"battlearena%d.png",cap++);
			saveImagePng(buf,(Color *)(0x04000000),480,272,512,0);
		}

		if(pad.Lx!=oldpad.Lx) handleAnalog(pad.Lx,pad.Ly);
		if(pad.Ly!=oldpad.Ly) handleAnalog(pad.Lx,pad.Ly);

		/* Draw to screen */
		draw();
//#define VID_CAPTURE
#ifdef VID_CAPTURE
		{
			static int shotNo=0;
			char fname[256];
			shotNo++;
			//sprintf(fname,"cap%04d.tga",shotNo);
			//saveImageTarga(fname,(Color *)(0x04000000+(char *)drawBuffer),480,272,512,0);
			sprintf(fname,"cap%04d.jpg",shotNo);
			saveImageJpeg(fname,(Color *)(0x04000000+(char *)drawBuffer),480,272,512);
		}
#endif

		oldTick=newTick;
		sceRtcGetCurrentTick(&newTick);

		updateSound();

		// Now update the game state.
		int elapsed=(newTick-oldTick)/1000;
#ifdef VID_CAPTURE
		elapsed=1000/30;
#endif
		//scePowerSetClockFrequency(333, 333, 166);
		if(elapsed<1000) update(elapsed);
		else update(1000/60);
		//scePowerSetClockFrequency(222, 222, 166);
		//sceDisplayWaitVblankStart();

		//if(game.mode==GAME_QUIT) break;
    }
    printf("quit requested.\n");
    //gprof_cleanup();
    //printf("wrote gmon.out\n");
    sceGuTerm();

    sceKernelExitGame();
    return 0;
}

int main (int argc, char *argv[])
{
#ifndef NETWORKING
	int rc=app_main(0,0);
	printf("Goodbye\n");
	return rc;
#else
#if _PSP_FW_VERSION >= 200
	int rc=sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	if(rc<0) printf("net common didn't load.\n");
	rc=sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	if(rc<0) printf("inet didn't load.\n");
	rc=app_main(0,0);
	printf("Goodbye\n");
	return rc;
#else
	pspSdkLoadInetModules();
	SceUID thid = sceKernelCreateThread("appmain_thread", app_main, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);
	sceKernelStartThread(thid, 0, NULL);
	sceKernelWaitThreadEnd(thid, 0);
	return 0;
#endif
#endif
}
