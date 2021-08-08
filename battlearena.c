#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<pspgu.h>
#include<pspgum.h>
#include<psprtc.h>
#include<pspdisplay.h>
#include<pspkernel.h>
#include<psputility.h>
#include "main.h"
#include "ase.h"
#include "particle.h"
#include "duel.h"
#include "map.h"
#include "cutscene.h"

#include "iconpic.h"

enum GameMode { MODE_TITLE,MODE_CUTSCENE,MODE_MENU,MODE_GAMESAVE,MODE_MAP,MODE_TRIANER,MODE_EXITCUTSCENE,MODE_VICTORY,MODE_DUEL,MODE_GAMEOVER } gameMode;
/*enum MotionType { MT_STAND, MT_RUN, MT_ATTACK, MT_PAIN_A, MT_PAIN_B, MT_PAIN_C,
MT_JUMP, MT_FLIP, MT_SALUTE, MT_FALLBACK, MT_WAVE, MT_POINT, MT_CROUCH_STAND, 
MT_CROUCH_WALK, MT_CROUCH_ATTACK, MT_CROUCH_PAIN, MT_CROUCH_DEATH, 
MT_DEATH_FALLBACK, MT_DEATH_FALLFORWARD, MT_DEATH_FALLBACKSLOW, MT_BOOM };*/

const char *animMotionName[]={
	"stand","run","attack","pain_a","pain_b","pain_c",
	"jump","flip","salute","fallback","wave","point","crouch_stand",
	"crouch_walk","crouch_attack","crouch_pain","crouch_death",
	"death_fallback","death_fallforward","death_fallbackslow","boom"
};

struct AnimMotion animMotion[]={
    {   0,  39,  9 },   // STAND
    {  40,  45, 10 },   // RUN
    {  46,  53, 10 },   // ATTACK
    {  54,  57,  7 },   // PAIN_A
    {  58,  61,  7 },   // PAIN_B
    {  62,  65,  7 },   // PAIN_C
    {  66,  71,  7 },   // JUMP
    {  72,  83,  7 },   // FLIP
    {  84,  94,  7 },   // SALUTE
    {  95, 111, 10 },   // FALLBACK
    { 112, 122,  7 },   // WAVE
    { 123, 134,  6 },   // POINT
    { 135, 153, 10 },   // CROUCH_STAND
    { 154, 159,  7 },   // CROUCH_WALK
    { 160, 168, 10 },   // CROUCH_ATTACK
    { 196, 172,  7 },   // CROUCH_PAIN
    { 173, 177,  5 },   // CROUCH_DEATH
    { 178, 183,  7 },   // DEATH_FALLBACK
    { 184, 189,  7 },   // DEATH_FALLFORWARD
    { 190, 197,  7 },   // DEATH_FALLBACKSLOW
    { 198, 198,  5 },   // BOOM
};

struct Message {
	char text[256];
	int timer;
};

struct Terrain;
struct Map;

struct Game {
	struct Team *team;
	struct Duel *duel;
	struct Map *map;
	struct CutScene *cs;

	struct Message message[10];
	int messageCount;
	
	int selected;	// current menu item
	int step;
	float giggle;	// for the menu
	int score;
	int timeLeft;	// in 1/1000ths of a second
} game;
int dx,dy,dz;
//ScePspFVector3 from={0,500,750},to={0,250,0},up={0,1,0};
//ScePspFVector3 from={20,15,10},to={0,0,0},up={0,1,0};
ScePspFVector3 from={-108,62,-66},to={0,44,0},up={0,1,0};
ScePspFMatrix4 view;

void cameraSetFrom(float fx,float fy,float fz)
{
	from.x=fx;
	from.y=fy;
	from.z=fz;
}

void cameraSetFromTo(float fx,float fy,float fz,float tx,float ty,float tz)
{
	cameraSetFrom(fx,fy,fz);
	to.x=tx;
	to.y=ty;
	to.z=tz;
}

void cameraSetFromToUp(float fx,float fy,float fz,float tx,float ty,float tz,float ux,float uy,float uz)
{
	cameraSetFromTo(fx,fy,fz,tx,ty,tz);
	up.x=ux;
	up.y=uy;
	up.z=uz;
}

SceUtilitySavedataParam gameSaveDialog;

int showFps=0;

struct AseScene *aseTitle;

void showFreeMem()
{
	// We allocate all memory, and report the findings.
	void **first=0,*next=0;
	int available=0;
	int biggest=0;
	int size=16*1024*1024;  // Start at 16 meg.
	int chunks=0;
	while( size>1024) {
		first=(void **)malloc(size);
		if(!first) {
			size/=2;
			continue;
		}
		available+=size;
		if(size>biggest) biggest=size;
		chunks++;
		first[0]=next;
		next=first;
	}
	printf("Allocated %d bytes of memory in %d chunks.\n"
	"Biggest chunk of memory was %d\n",available,chunks,biggest);
	while(next) {
	first=(void **)next;
	next=first[0];
	free(first);
	}
}

void startTitle()
{
	gameMode=MODE_TITLE;
	playSong(SONG_INTRO);
	aseTitle=loadAseScene("sushi");
	sceKernelDcacheWritebackAll();
	if(aseTitle) {
		game.timeLeft=aseTitle->fps*(aseTitle->lastFrame+1)+3000;
	}
	intraFontSetStyle(iFont[FONT_BODY],0.8f,GU_RGBA(255,160,0,0xff),GU_RGBA(0,0,0,255),0);
	intraFontSetStyle(iFont[FONT_BODYHIGHLIGHT],0.8f,GU_RGBA(255,160,0,0xff),GU_RGBA(0,0,0,255),0);
	intraFontSetStyle(iFont[FONT_SMALL],0.8f,GU_RGBA(255,255,255,0xff),GU_RGBA(0,0,0,255),0);
	//intraFontSetStyle(iFont[FONT_MESSAGE],0.8f,GU_RGBA(255,255,255,0xff),GU_RGBA(0,0,0,255),0);
	intraFontSetStyle(iFont[FONT_SMALLHIGHLIGHT],0.8f,GU_RGBA(255,255,255,0xff),GU_RGBA(0,0,0,255),0);
	intraFontSetStyle(iFont[FONT_HEADLINE],1.5f,GU_RGBA(0xff,0xd0,0x00,0xff),GU_RGBA(0,0,0,255),0);	
	from.x=7;
	from.y=-7;
	from.z=5;
	to.x=0;
	to.y=0;
	to.z=0;
	up.x=0;
	up.y=0;
	up.z=1;
	game.selected=0;
}

void newGame()
{
	showFreeMem();
	gameMode=MODE_MENU;
	//gameMode=MODE_MAP;
	game.score=0;
	game.timeLeft=0;
	if(game.team) freeTeam(game.team);
	game.team=newTeam();
	game.selected=1;
	if(getItemCount()<11) {
		freeItems();
		loadItems();
	}
	if(!game.map) game.map=initMap();
	if(!game.duel) game.duel=newDuel();
	if(!game.cs) game.cs=initCutScene();
	if(characterCount==0) {
		loadCharacters(0,0);
	}
	game.step=0;
	game.giggle=0;
/*
	from.x=-358;
	from.y=438;
	from.z=-767;
	to.x=0;
	to.y=44;
	to.z=0;
	up.x=0;
	up.y=1;
	up.z=0;
*/
	showFreeMem();
	reportVRam();
	//newParticle(&testEmitter,&testDesc,0,0,0);
}

void addMessage(const char *message,int timer)
{
	if(game.messageCount>9) {		// we don't care about the messages, so just overwrite the first one.
		strcpy(game.message[0].text,message);
		game.message[0].timer=timer;
		return;
	}
	strcpy(game.message[game.messageCount].text,message);
	printf(">>>%s\n",message);
	game.message[game.messageCount++].timer=timer;
}

int getTimeLeft() 
{
	return game.timeLeft;
}

void setTimeLeft(int duration)
{
	game.timeLeft=duration;
}

float getGiggle()
{
	return game.giggle;
}

int getMaxTeamLevel()
{
	int i;
	int max=0;
	for(i=0;i<game.team->actorCount;i++) {
		if(game.team->actor[i].level>max) max=game.team->actor[i].level;
	}
	return max;
}

void update(unsigned long elapsed)
{
	from.x+=dx;
	from.y+=dy;
	from.z+=dz;

	game.timeLeft-=elapsed;
	if(game.timeLeft<0) game.timeLeft=0;
	
	if(gameMode==MODE_TITLE && game.timeLeft==0) {
		freeAseScene(aseTitle);
		aseTitle=0;
		gameMode=MODE_CUTSCENE;
		game.cs=initCutScene();
	} else if(gameMode==MODE_CUTSCENE) {
		int status=updateCutScene(game.cs,elapsed);
		if(status && !game.map) {
			freeCutScene(game.cs);
			game.cs=0;
			gameMode=MODE_MENU;
			game.selected=1;
			newGame();
		} else if(status) {
			gameMode=MODE_DUEL;
			cameraSetFromToUp(-108,62,-66, 0,44,0, 0,1,0);
			game.selected=0;
			if(getItemCount()<11) {
				freeItems();
				loadItems();
			}
		}
	} else if(gameMode==MODE_EXITCUTSCENE) {
		int status=updateCutScene(game.cs,elapsed);
		if(status) {
			gameMode=MODE_VICTORY;
			game.selected=0;
			if(getItemCount()<11) {
				freeItems();
				loadItems();
			}
		}
	}
	if(aseTitle) updateAseSceneMax(aseTitle,elapsed,aseTitle->lastFrame);	
	if(gameMode==MODE_DUEL) {
		DuelState state=updateDuel(game.duel,elapsed);
		if(state!=DS_ACTIVE) {
			if(applyDuelResult(game.map,state) ) {
				addOneTeammate(game.duel);
				//PlaceType pt=getActivePlace(game.map);
				int level=getActiveLevel(game.map);
				int scene=level/12;
				scene=scene*2+2;
				if(scene<=12) {
					gameMode=MODE_EXITCUTSCENE;
					//printf("**** Selecting scene %d (based on level %d)\n",scene,level);
					selectCutScene(game.cs,scene,game.duel);
				} else {
					gameMode=MODE_VICTORY;
				}
			} else {
				gameMode=MODE_MAP;
				game.selected=0;
			}
		}
	} else if(gameMode==MODE_MAP) {
		MapState state=updateMap(game.map,elapsed);
		if(state==MS_DUEL || state==MS_TOURNAMENT) {
			gameMode=MODE_DUEL;
			PlaceType pt=getActivePlace(game.map);
			int level=getActiveLevel(game.map);
			initDuel(game.duel,pt,level,pt==PT_BLACK?DT_TOURNAMENT:DT_ONEONONE,game.team);
			game.selected=0;
			if(pt==PT_BLACK) {
				int scene=level/12;
				scene=scene*2+1;
				//printf("**** Selecting scene %d (based on level %d)\n",scene,level);
				if(scene<=12 && state==MS_TOURNAMENT) {
					gameMode=MODE_CUTSCENE;
					selectCutScene(game.cs,scene,game.duel);
				}
			}
		}
	} else if(gameMode!=MODE_TITLE) {
		updateDuelSuspended(game.duel,elapsed);
	}

	if(game.messageCount>0) {
		game.message[0].timer-=elapsed;
		if(game.message[0].timer<0) {
			game.messageCount--;
			if(game.messageCount>0) {
				memcpy(game.message,game.message+1,sizeof(struct Message)*game.messageCount);
			}
		}
	}

	// for the selected menu item
	game.giggle+=elapsed/1000.0f*GU_PI;
	if(game.giggle>2*GU_PI) game.giggle-=2*GU_PI;
}

void drawTitleText()
{
	float theta=game.timeLeft/1000.0f-2.0f;
	if(theta<0) theta=0;
	const char *text[]={
		"Team Sushi presents",
		"HardHat's Battle Arena",
		"PSP-Hacks.com Homebrew Idol 2"
	};
	const enum FontId fontId[]={
		FONT_BODYHIGHLIGHT,
		FONT_HEADLINE,
		FONT_BODY,
	};
	const int textCount=3;
	int i;
	
	for(i=0;i<textCount;i++) {	
		int w,h=24;
		int j;
		w=(int)intraFontMeasureText(iFont[fontId[i]],text[i]);
		for(j=0;j<4;j++) {
			float radius=(48+(j+i)*24)*theta;
			float x=240+cosf(theta*(1+j*(1+i)))*radius;
			float y=(272-textCount*h)/2+i*48+sinf(theta*(1+j*(1+i)))*radius;
			intraFontPrint(iFont[fontId[i]],x-w/2,y,text[i]);
		}
	}
}

void drawMenu()
{
	const char *name[]={
		"New",
		"Load Game",
		"Credits",
		"Quit"
	};
	int i;
	int width=(int)intraFontMeasureText(iFont[FONT_HEADLINE],"HardHat's Battle Arena");
	intraFontPrint(iFont[FONT_HEADLINE],240-width/2,24,"HardHat's Battle Arena");
	if(game.selected<0) game.selected=3;
	if(game.selected>3) game.selected=0;
	for(i=0;i<4;i++) {
		intraFont *font=iFont[FONT_SMALL];
		int offset=0;
		if(game.selected==i) {
			font=iFont[FONT_SMALLHIGHLIGHT];
			offset=5*cosf(game.giggle);
		}
		intraFontPrint(font,50+offset,50+i*15,name[i]);
	}
}

PspUtilitySavedataListSaveNewData newData;

char titleShow[128] = "New Save";

char nameMultiple[][20] =       // End list with ""
{
 "0000",
 "0001",
 "0002",
 "0003",
 "0004",
 ""
};

char key[] = "QTAK319JQKJ952HA";        // Key to encrypt or decrypt savedata

void initSavedata(SceUtilitySavedataParam *savedata,int mode)
{
        if(savedata->dataBuf) free(savedata->dataBuf);	// so we don't memory leak.
        memset(savedata, 0, sizeof(SceUtilitySavedataParam));
        savedata->base.size = sizeof(SceUtilitySavedataParam);

        savedata->base.language=PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
        savedata->base.buttonSwap=PSP_UTILITY_ACCEPT_CROSS;
        savedata->base.graphicsThread=0x11;
        savedata->base.accessThread=0x13;
        savedata->base.fontThread=0x12;
        savedata->base.soundThread=0x10;

        savedata->mode=(PspUtilitySavedataMode)mode;
        if(mode==PSP_UTILITY_SAVEDATA_LISTSAVE) printf("savedata mode List save\n");
        if(mode==PSP_UTILITY_SAVEDATA_LISTLOAD) printf("savedata mode List load\n");
        savedata->overwrite=1;
        savedata->focus=PSP_UTILITY_SAVEDATA_FOCUS_LATEST; // Set initial focus to the newest file (for loading)

#if _PSP_FW_VERSION >= 200
        strncpy(savedata->key, key, 16);
#endif

        strcpy(savedata->gameName, "HHBA11111");        // First part of the save name, game identifier name
        strcpy(savedata->saveName, "0000");     // Second part of the save name, save identifier name

        // List of multiple names
        savedata->saveNameList = nameMultiple;

        strcpy(savedata->fileName, "DATA.BIN"); // name of the data file

		int databufflen=24;
		databufflen+=mapSaveSize(game.map);
		databufflen+=teamSaveSize(game.team);
		databufflen+=16;	// round up to a multiple of 32.
		if(databufflen<10240) databufflen=10240;
		//databufflen+=trophySaveSize(game.trophy);
		printf("databufflen: %d\n",databufflen);
		
        // Allocate buffers used to store various parts of the save data
        savedata->dataBuf = malloc(databufflen);
        savedata->dataBufSize = databufflen;
        savedata->dataSize = databufflen;
		memset(savedata->dataBuf, 0, databufflen);

        // Set some data
        if (mode == PSP_UTILITY_SAVEDATA_LISTSAVE) {
			memset(savedata->dataBuf, 0, databufflen);
			//strcpy((char *)savedata->dataBuf,"score: 10");
			int mapSize=mapSaveSize(game.map);
			int teamSize=teamSaveSize(game.team);
			((int *)savedata->dataBuf)[0]=mapSize;
			((int *)savedata->dataBuf)[1]=teamSize;
			((int *)savedata->dataBuf)[2]=60;		// for expansion.
			{
				char *buffer=((char *)savedata->dataBuf)+24;
				saveMapBuffer(game.map,buffer,mapSize);
				buffer+=mapSize;
				saveTeam(game.team,buffer,teamSize);
			}
#if 0
			FILE *file=fopen("testsavedata.bin","wb");
			if(file) {
				fwrite(savedata->dataBuf,databufflen,1,file);
				fclose(file);
			}
#endif
			
			strcpy(savedata->sfoParam.title,"HardHat Battle Arena");
			char msg[256];
			int i;
			sprintf(msg,"%d%% done: ",mapCompletePercent(game.map));
			for(i=0;i<game.team->actorCount;i++) {
				if(strlen(msg)+2+strlen(game.team->actor[i].name)+1<128) {
					if(i>0) strcat(msg,", ");
					strcat(msg,game.team->actor[i].name);
				}
			}
			strcpy(savedata->sfoParam.savedataTitle,msg);
			sprintf(msg,"Training centre %d with ",getActivePlace(game.map)+1);
			for(i=0;i<game.team->actorCount;i++) {
				if(i>0) strcat(msg,", ");
				sprintf(msg+strlen(msg),"Level %d Mage",game.team->actor[i].level);
			}
			if(game.team->actorCount==0) strcpy(msg,"New Adventure");
			strcpy(savedata->sfoParam.detail,msg);
			savedata->sfoParam.parentalLevel=1;
			
			// No icon1
			savedata->icon1FileData.buf=NULL;
			savedata->icon1FileData.bufSize=0;
			savedata->icon1FileData.size=0;
			
			savedata->pic1FileData.buf=pic1;
			savedata->pic1FileData.bufSize=size_pic1;
			savedata->pic1FileData.size=size_pic1;			
			savedata->icon0FileData.buf=icon0;
			savedata->icon0FileData.bufSize=size_icon0;
			savedata->icon0FileData.size=size_icon0;
			
			// No snd0
			savedata->snd0FileData.buf=NULL;
			savedata->snd0FileData.bufSize=0;
			savedata->snd0FileData.size=0;
			
			// Set title
			sprintf(titleShow,"Battle Arena %d%% done",mapCompletePercent(game.map));
			newData.title=(char *)titleShow;
			newData.icon0.buf=0;
			newData.icon0.bufSize=0;
			newData.icon0.size=0;
			newData.icon0.unknown=0;
			
			// Set new data
			savedata->newData=&newData;
			
			savedata->focus=PSP_UTILITY_SAVEDATA_FOCUS_FIRSTEMPTY; // If saving, set inital focus to the first empty slot
        }
}

void drawGameSave()
{
	//printf("Drawing game save\n");
	static int oldRc=-1;
	int refRc=oldRc;
	int rc=sceUtilitySavedataGetStatus();
	if(oldRc!=rc) printf("SavedataGetStatus returned: %08x\n",rc);
	oldRc=rc;
	
#if _PSP_FW_VERSION >= 200
    if(sceKernelDevkitVersion()<0x02000010) {
            addMessage("Can't do game save dialog in 1.5 folder of CFW.",4000);
            addMessage("Game save is disabled.",2000);
			gameMode=MODE_MAP;
			playSong(SONG_MENU);
			game.selected=0;
            return;
    }
#endif

	switch(rc) {
	case PSP_UTILITY_DIALOG_VISIBLE:
		sceUtilitySavedataUpdate(1);
		break;
	case PSP_UTILITY_DIALOG_QUIT:
	    sceUtilitySavedataShutdownStart();
	    break;
	case PSP_UTILITY_DIALOG_FINISHED:
		printf("Got result: %08x\n",gameSaveDialog.base.result);
		if(gameSaveDialog.base.result==1) {
			if(mapCompletePercent(game.map)==0) {
				printf("Back to menu.\n");
				gameMode=MODE_MENU;	// cancelled.
				oldRc=-1;
				game.selected=1;
			} else {
				gameMode=MODE_MAP;	// cancelled.
				oldRc=-1;
				game.selected=0;
			}
			return;
#define ERROR_SAVEDATA_LOAD_DATA_BROKEN 0x80110306
		} else if(gameSaveDialog.base.result==ERROR_SAVEDATA_LOAD_DATA_BROKEN) {
			printf("Error: Data was corrupt.\n");

#if 0
			FILE *file=fopen("testsavedata.bin","rb");
			if(file) {
				printf("Restoring loading from test game save.\n");
				char *buf=calloc(gameSaveDialog.dataBufSize,1);
				fread(buf,gameSaveDialog.dataBufSize,1,file);
				fclose(file);
				int *header=(int *)buf;
				loadMapBuffer(game.map,buf+24,header[0]);
				loadTeam(game.team,buf+24+header[0],header[1]);
				free(buf);
			}
#endif
		}
		if(gameSaveDialog.mode!=PSP_UTILITY_SAVEDATA_LISTLOAD) {
			printf("Saved: percent complete %d\n",mapCompletePercent(game.map));
		} else {
			char *buf=(char *)gameSaveDialog.dataBuf;
			int *header=(int *)buf;
			if(mapSaveSize(game.map)!=header[0]) {
				printf("Unmatched map save size");
			} else if(teamSaveSize(game.team)!=header[1]) {
				printf("Unmatched team save size");
			} else if(header[2]!=60|| header[2]!=30) {
				printf("Incompatible header for level load\n");
			} else if(gameSaveDialog.mode==PSP_UTILITY_SAVEDATA_LISTLOAD) {
				printf("Restoring game save\n");
#if 0
				FILE *file=fopen("testloaddata.bin","wb");
				if(file) {
					fwrite(buf,gameSaveDialog.dataBufSize,1,file);
					fclose(file);
				}
#endif
				loadMapBuffer(game.map,buf+24,header[0]);
				if(header[2]==30) {
					// repair the 6 stations into the correct sequence.
				}
				loadTeam(game.team,buf+24+header[0],header[1]);
			}
		}
		printf("Mode is map\n");
		oldRc=-1;
		gameMode=MODE_MAP;
		playSong(SONG_MENU);
		game.selected=0;
		break;
	case PSP_UTILITY_DIALOG_NONE:
		if(refRc!=0 && refRc!=-1) {
			gameMode=MODE_MAP;	// shouldn't need to be done.
			playSong(SONG_MENU);
			game.selected=0;
			oldRc=-1;
		}
	    break;
	}
}

/* enum Buttons {
	BT_UP,BT_DOWN,BT_LEFT,BT_RIGHT,
	BT_TRIANGLE,BT_CIRCLE,BT_SQUARE,BT_CROSS,
	BT_LTRIGGER,BT_RTRIGGER,
	BT_START,BT_SELECT,BT_HOLD,
	BT_NONE
}; */
const char *buttonName[]={
		"Up","Down","Left","Right",
		"/\\","O","[]","X",
		"L","R",
		"START","SELECT","HOLD",
		"none"
};

extern int trophyId[6];
const char *tournamentName[6]={
	"Bluffs Cup",
	"Creek Bed Cup",
	"Lake Cup",
	"Granite Cup",
	"Volcanic Cup",
	"Sushi Cup"
};

int trophyExplorer=0;

void drawVictory()
{
	if(game.duel) drawHud(game.duel);

	int tournament=getActiveLevel(game.map)/12;
	sceGumMatrixMode(GU_MODEL);
	int trophy=tournament+trophyExplorer;
	int i;
	for(i=0;i<(trophy%6)+1;i++) {
		sceGumPushMatrix();
		ScePspFVector3 pos={to.x,40,to.z+(i-(trophy%6)/2.0f)*15};	// front and centre -- but high on the screen.
		sceGumTranslate(&pos);
		sceGumRotateY(game.timeLeft/1000.0f*GU_PI);
		setItemPos(trophyId[i],0,0,0);
		renderItem(trophyId[i]);
		sceGumPopMatrix();
	}
	// breaking encapsulation:
	if(game.timeLeft==0) game.timeLeft=2000;

	int w=intraFontMeasureText(iFont[FONT_BODY],"Tournament Victory");
	intraFontPrint(iFont[FONT_BODY],240-w/2,50,"Tournament Victory");
	intraFontPrint(iFont[FONT_SMALL],50,160,"Good work, you beat your brother when it counted.");
	intraFontPrint(iFont[FONT_SMALL],50,180,tournamentName[trophy%6]);
	intraFontPrint(iFont[FONT_SMALL],50,200,"You get to add another mage to your team.");
	intraFontPrint(iFont[FONT_SMALL],50,220,"You will find crates are filled again.");
}

void drawGameOver()
{
	if(game.duel) drawHud(game.duel);
	int w=intraFontMeasureText(iFont[FONT_BODY],"Game Complete!");
	intraFontPrint(iFont[FONT_BODY],240-w/2,50,"Game Complete!");
	intraFontPrint(iFont[FONT_SMALL],50,100,"Good work, you make the complete character set.");
	//intraFontPrint(iFont[FONT_SMALL],50,120,"Rank: senior intern");
	intraFontPrint(iFont[FONT_SMALL],50,220,"Press X to try another game.");
}

/*void drawTestBillboard()
{
	float size=20;
	// test billboard.
	static struct Vertex3DTCP billboard[2];
	billboard[0].x=(view.x.x+view.x.y)*20;
	billboard[0].y=(view.y.x+view.y.y)*20;
	billboard[0].z=(view.z.x+view.z.y)*20;
	billboard[0].u=0.0f;
	billboard[0].v=0.0f;
	billboard[0].color=0xff00ffff;
	billboard[1].x=-(view.x.x+view.x.y)*20;
	billboard[1].y=-(view.y.x+view.y.y)*20;
	billboard[1].z=-(view.z.x+view.z.y)*20;
	billboard[1].u=0.25f;
	billboard[1].v=0.25f;
	billboard[1].color=0xff00ffff;
	sceKernelDcacheWritebackAll();
	sceGumDrawArray(GU_SPRITES,GU_VERTEX_32BITF|GU_TEXTURE_32BITF|GU_COLOR_8888|GU_TRANSFORM_3D,2,0,&billboard);
} */

void draw()
{
	u64 oldTick,newTick;

	pspDebugScreenSetXY(0,1);
	sceGuStart(GU_DIRECT,gulist);
	// clear screen
	sceRtcGetCurrentTick(&oldTick);
	sceGuClearColor(GU_RGBA(0x57,0x73,0x9b,255));	//GU_RGBA(0x80,0xc0,0xc0,255));
	if(gameMode==MODE_TITLE) sceGuClearColor(GU_RGBA(0x80,0xc0,0xc0,255));
	sceGuClearDepth(0);
#if 0
	if(gameMode==MODE_TITLE || gameMode==MODE_CUTSCENE || gameMode==MODE_EXITCUTSCENE) sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	else sceGuClear(GU_DEPTH_BUFFER_BIT);
#else
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
#endif
	sceGuTexWrap(GU_REPEAT, GU_REPEAT);

	// move camera.
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumLookAt(&from,&to,&up);
	gumLoadIdentity(&view);
	gumLookAt(&view,&from,&to,&up);
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();

	if(gameMode==MODE_TITLE) {
		sceGuEnable(GU_LIGHTING);

		sceGuEnable(GU_BLEND);
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGuEnable(GU_DEPTH_TEST);

		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		
		sceGuColor(0xffffffff);
		drawAseScene(aseTitle);
		drawTitleText();
	} else if(gameMode==MODE_CUTSCENE || gameMode==MODE_EXITCUTSCENE) {
		drawCutScene(game.cs);
	} else if(gameMode==MODE_MAP) {
		drawMap(game.map);
	} else {
		if(gameMode!=MODE_DUEL) {
			//sceGuClear(GU_COLOR_BUFFER_BIT);	// mostly for debugging.
			drawDuelSuspended(game.duel);
		} else {
			drawDuel(game.duel);
		}
	}

	if(gameMode==MODE_MENU) drawMenu();
	else if(gameMode==MODE_GAMEOVER) drawGameOver();
	else if(gameMode==MODE_VICTORY) drawVictory();

	static enum GameMode oldGameMode=MODE_GAMEOVER;	
	if(oldGameMode!=gameMode) printf("Game mode %d\n",(int)gameMode);
	if(oldGameMode!=gameMode && gameMode!=MODE_DUEL && gameMode!=MODE_MAP && gameMode!=MODE_TITLE) {
		// reset the camera.
		cameraSetFromToUp(-108,62,-66, 0,44,0, 0,1,0);
	}
	oldGameMode=gameMode;

	if(game.messageCount>0) {
		int w=0,h=8;
		const char *msg=game.message[0].text;
		w=(int)intraFontMeasureText(iFont[FONT_MESSAGE],msg);
		drawFilledRect(240-w/2-20,136-h/2-20,w+40,h+40,GU_RGBA(0,0,0,64));
		intraFontPrint(iFont[FONT_MESSAGE],240-w/2,136,msg);
	}
	// End the frame
	/*int used= */sceGuFinish();
    //printf("Used %d gulist words\n",used);
	sceGuSync(0,0);
	if(showFps) {
		static u64 realOldTick=0;
		sceRtcGetCurrentTick(&newTick);
		pspDebugScreenSetXY(0,32);
		pspDebugScreenPrintf("FPS: %.2f (%.2f)",1000000.0f/(newTick-oldTick),((newTick-oldTick)/1000.0f));
		pspDebugScreenPrintf("real: %.2f (%.2f)",1000000.0f/(newTick-realOldTick),((newTick-realOldTick)/1000.0f));
		realOldTick=newTick;
		pspDebugScreenSetXY(20,1);
	}
	//pspDebugScreenSetXY(40,0);
	//pspDebugScreenPrintf("Score: %d Level %d Step %d",game.score,game.levelNo+1,game.stepNo+1);
	if(showFps) {
		pspDebugScreenSetXY(40,28);
		pspDebugScreenPrintf("from %.2f,%.2f,%.2f",from.x,from.y,from.z);
	}
#ifdef DEBUG_TEARING
	pspDebugScreenSetXY(42,40);
	static int flipCount=0;
	pspDebugScreenPrintf("frame %d",flipCount++);
#endif

	if(gameMode==MODE_GAMESAVE) drawGameSave();
	
	sceDisplayWaitVblankStart();
	//printf("-------------------\n");
	drawBuffer=sceGuSwapBuffers();
	pspDebugScreenSetOffset((int)drawBuffer);
}

int handleTitle(enum Buttons button) {
	switch(button) {
	case BT_CROSS:
	case BT_START:
		playSfx(SFX_MENUSELECT);
		game.timeLeft=0;
		break;
	default:
		break;
	}
	return 0;
}

int handleMenu(enum Buttons button)
{
	switch(button) {
	case BT_UP:
		playSfx(SFX_MENUNEXT);
		game.selected--;
		if(game.selected<0) game.selected=4;
		break;
	case BT_DOWN:
		playSfx(SFX_MENUNEXT);
		game.selected++;
		if(game.selected>5) game.selected=0;
		break;
	case BT_CROSS:
		playSfx(SFX_MENUSELECT);
		if(game.selected==0 || game.selected==1) {
			int selected=game.selected;
			printf("Entering game save mode\n");
			newGame();
			gameMode=MODE_GAMESAVE;
			printf("game.selected=%d\n",game.selected);
			initSavedata(&gameSaveDialog, selected==0?PSP_UTILITY_SAVEDATA_LISTSAVE:PSP_UTILITY_SAVEDATA_LISTLOAD);
			int rc=sceUtilitySavedataInitStart(&gameSaveDialog);
			printf("InitStart returned: %08x\n",rc);
		} else if(game.selected==4) {
			exitRequest=1;
		} else {
			printf("Menu item %d not implemented.\n",game.selected);
#if 0
			int i;
			for(i=0;i<256;i++) {
				printf("<<<<<<<<<<<Pass %d>>>>>>>>>>\n",i);
				//freeItems();
				//loadItems();
				//loadItemImage("buildings/throneroom");
				loadCharacters(0,0);
				playSong((rand()>>16)&1);
				loadCharacters(&character[(rand()>>16)%characterCount],&character[(rand()>>16)%characterCount]);
				showFreeMem();
				printf(">>>>>>>>>>>>End Pass %d<<<<<<<<<<<<<\n",i);
				sceKernelDelayThread(2000000);
			}
#endif
		}
		break;
	default:
		break;
	}
	return 0;
}

int handleVictory(enum Buttons button)
{
	switch(button) {
	case BT_LEFT:
		trophyExplorer+=5;
		break;
	case BT_RIGHT:
		trophyExplorer++;
		break;
	case BT_CROSS:
		playSfx(SFX_MENUSELECT);
		gameMode=MODE_MAP;
		game.selected=1;
		break;
	default:
		break;
	}
	return 0;
}

int handleGameOver(enum Buttons button)
{
	switch(button) {
	case BT_CROSS:
		playSfx(SFX_MENUSELECT);
		printf("Game over.  Returning to menu.\n");
		gameMode=MODE_MENU;
		game.selected=1;
		break;
	default:
		break;
	}
	return 0;
}

int handleJoy(enum Buttons button,int up)
{
	if(up==1) {
		if(button==BT_TRIANGLE) showFps=1-showFps;
		if(button==BT_RTRIGGER || button==BT_LTRIGGER) dz=0;
	    if(gameMode==MODE_TITLE) return handleTitle(button);
	    else if(gameMode==MODE_CUTSCENE || gameMode==MODE_EXITCUTSCENE) return handleCutScene(game.cs,button);
		else if(gameMode==MODE_MENU) return handleMenu(button);
		else if(gameMode==MODE_MAP) {
			if(button==BT_START) {
				printf("Entering game save mode\n");
				gameMode=MODE_GAMESAVE;
				initSavedata(&gameSaveDialog, PSP_UTILITY_SAVEDATA_LISTSAVE);
		       	gameSaveDialog.focus=PSP_UTILITY_SAVEDATA_FOCUS_LATEST; // Set initial focus to the newest file (for overwriting)
				int rc=sceUtilitySavedataInitStart(&gameSaveDialog);
				printf("InitStart returned: %08x\n",rc);
				game.selected=0;
			}
			return handleMap(game.map,button);
		} else if(gameMode==MODE_DUEL) return handleDuel(game.duel,button);
		else if(gameMode==MODE_GAMEOVER) return handleGameOver(button);
		else if(gameMode==MODE_VICTORY) return handleVictory(button);
	} else {
		if(button==BT_LTRIGGER) dz=1;
		if(button==BT_RTRIGGER) dz=-1;
		if(gameMode==MODE_MAP) return handleMapDown(game.map,button);
	}

	return 0;
}

int handleAnalog(int lx,int ly)
{
	if(gameMode==MODE_MAP) return analogMap(game.map,lx,ly);
	
	dx=lx-100;
	if(dx>56) dx-=56;
	else if(dx>0) dx=0;
	dy=ly-100;
	if(dy>56) dy-=56;
	else if(dy>0) dy=0;
	dx/=12.0f*6.0f;
	dy/=12.0f*6.0f;
	return 0;
}

void addInventory(const char *name)
{
	ItemClass itemClass=mapNameToItemClass(name);
	printf("Mapped %s to class %d\n",name,(int)itemClass);
	if(itemClass==IC_SHARD) teamAddToInventory(&game.team->shard[0],8,name);
	else if(itemClass==IC_ITEM) teamAddToInventory(&game.team->item[0],32,name);
	else printf("didn't add to inventory, because I'm confused.\n");
}
