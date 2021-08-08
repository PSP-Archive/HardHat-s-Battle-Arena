#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include "duel.h"
#include "particle.h"

enum ActionHeightType { AHT_NORMAL,AHT_HIGH,AHT_LOW,AHT_ATTACK_HIGH,AHT_ATTACK_LOW };
const char *actionHeightName[]={"Normal","High","Low","Attack High","Attack Low"};
//const char *actionHeightName[]={"normal","high","low","attack_high","attack_low"};

enum SpecialActionType { SAT_NO,SAT_SWITCH,SAT_SWAPHP,SAT_SWAPAP,SAT_HPMAXUP,SAT_APMAXUP,SAT_DEFLECT,SAT_MIRROR,SAT_SWAPHPAP,SAT_BIZARRO,SAT_POISON,SAT_CONFUSE,SAT_PARALIZE};
const char *specialActionName[]={"None","Switch","Swap HP","Swap AP","HP Max Up","AP Max UP","Deflect","Mirror","Swap HP/AP","Bizarro","Poision","Confuse","Paralize"};

struct Action {
	char name[48];
	int id;
	PlaceType alignment;	// which place this is associated with.
	enum SpecialActionType specialAction;
	int minLevel;
	enum MotionType warriorMotion;
	enum MotionType opponentMotion;
	int hpCost;
	int apCost;
	char hpDice[16];
	char apDice[16];
	char boostDice[16];
	char xpHitDice[16];
	char xpMissDice[16];
	char selfHpDice[16];
	char selfApDice[16];
	enum ActionHeightType actionHeight;
	struct ParticleDescription *sourceDesc,*transmitDesc,*targetDesc;
};
//struct ParticleDescription sampleDesc={1000,40,600,{0,100,0},1.1f,{10,0,10},0.0f/*6.28f*/,20,1,0,0.05,0xffff00ff,0xff00ffff};
struct ParticleDescription sourceDesc={1000,4,500,0.4f,{0,-90,0},-0.8f,{0,0,0},0.0f/*6.28f*/,20,5,13,0.0f,GU_RGBA(64,0,0,255),GU_RGBA(255,128,96,255)};
struct ParticleDescription transmitDesc={1000,4,500,0.4f,{0,0,-40},5.0f,{0,0,0},6.28f,2,5,0,0.0f,GU_RGBA(255,255,255,255),GU_RGBA(255,0,0,255)};
struct ParticleDescription targetDesc={1000,4,500,0.4f,{0,50,0},0.9f,{0,0,0},0.0f/*6.28f*/,20,5,3,0.0f,GU_RGBA(255,0,0,255),GU_RGBA(255,255,0,255)};
struct ParticleDescription testDesc={45000,200,5000,0.5f,{0,25,0},1.0f,{10,0,10},0.0f/*6.28f*/,40,20,0,0.0f,0xffff00ff,0xff00ffff};
//struct ParticleEmitter testEmitter;
struct ParticleDescription sourceFbDesc={1000,4,500,0.4f,{0,-90,0},-0.5f,{0,0,0},0.0f/*6.28f*/,20,5,9,0.0f,GU_RGBA(255,255,0,255),GU_RGBA(255,255,0,255)},
	transmitFbDesc={1000,4,500,0.4f,{0,0,-40},5.0f,{0,0,0},6.28f,2,5,8,0.0f,GU_RGBA(255,128,0,255),GU_RGBA(128,0,0,255)},
	targetFbDesc={1000,4,500,0.4f,{0,50,0},0.9f,{0,0,0},0.0f/*6.28f*/,20,5,8,0.0f,GU_RGBA(255,0,0,255),GU_RGBA(255,255,0,255)};
struct ParticleDescription sourceIbDesc={1000,4,500,0.4f,{0,-90,0},-0.5f,{0,0,0},0.0f/*6.28f*/,20,5,3,0.0f,GU_RGBA(0,0,255,255),GU_RGBA(255,255,255,255)},
	transmitIbDesc={1000,4,500,0.4f,{0,0,-40},5.0f,{0,0,0},6.28f,2,5,9,0.0f,GU_RGBA(255,255,255,255),GU_RGBA(128,128,255,255)},
	targetIbDesc={1000,4,500,0.4f,{0,50,0},0.9f,{0,0,0},0.0f/*6.28f*/,20,5,1,0.0f,GU_RGBA(0,0,128,255),GU_RGBA(128,128,255,255)};
struct ParticleDescription sourceLdDesc={1000,4,500,0.4f,{0,-90,0},-0.5f,{0,0,0},0.0f/*6.28f*/,20,5,13,0.0f,GU_RGBA(0,0,0,255),GU_RGBA(128,0,0,255)},
	transmitLdDesc={1000,4,500,0.4f,{0,0,-40},5.0f,{0,0,0},6.28f,2,5,11,0.0f,GU_RGBA(128,0,0,255),GU_RGBA(0,0,0,255)},
	targetLdDesc={1000,4,500,0.4f,{0,50,0},0.9f,{0,0,0},0.0f/*6.28f*/,20,5,14,0.0f,GU_RGBA(128,0,0,255),GU_RGBA(255,128,0,255)};
struct ParticleDescription sourceBoostDesc={1000,4,500,0.4f,{0,-90,0},-0.5f,{0,0,0},0.0f/*6.28f*/,20,5,2,0.0f,GU_RGBA(255,255,0,255),GU_RGBA(255,128,0,255)},
	transmitBoostDesc={1000,4,500,0.4f,{0,50,-20},0.9f,{0,0,0},0.0f/*6.28f*/,20,5,5,0.0f,GU_RGBA(255,128,0,255),GU_RGBA(255,255,0,255)};
struct ParticleDescription sourceHealDesc={1000,4,500,0.4f,{0,-90,0},-0.5f,{0,0,0},0.0f/*6.28f*/,20,5,2,0.0f,GU_RGBA(0,64,0,255),GU_RGBA(255,255,255,255)},
	transmitHealDesc={1000,4,500,0.4f,{0,50,-20},0.9f,{0,0,0},0.0f/*6.28f*/,20,5,5,0.0f,GU_RGBA(64,128,64,255),GU_RGBA(0,255,0,255)};
struct ParticleDescription sourceSwitchDesc={1000,4,500,0.4f,{0,-90,0},-0.5f,{0,0,0},0.0f/*6.28f*/,20,5,2,0.0f,GU_RGBA(0,64,0,255),GU_RGBA(0,255,0,255)},
	transmitSwitchDesc={1000,4,500,0.4f,{0,50,-20},0.9f,{0,0,0},0.0f/*6.28f*/,20,5,5,0.0f,GU_RGBA(64,128,64,255),GU_RGBA(32,192,32,255)};


struct Action action[]={
	{"Normal Attack",1,PT_WHITE,SAT_NO,0,MT_ATTACK,MT_PAIN_A,0,0,"3d4","0","0","100","20","0","0",AHT_NORMAL,&sourceDesc,&transmitDesc,&targetDesc},
	{"Fire Breath",2,PT_RED,SAT_NO,0,MT_POINT,MT_PAIN_C,0,5,"1d20+20","0","0","200","40","0","0",AHT_NORMAL,&sourceFbDesc,&transmitFbDesc,&targetFbDesc},
	{"Boost",3,PT_WHITE,SAT_NO,0,MT_SALUTE,MT_STAND,0,1,"0","0","3d6","50","10","0","0",AHT_NORMAL,&sourceBoostDesc,&transmitBoostDesc,NULL},
	{"Crouch",4,PT_BLUE,SAT_NO,0,MT_CROUCH_STAND,MT_STAND,0,0,"0","0","0","0","0","1d4","0",AHT_LOW,NULL,NULL,NULL},
	{"Jump",5,PT_WHITE,SAT_NO,0,MT_JUMP,MT_STAND,0,0,"0","0","0","0","0","0","1d2-1",AHT_HIGH,NULL,NULL,NULL},
	{"Heal",6,PT_WHITE,SAT_NO,0,MT_WAVE,MT_STAND,0,1,"0","0","0","0","0","3d4+4","0",AHT_NORMAL,&sourceHealDesc,&transmitHealDesc,NULL},
	{"Low Attack",7,PT_YELLOW,SAT_NO,3,MT_CROUCH_ATTACK,MT_PAIN_B,0,0,"3d4","0","0","100","20","0","0",AHT_ATTACK_LOW,&sourceDesc,&transmitDesc,&targetDesc},
	{"Blizard",8,PT_BLACK,SAT_NO,4,MT_POINT,MT_PAIN_C,0,20,"3d20+20","0","0","250","100","0","0",AHT_NORMAL,&sourceIbDesc,&transmitIbDesc,&targetIbDesc},
	{"High Attack",9,PT_BLUE,SAT_NO,5,MT_FLIP,MT_PAIN_B,0,0,"3d4","0","0","100","20","0","0",AHT_ATTACK_HIGH,&sourceDesc,&transmitDesc,&targetDesc},
	{"Life Drain",10,PT_GREEN,SAT_NO,6,MT_BOOM,MT_PAIN_C,20,0,"1d20+20","0","0","200","40","0","0",AHT_NORMAL,&sourceLdDesc,&transmitLdDesc,&targetLdDesc},
	{"Crouch 2",11,PT_BLUE,SAT_NO,7,MT_CROUCH_STAND,MT_STAND,0,0,"0","0","0","0","0","1d4+10","0",AHT_LOW,NULL,NULL,NULL},
	{"Normal Attack 2",12,PT_WHITE,SAT_NO,9,MT_ATTACK,MT_PAIN_A,0,0,"3d5+10","0","0","100","20","0","0",AHT_NORMAL,&sourceDesc,&transmitDesc,&targetDesc},
	{"Fire Breath 2",13,PT_RED,SAT_NO,11,MT_POINT,MT_PAIN_C,0,15,"1d20+40","0","0","200","40","0","0",AHT_NORMAL,&sourceFbDesc,&transmitFbDesc,&targetFbDesc},
	{"Jump 2",14,PT_YELLOW,SAT_NO,13,MT_JUMP,MT_STAND,0,0,"0","0","0","0","0","0","1d5-1",AHT_HIGH,NULL,NULL,NULL},
	{"Low Attack 2",15,PT_YELLOW,SAT_NO,15,MT_CROUCH_ATTACK,MT_PAIN_B,0,0,"3d5+10","0","0","100","20","0","0",AHT_ATTACK_LOW,&sourceDesc,&transmitDesc,&targetDesc},
	{"Boost 2",16,PT_WHITE,SAT_NO,17,MT_SALUTE,MT_STAND,0,10,"0","0","3d6+20","50","10","0","0",AHT_NORMAL,&sourceBoostDesc,&transmitBoostDesc,NULL},
	{"High Attack 2",17,PT_BLUE,SAT_NO,19,MT_FLIP,MT_PAIN_B,0,0,"3d5+10","0","0","100","20","0","0",AHT_ATTACK_HIGH,&sourceDesc,&transmitDesc,&targetDesc},
	{"Blizard 2",18,PT_BLACK,SAT_NO,21,MT_POINT,MT_PAIN_C,0,40,"3d20+40","0","0","250","100","0","0",AHT_NORMAL,&sourceIbDesc,&transmitIbDesc,&targetIbDesc},
	{"Life Drain 2",19,PT_GREEN,SAT_NO,23,MT_BOOM,MT_PAIN_C,40,0,"1d20+40","0","0","200","40","0","0",AHT_NORMAL,&sourceLdDesc,&transmitLdDesc,&targetLdDesc},
	{"Heal 2",20,PT_WHITE,SAT_NO,25,MT_WAVE,MT_STAND,0,15,"0","0","0","0","0","3d5+24","0",AHT_NORMAL,&sourceHealDesc,&transmitHealDesc,NULL},
	{"Normal Attack 3",21,PT_WHITE,SAT_NO,27,MT_ATTACK,MT_PAIN_A,0,0,"3d6+20","0","0","100","20","0","0",AHT_NORMAL,&sourceDesc,&transmitDesc,&targetDesc},
	{"Crouch 3",22,PT_BLUE,SAT_NO,29,MT_CROUCH_STAND,MT_STAND,0,0,"0","0","0","0","0","1d4+20","0",AHT_LOW,NULL,NULL,NULL},
	{"Jump 3",23,PT_YELLOW,SAT_NO,31,MT_JUMP,MT_STAND,0,0,"0","0","0","0","0","0","1d10-1",AHT_HIGH,NULL,NULL,NULL},
	{"Blizard 3",24,PT_BLACK,SAT_NO,33,MT_POINT,MT_PAIN_C,0,60,"3d20+60","0","0","250","100","0","0",AHT_NORMAL,&sourceIbDesc,&transmitIbDesc,&targetIbDesc},
	{"Low Attack 3",25,PT_YELLOW,SAT_NO,35,MT_CROUCH_ATTACK,MT_PAIN_B,0,0,"3d6+20","0","0","100","20","0","0",AHT_ATTACK_LOW,&sourceDesc,&transmitDesc,&targetDesc},
	{"Heal 3",26,PT_WHITE,SAT_NO,37,MT_WAVE,MT_STAND,0,30,"0","0","0","0","0","3d6+44","0",AHT_NORMAL,&sourceHealDesc,&transmitHealDesc,NULL},
	{"Boost 3",27,PT_WHITE,SAT_NO,39,MT_SALUTE,MT_STAND,0,20,"0","0","3d6+40","50","10","0","0",AHT_NORMAL,&sourceBoostDesc,&transmitBoostDesc,NULL},
	{"High Attack 3",28,PT_BLUE,SAT_NO,41,MT_FLIP,MT_PAIN_B,0,0,"3d6+20","0","0","100","20","0","0",AHT_ATTACK_HIGH,&sourceDesc,&transmitDesc,&targetDesc},
	{"Fire Breath 3",29,PT_RED,SAT_NO,43,MT_POINT,MT_PAIN_C,0,25,"1d20+60","0","0","200","40","0","0",AHT_NORMAL,&sourceFbDesc,&transmitFbDesc,&targetFbDesc},
	{"Normal Attack 4",30,PT_WHITE,SAT_NO,45,MT_ATTACK,MT_PAIN_A,0,0,"3d7+30","0","0","100","20","0","0",AHT_NORMAL,&sourceDesc,&transmitDesc,&targetDesc},
	{"Crouch 4",31,PT_BLUE,SAT_NO,47,MT_CROUCH_STAND,MT_STAND,0,0,"0","0","0","0","0","1d5+30","0",AHT_LOW,NULL,NULL,NULL},
	{"Jump 4",32,PT_YELLOW,SAT_NO,49,MT_JUMP,MT_STAND,0,0,"0","0","0","0","0","0","1d15-1",AHT_HIGH,NULL,NULL,NULL},
	{"Blizard 4",33,PT_BLACK,SAT_NO,51,MT_POINT,MT_PAIN_C,0,80,"3d20+80","0","0","250","100","0","0",AHT_NORMAL,&sourceIbDesc,&transmitIbDesc,&targetIbDesc},
	{"Low Attack 4",34,PT_YELLOW,SAT_NO,53,MT_CROUCH_ATTACK,MT_PAIN_B,0,0,"3d7+30","0","0","100","20","0","0",AHT_ATTACK_LOW,&sourceDesc,&transmitDesc,&targetDesc},
	{"Heal 4",35,PT_WHITE,SAT_NO,55,MT_WAVE,MT_STAND,0,30,"0","0","0","0","0","3d7+66","0",AHT_NORMAL,&sourceHealDesc,&transmitHealDesc,NULL},
	{"Boost 4",36,PT_WHITE,SAT_NO,57,MT_SALUTE,MT_STAND,0,40,"0","0","3d7+60","50","10","0","0",AHT_NORMAL,&sourceBoostDesc,&transmitBoostDesc,NULL},
	{"High Attack 4",37,PT_BLUE,SAT_NO,59,MT_FLIP,MT_PAIN_B,0,0,"3d7+30","0","0","100","20","0","0",AHT_ATTACK_HIGH,&sourceDesc,&transmitDesc,&targetDesc},
	{"Fire Breath 4",38,PT_RED,SAT_NO,61,MT_POINT,MT_PAIN_C,0,50,"1d20+80","0","0","200","40","0","0",AHT_NORMAL,&sourceFbDesc,&transmitFbDesc,&targetFbDesc},

	//{"Switch Mage",SAT_SWITCH,MT_CROUCH_DEATH,MT_STAND,0,0,"0","0","0","0","0","0","0",AHT_NORMAL,&sourceSwitchDesc,&transmitSwitchDesc,NULL},
};
int actionCount=38;
#define DAMAGEDURATION 800

enum DuelMode { DM_ADDTEAMMATE, DM_CHOOSETEAMMATE, DM_SELECTACTION, DM_ATTACK, DM_OPPONENTATTACK, DM_SWITCHTEAMMATE, DM_SUCCESS, DM_FAILURE, DM_GAMEOVER, DM_LOST };
struct Duel {
	enum DuelMode mode;
	enum DuelMode oldMode;	// for debugging only.
	enum DuelType type;
	struct Team *team;
	int opponentLevel;
	struct Terrain *terrain;
	struct Actor *warrior[3];	// could be from team, or borrowed
	struct Actor opponent[3];	// dynamically generated by AI
	struct Actor *activeWarrior;
	struct Actor *activeOpponent;
	struct ParticleEmitter warriorParticles[2];
	struct ParticleEmitter opponentParticles[2];
	struct Action *warriorAction;
	struct Action *opponentAction;
	PlaceType alignment;	// which place this is associated with.
	int step;
	int selected;
	int menu;
	int charactersAcross;
	int warriorCount;	// from 1 to 3 depending on the battle size.
};

#define CHARACTERMAX 64
struct Character *character=0;
int characterCount;

struct CharacterFile {
	char *name;
	char *md2fname;
	char *skinfname;
	char *iconfname;
} characterFname[CHARACTERMAX];

/*	const char *fname[]={
		"Yvonne","models/yvonne/tris.md2","models/yvonne/yvonne.png","models/yvonne/yvonne_i.png",
		"Lizard","models/yvonne/tris.md2","models/yvonne/lizard.png","models/yvonne/lizard_i.png",
		"Demon","models/yvonne/tris.md2","models/yvonne/demon.png","models/yvonne/demon_i.png",
		"Morrigan","models/yvonne/tris.md2","models/yvonne/morrigan.png","models/yvonne/morrigan_i.png",
		"Techbot","models/yvonne/tris.md2","models/yvonne/tech.png","models/yvonne/tech_i.png",
		"Casual Yvonne","models/yvonne/tris.md2","models/yvonne/jeans.png","models/yvonne/jeans_i.png",
		"Ominous Sorceress","models/yvonne/tris.md2","models/yvonne/ominous.png","models/yvonne/ominous_i.png",
		"Chunli","models/yvonne/tris.md2","models/yvonne/chunli.png","models/yvonne/chunli_i.png",
		"Stealthy Yvonne","models/yvonne/tris.md2","models/yvonne/stealth.png","models/yvonne/stealth_i.png",
	};
*/

void loadCharacterFname()
{
	if(characterFname[0].name!=0) return;	// we loaded it already.
	FILE *file=fopen("data/character.txt","r");
	if(!file) return;
	
	char buffer[256];
	int first=0;
	int i=0;
	while( fgets(buffer,255,file) ) {
		buffer[255]=0;
		if(strchr(buffer,'\n')) strchr(buffer,'\n')[0]=0;
		if(strchr(buffer,'\r')) strchr(buffer,'\r')[0]=0;
		char *value=strchr(buffer,'=');
		if(value) value++;
		if(buffer[0]=='[') {
			first++;
			if(first>1) i++;
			if(i==CHARACTERMAX) break;
			if(strchr(buffer,']')) strchr(buffer,']')[0]=0;
			characterFname[i].name=strdup(buffer+1);
			printf("\nCharacter: %s; ",buffer+1);
		} else if(strstr(buffer,"MD2=")) {
			characterFname[i].md2fname=strdup(value);
			printf("MD2=%s; ",value);
		} else if(strstr(buffer,"SKIN=")) {
			characterFname[i].skinfname=strdup(value);
			printf("SKIN=%s; ",value);
		} else if(strstr(buffer,"ICON=")) {
			characterFname[i].iconfname=strdup(value);
			printf("ICON=%s; ",value);
		}		
	}
	fclose(file);
}

void loadCharacters(struct Character *one,struct Character *two)
{
	struct Character *c;
	int i;

	printf("Load characters %08x, %08x\n",(int)one,(int)two);
	if(character) {
		c=character;
	} else {
		c=(struct Character *)calloc(sizeof(struct Character),CHARACTERMAX);
		loadCharacterFname();
	}
	character=c;
	characterCount=0;
	
	showFreeMem();
	for(i=0;i<CHARACTERMAX;i++) {
		if(character+i!=one && character+i!=two && character[i].mesh) {
			// free any unused ones.
			printf("About to free mesh for model '%s'\n",characterFname[i].name);
			md2Free(character[i].mesh);
			character[i].mesh=0;
			printf("Done free mesh for model '%s'\n",characterFname[i].name);
			//showFreeMem();
		}			
	}
	for(i=0;i<CHARACTERMAX;i++) {
		const char *name=characterFname[i].name; //fname[i*4];
		const char *md2fname=characterFname[i].md2fname; //fname[i*4+1];
		const char *texfname=characterFname[i].skinfname; //fname[i*4+2];
		const char *iconfname=characterFname[i].iconfname; //fname[i*4+3];
		// Load all of the characters from data/characters.txt, and set their initial attributes
		if(!name) break;	// no more characters to load.
		strcpy(c->name,name);
		if((c==one || c==two) && !character[i].mesh) {
			//showFreeMem();
			printf("About to load mesh for model '%s'\n",md2fname);
			c->mesh=md2Load(md2fname);
			if(!c->mesh) {
				printf("Could not find model '%s'\n",md2fname);
				continue;
			}
			printf("Loaded mesh for model '%s'\n",md2fname);
			//showFreeMem();
			c->mesh->texture=loadPng(texfname);
			swizzleToVRam=1;
			swizzleFast(c->mesh->texture);
			swizzleToVRam=0;
		}
		if(!c->icon) {
			//printf("<<<<<<Loading icon '%s'\n",iconfname);
			c->icon=loadPng(iconfname);
			swizzleToVRam=0;
			swizzleFast(c->icon);
			//printf("<<<<<<Loaded icon %08x\n",(int)c->icon);
			if(!c->icon) c->icon=newImage(16,16);
		}

		c++;
		characterCount++;
	}
	showFreeMem();
	sceKernelDcacheWritebackAll();
}

struct Duel *newDuel()
{
	struct Duel *duel=(struct Duel *)calloc(sizeof(struct Duel),1);
	if(!duel->terrain) duel->terrain=terrainInit("data/field.png");
	printf("duel->terrain=%08x\n",(int)duel->terrain);
	duel->charactersAcross=1;
	return duel;
}

void copyActor(struct Actor *dest,struct Actor *source)
{
	strcpy(dest->name,source->name);	// name of the creature
	dest->level=source->level;	// level 1
	dest->xp=source->xp;		// level up every 8000 points?
	dest->hpMax=source->hpMax;	// hpMax is a dice roll for each level
	dest->hp=dest->hpMax;
	dest->hpOld=dest->hpMax;
	dest->apMax=source->apMax;	// apMax is a dice roll for each 5 levels
	dest->ap=dest->apMax;
	dest->apOld=dest->apMax;
	dest->oldTimer=0;
	dest->attack=source->attack;	// attack is a dice roll for each level
	dest->defense=source->defense;	// defense is a dice roll for each level
	dest->character=source->character;
	dest->boost=0;
	dest->oldMotion=dest->motion;
	dest->motion=MT_STAND;
	dest->lastFrame=dest->frame;
	dest->frame=0;
	dest->frameElapsed=0;
}

void resetActor(struct Actor *a)
{
	a->hp=a->hpMax;
	a->ap=a->apMax;
	a->boost=0;
	// for character animation
	a->oldMotion=a->motion;
	a->motion=MT_STAND;
	a->lastFrame=a->frame;
	a->frame=animMotion[MT_STAND].startFrame;
	a->frameElapsed=0;
	// for hud animation
	a->hpOld=a->hp;
	a->apOld=a->ap;
	a->oldTimer=0;
}

unsigned long placeColor[]={
	GU_RGBA(255,255,255,255),	// white
	GU_RGBA(255,255,128,255),	// yellow
	GU_RGBA(255,128,128,255),	// red
	GU_RGBA(128,128,255,255),	// blue
	GU_RGBA(128,128,128,255),	// black
	GU_RGBA(128,255,128,255),	// green	
};

void initDuel(struct Duel *duel,PlaceType place,int enemyLevel,DuelType type,struct Team *team)
{
	printf("Reseting the duel.\n");
	printf("<<< actorCount: %d, targetCount: %d\n",team->actorCount,team->targetCount);
	duel->mode=team->actorCount<team->targetCount && place!=PT_BLACK?DM_ADDTEAMMATE:DM_CHOOSETEAMMATE;
	duel->oldMode=(enum DuelMode)-1;
	duel->warrior[0]=0;
	duel->warrior[1]=0;
	duel->warrior[2]=0;
	duel->type=type;
	duel->team=team;
	duel->opponentLevel=enemyLevel;
	duel->activeWarrior=0;
	duel->warriorAction=0;
	duel->warriorParticles[0].desc=0;
	duel->warriorParticles[1].desc=0;
	duel->activeOpponent=0;
	duel->opponentAction=0;
	duel->opponentParticles[0].desc=0;
	duel->opponentParticles[1].desc=0;
	duel->alignment=place;
	if(!duel->terrain) duel->terrain=terrainInit("data/field.png");
	printf("duel->terrain=%08x\n",(int)duel->terrain);
	setVertexColor(duel->terrain,placeColor[place]);
	cameraSetFromToUp(-108,62,-66, 0,44,0, 0,1,0);
	int i;
	for(i=0;i<team->actorCount;i++) {
		resetActor(team->actor+i);
	}
}

int updateActor(struct Actor *actor,int elapsed)
{
	int result=0;	// motion incomplete.
	
	if(!actor) return result;
	if(!actor->character) return result;

	actor->oldTimer-=elapsed;
	if(actor->oldTimer<0) actor->oldTimer=0;
	if(actor->oldMotion!=actor->motion) {
		printf("character %s now doing motion %s.\n",actor->name,animMotionName[actor->motion]);
		actor->oldMotion=actor->motion;
	}
	
	// animate the MD2
	actor->frameElapsed+=elapsed;
	struct AnimMotion *motion=animMotion+actor->motion;
	int frameMod=actor->frameElapsed/(1000/motion->fps);
	frameMod+=motion->startFrame;
	if(frameMod>motion->endFrame) {
		//printf("Fmod: %d, endFrame=%d\n",frameMod,motion->endFrame);
		//printf("Motion '%s' complete.\n",animMotionName[actor->motion]);
		actor->motion=MT_STAND;	// always go back to standing
		frameMod=animMotion[MT_STAND].startFrame;
		actor->frameElapsed=0;
		result=1;	// motion complete.
	}
	if(frameMod!=actor->frame) {
		actor->lastFrame=actor->frame;
		actor->frame=frameMod;
	}
	//printf("Actor frame: %d, frameElapsed=%d\n",actor->frame,actor->frameElapsed);
	return result;
}

void actorMotion(struct Actor *actor,enum MotionType motionType)
{
	if(actor->motion==MT_STAND && motionType==MT_STAND) return;
	actor->motion=motionType;
	actor->frameElapsed=0;
}

/*
 * Rolls dice from a parameter file.
 * \param dice the dice to roll in "5" or "2d6" or "3d18+24" formats.
 */
int roll(const char *dice)
{
	int count=0;
	int sides=0;
	int plus=0;
	printf("Requested dice %s: ",dice);
	if( sscanf(dice,"%dd%d+%d",&count,&sides,&plus) != 3) {
		count=0;
		sides=0;
		if( sscanf(dice,"%dd%d",&count,&sides) !=2) {
			if( sscanf(dice,"%dd%d-%d",&count,&sides,&plus) != 3) {
				count=0;
				sscanf(dice,"%d",&plus);
			} else {
				plus=-plus;
			}
		}
	}
	static int howRandom=0;
	if(!howRandom) {
		srand(time(0));
		howRandom=1;
	}
	if(sides<=0) sides=1;
	int result=plus;
	int i;
	for(i=0;i<count;i++) {
		int die;
		while(sides <= (die=rand()/(RAND_MAX/sides))) ;	// excludes the die==sides outlier.
		die++;
		result+=die;
	}
	printf("%d\n",result);
	return result;
}

int isBlocked(struct Action *action,struct Action *opponentAction)
{
	printf("proposed action from %s to %s:",actionHeightName[action->actionHeight],actionHeightName[opponentAction->actionHeight]);
	if(action->actionHeight==AHT_ATTACK_LOW) {
		// must be AHT_LOW
		printf(opponentAction->actionHeight==AHT_HIGH?"blocked\n":"good\n");	// jumped or something.
		return opponentAction->actionHeight==AHT_HIGH;
	} else if(action->actionHeight==AHT_ATTACK_HIGH) {
		printf(opponentAction->actionHeight==AHT_LOW?"blocked\n":"good\n");
		return opponentAction->actionHeight==AHT_LOW;
	}
	// any other situation
	printf(opponentAction->actionHeight==AHT_HIGH || opponentAction->actionHeight==AHT_LOW?"blocked\n":"good\n");
	return opponentAction->actionHeight==AHT_HIGH || opponentAction->actionHeight==AHT_LOW;
}

void printStats(struct Actor *actor)
{
	printf("!%s: HP %d/%d; AP %d/%d; Boost %d; XP %d; Level %d\n",actor->name,actor->hp,actor->hpMax,actor->ap,actor->apMax,actor->boost,actor->xp,actor->level);
}

int getNextLevel(int level)
{
	if(level<0) {
		return 0;
	} else if(level<10) {
		return level*1000;
	} else if(level<20) {
		return 10000+(level-10)*2000;
	} else if(level<30) {
		return 30000+(level-20)*4000;
	} else if(level<50) {
		return 70000+(level-30)*6000;
	}
	return 150000+(level-50)*8000;
}

int levelUp(struct Actor *from)
{
	int updated=0;
	// Level up, if we just crossed the leveling zone.
	int nextXp=getNextLevel(from->level);
//	if(from->xp==0) from->xp=nextXp;
	while(from->xp>=nextXp) {
		from->level++;
		int newHp=roll("5d2");
		from->hpMax+=newHp;
		from->hp+=newHp;
		from->attack+=roll("1d2");
		from->defense+=roll("1d2");
		from->apMax+=2;
		from->ap+=2;
		updated=1;
		nextXp=getNextLevel(from->level);
	}
	if(updated) {
		char buf[256];
		sprintf(buf,"%s now at level %d",from->name,from->level);
		addMessage(buf,3000);
		int i;
		for(i=0;i<actionCount;i++) {
			if(from->level==action[i].minLevel) {
				sprintf(buf,"%s now can use %s",from->name,action[i].name);
				addMessage(buf,3000);
				break;
			}
		}
	}
	return updated;
}

int doDamage(struct Actor *from,struct Actor *to,struct Action *action,struct Action *opponentAction)
{
	int hit=0;
	printf("---- Doing %s damage from %s to %s\n",action->name,from->name,to->name);
	printStats(from);
	printStats(to);
	// Doesn't factor in jump or crouch, but otherwise should work.
	from->hpOld=from->hp;
	from->apOld=from->ap;
	from->oldTimer=DAMAGEDURATION;
	to->hpOld=to->hp;
	to->apOld=to->ap;
	to->oldTimer=DAMAGEDURATION;
	int oldBoost=from->boost;
	printf("from %d+roll attack > to %d defense?",from->attack,to->defense);
	int spread=from->attack-to->defense;
	if(spread>5) spread=5;
	if(roll("1d20")>spread && isBlocked(action,opponentAction)==0) {
		// hit!
		printf("update to hp");
		int hp=roll(action->hpDice);
		if(hp!=0 && hp+from->attack-to->defense>0) to->hp-=hp+from->attack-to->defense;
		else if(hp!=0) {
			char buf[256];
			sprintf(buf,"Damage ineffective.");
			addMessage(buf,1000);
		}
		printf("update to ap");
		to->ap-=roll(action->apDice);
		hit=1;
		if(to->hp!=to->hpOld || to->ap!=to->apOld) playSfx(to->hp>0?SFX_PAIN:SFX_EXPLODE);
	} else {
		// missed
		int hp=roll(action->hpDice);
		int ap=roll(action->apDice);
		if(hp!=0 || ap!=0) {
			char msg[256];
			if(isBlocked(action,opponentAction)) {
				if(ap>0) sprintf(msg,"%d ap damage dodged by %s.",ap,opponentAction->name);
				if(hp>0) sprintf(msg,"%d damage dodged by %s.",hp,opponentAction->name);
				addMessage(msg,1000);
			} else {
				if(ap>0) sprintf(msg,"%d ap damage missed.",ap);
				if(hp>0) sprintf(msg,"%d damage missed.",hp);
				addMessage(msg,1000);
			}
		}
	}
	printStats(from);
	printStats(to);
	printf("from %d+roll attack > from %d defence?",from->attack,from->defense);
	spread=(from->attack-from->defense)/2;
	if(spread>5) spread=5;
	if(roll("1d20")>spread) {
		// hit
		printf("update from hp");
		from->hp+=roll(action->selfHpDice);
		if(from->hp>from->hpMax) from->hp=from->hpMax;
		printf("update from ap");
		from->ap+=roll(action->selfApDice);
		if(from->ap>from->apMax) from->ap=from->apMax;
		printf("update from boost");
		int oldBoost=from->boost;
		from->boost+=roll(action->boostDice);
		if(from->hp!=from->hpOld || from->ap!=from->apOld || from->boost!=oldBoost) playSfx(SFX_POWERUP);
	} else {
		int hp=roll(action->selfHpDice);
		int ap=roll(action->selfApDice);
		int boost=roll(action->boostDice);
		if(hp!=0) {
			addMessage("Heal missed.",3000);
		} else if(ap!=0) {
			addMessage("Ap bonus missed.",3000);
		} else if(boost!=0) {
			addMessage("Boost missed.",3000);
		}
	}
	printStats(from);
	printStats(to);
	// apply boost
	printf("apply boost %d, to increase whatever momentum there is\n",from->boost);
	if(from->hp!=from->hpOld) from->hp+=from->boost;
	if(from->ap!=from->apOld) from->ap+=from->boost/2;
	if(to->hp!=to->hpOld) to->hp-=from->boost;
	if(to->ap!=to->apOld) to->ap-=from->boost/2;
	printStats(from);
	printStats(to);

	// Now apply XP, and clear boost if it was used.
	if(from->hp!=from->hpOld || from->ap!=from->apOld || to->hp!=to->hpOld || to->ap!=to->apOld || oldBoost!=from->boost) {
		printf("Add in XP for a hit\n");
		from->xp+=roll(action->xpHitDice);
		if(oldBoost==from->boost) from->boost=0;	// boost is used.
	} else {
		printf("Add in XP for a miss\n");
		from->xp+=roll(action->xpMissDice);
	}
	printStats(from);
	printStats(to);
	// use up cost anyway
	if(action->hpCost>0) printf("Spend %d hp cost\n",action->hpCost);
	from->hp-=action->hpCost;
	if(action->apCost>0) printf("Spend %d ap cost\n",action->apCost);
	from->ap-=action->apCost;

	levelUp(from);

	// make it all work out
	if(from->hp<0) from->hp=0;
	if(to->hp<0) to->hp=0;
	if(from->ap<0) from->ap=0;
	if(to->ap<0) to->ap=0;
	printStats(from);
	printStats(to);
	printf("---- doDamage Complete\n");
	
	return hit;
}

void updateDuelSuspended(struct Duel *duel,int elapsed)
{
	if(!duel) return;
	enum MotionType motion;
	motion=duel->activeWarrior?duel->activeWarrior->motion:MT_STAND;
	int done;
	done=updateActor(duel->activeWarrior,elapsed);
	if(done && motion==MT_DEATH_FALLBACK) {
		duel->activeWarrior=0;
	}
	motion=duel->activeOpponent?duel->activeOpponent->motion:MT_STAND;
	done=updateActor(duel->activeOpponent,elapsed);
	if(done && motion==MT_DEATH_FALLBACK) {
		duel->activeOpponent=0;
	}
	if(updateParticles(duel->warriorParticles,elapsed)) duel->warriorParticles->desc=0;
	if(updateParticles(duel->warriorParticles+1,elapsed)) duel->warriorParticles[1].desc=0;
	if(updateParticles(duel->opponentParticles,elapsed)) duel->opponentParticles->desc=0;
	if(updateParticles(duel->opponentParticles+1,elapsed)) duel->opponentParticles[1].desc=0;
	//if(updateParticles(&testEmitter,elapsed)) {printf("new emitter needed.\n"); newParticle(&testEmitter,&testDesc,0,0,0); }
}

DuelState updateDuel(struct Duel *duel,int elapsed)
{
	if(duel->oldMode!=duel->mode) {
		duel->oldMode=duel->mode;
		printf("duel mode %d\n",duel->mode);
	}
	if(duel->mode!=DM_ADDTEAMMATE && duel->mode!=DM_CHOOSETEAMMATE && duel->mode!=DM_SWITCHTEAMMATE && duel->mode!=DM_FAILURE && duel->mode!=DM_SUCCESS) {	// freeze the timer when the game play stops.
		if(duel->mode==DM_ATTACK || duel->mode==DM_OPPONENTATTACK) {
			if(getTimeLeft()+elapsed>3000 && getTimeLeft()<=3000) {
				//transmitDesc.which=(sourceDesc.which+5)%16;
				enum ActionHeightType ah=duel->mode==DM_ATTACK?duel->warriorAction->actionHeight:duel->opponentAction->actionHeight;
				float y=ah==AHT_ATTACK_LOW?-6:ah==AHT_ATTACK_HIGH?14:4;
				struct ParticleDescription *desc=duel->mode==DM_ATTACK?duel->warriorAction->transmitDesc:duel->opponentAction->transmitDesc;
				if(desc) newParticle(duel->mode==DM_ATTACK?duel->warriorParticles+1:duel->opponentParticles+1,desc,0,y,-10);
			} else if( getTimeLeft()+elapsed>=2000 && getTimeLeft()<2000) {
				// apply damage.
				int hit=0;
				if(duel->mode==DM_ATTACK) hit=doDamage(duel->activeWarrior,duel->activeOpponent,duel->warriorAction,duel->opponentAction);
				else hit=doDamage(duel->activeOpponent,duel->activeWarrior,duel->opponentAction,duel->warriorAction);
				//targetDesc.which=(targetDesc.which+3)%16;
				if(hit) {
					struct ParticleDescription *desc=duel->mode==DM_ATTACK?duel->warriorAction->targetDesc:duel->opponentAction->targetDesc;
					if(desc) newParticle(duel->mode==DM_ATTACK?duel->opponentParticles:duel->warriorParticles,desc,0,0,0);
				}
				if(duel->mode==DM_OPPONENTATTACK) {
					enum MotionType m=duel->opponentAction->opponentMotion;
					if(duel->warriorAction->warriorMotion==MT_JUMP || duel->warriorAction->warriorMotion==MT_CROUCH_STAND) m=duel->warriorAction->warriorMotion;
					actorMotion(duel->activeWarrior,m);
				} else {
					enum MotionType m=duel->warriorAction->opponentMotion;
					if(duel->opponentAction->warriorMotion==MT_JUMP || duel->opponentAction->warriorMotion==MT_CROUCH_STAND) m=duel->opponentAction->warriorMotion;
					actorMotion(duel->activeOpponent,m);
				}
			}
		}
		if(duel->mode==DM_ATTACK && getTimeLeft()==0) {
			if(duel->activeOpponent->hp<=0) {
				duel->mode=DM_SUCCESS;
				playSfx(SFX_WIN);
				actorMotion(duel->activeOpponent,MT_DEATH_FALLBACK);
				actorMotion(duel->activeWarrior,MT_FALLBACK);
				char buf[256];
				sprintf(buf,"%s gains %d XP for the win.",duel->activeWarrior->name,duel->activeOpponent->level*60);
				addMessage(buf,2000);
				duel->activeWarrior->xp+=duel->activeOpponent->level*60;
				//copyActor(whence,duel->activeWarrior);
			} else {
				duel->mode=DM_OPPONENTATTACK;
				setTimeLeft(4000);
				actorMotion(duel->activeOpponent,duel->opponentAction->warriorMotion);
				//sourceDesc.which=(sourceDesc.which+1)%16;
				struct ParticleDescription *desc=duel->opponentAction->sourceDesc;
				if(desc) newParticle(duel->opponentParticles,desc,0,32,0);
			}
		}
		if(duel->mode==DM_OPPONENTATTACK && getTimeLeft()==0) {
			if(duel->activeWarrior->hp<=0) {
				duel->mode=DM_FAILURE;
				playSfx(SFX_LOSE);
				actorMotion(duel->activeWarrior,MT_DEATH_FALLBACK);
				actorMotion(duel->activeOpponent,MT_FALLBACK);
				//copyActor(whence,duel->activeWarrior);
				char buf[256];
				sprintf(buf,"%s gains %d XP for the win.",duel->activeOpponent->name,duel->activeWarrior->level*60);
				addMessage(buf,2000);
				duel->activeOpponent->xp+=duel->activeWarrior->level*60;
			} else {
				duel->mode=DM_SELECTACTION;
				duel->selected=0;
				duel->menu=0;
			}
		}
	}
	updateDuelSuspended(duel,elapsed);
	
	return duel->mode==DM_LOST?DS_LOST:duel->mode!=DM_GAMEOVER?DS_ACTIVE:DS_WIN;
}

void drawStats(int left,int top,struct Actor *actor)
{
	if(!actor) return;
	if(!actor->character) return;
	static Image *hudLeft;

	if(!hudLeft) {
		hudLeft=loadPng("data/hudleft.png");
		swizzleFast(hudLeft);
	}
	int x,y,w,h;
	char buf[128];

	if(hudLeft) {
		drawSprite(0,0,hudLeft->imageWidth,hudLeft->imageHeight,hudLeft,left,top);
	}
	//if(hudRight) {
	//	drawSprite(0,0,hudRight->imageWidth,hudRight->imageHeight,hudRight,480-hudRight->imageWidth,272-hudRight->imageHeight);
	//}

	x=10+left; y=12+top; w=0; h=12;
	sprintf(buf,"%.64s",actor->name);
	w=intraFontMeasureText(iFont[FONT_BODY],buf);
	intraFontPrint(iFont[FONT_BODY],x,y,buf);
			
	intraFontSetStyle(iFont[FONT_SMALL],0.8f,GU_RGBA(255,255,255,0xff),GU_RGBA(0,0,0,255),0);
	intraFontSetStyle(iFont[FONT_SMALLHIGHLIGHT],0.8f,GU_RGBA(255,255,255,0xff),GU_RGBA(0,0,0,255),0);
	x=10+left; y=30+top; w=0; h=8;
	sprintf(buf,"%d / %d",actor->hp,actor->hpMax);
	drawFilledRect(x+15,y-3,128,6,GU_RGBA(0,0x80,0x80,255));
	w=(128*actor->hp/actor->hpMax*(DAMAGEDURATION-actor->oldTimer)+
	128*actor->hpOld/actor->hpMax*actor->oldTimer)/DAMAGEDURATION;
	drawFilledRect(x+15,y-1,w,2,GU_RGBA(0,255,255,255));
	intraFontPrint(iFont[FONT_SMALL],x-8,y+h/2,"HP");
	w=intraFontMeasureText(iFont[FONT_SMALL],buf);
	intraFontPrint(iFont[FONT_SMALL],x+10+64-w/2,y+h/2,buf);

	x=10+left; y=42+top; w=0; h=8;
	sprintf(buf,"%d / %d",actor->ap,actor->apMax);
	drawFilledRect(x+15,y-3,128,6,GU_RGBA(0x80,0x40,0x0,255));
	w=(128*actor->ap/actor->apMax*(DAMAGEDURATION-actor->oldTimer)+
	128*actor->apOld/actor->apMax*actor->oldTimer)/DAMAGEDURATION;
	drawFilledRect(x+15,y-1,w,2,GU_RGBA(255,255,0,255));
	intraFontPrint(iFont[FONT_SMALL],x-8,y+h/2,"AP");
	w=intraFontMeasureText(iFont[FONT_SMALL],buf);
	intraFontPrint(iFont[FONT_SMALL],x+10+64-w/2,y+h/2,buf);

	x=10+left; y=54+top; w=0; h=8;
	w=intraFontMeasureText(iFont[FONT_SMALL],"XP");
	sprintf(buf,"XP  %d",actor->xp);
	intraFontPrint(iFont[FONT_SMALL],x-8,y+h/2,buf);

	if(actor->boost==0) {
		sprintf(buf,"Level %d",actor->level);
	} else {
		sprintf(buf,"Boost %d",actor->boost);
	}
	intraFontPrint(iFont[FONT_SMALL],x+75,y+h/2,buf);

	x=10+left; y=66+top; w=0; h=8;
	w=intraFontMeasureText(iFont[FONT_SMALL],"ATK");
	sprintf(buf,"atk %d",actor->attack);
	intraFontPrint(iFont[FONT_SMALL],x-8,y+h/2,buf);
	sprintf(buf,"def %d",actor->defense);
	intraFontPrint(iFont[FONT_SMALL],x+75,y+h/2,buf);

	/*
	// Debug:
	x=10+left; y=75+top; w=0; h=8;
	w=intraFontMeasureText(iFont[FONT_SMALL],"motion");
	sprintf(buf,"motion: %s",animMotionName[actor->motion]);
	intraFontPrint(iFont[FONT_SMALL],x-8,y+h/2,buf);
	*/
}

/*
 * Draws the heads up display.
 */
void drawHud(struct Duel *duel)
{
	struct Team *team=duel->team;
	static Image *eliminated=0;
	if(!eliminated) {
		eliminated=loadPng("data/eliminated.png");
		swizzleFast(eliminated);
	}
	drawStats(0,0,duel->activeWarrior);
	drawStats(320,0,duel->activeOpponent);
	int i;
	for(i=0;i<(team->actorCount+1)/2;i++) {
		if(!duel->warrior[i]) continue;	// happens during an upgrade.
		Image *source;
		source=duel->warrior[i]->character->icon;
		int x=6,y=100+i*40;
		if(duel->warrior[i]==duel->activeWarrior) drawFilledRect(x-1,y-1,source->imageWidth+2,source->imageHeight+2,GU_RGBA(128,128,128,255));
		drawSprite(0,0,source->imageWidth,source->imageHeight,source,x,y);
		if(duel->warrior[i]->hp==0) drawSprite(0,0,32,32,eliminated,x,y);
		source=duel->opponent[i].character->icon;
		x=480-6-32;
		if(&duel->opponent[i]==duel->activeOpponent) drawFilledRect(x-1,y-1,source->imageWidth+2,source->imageHeight+2,GU_RGBA(128,128,128,255));
		drawSprite(0,0,source->imageWidth,source->imageHeight,source,x,y);
		if(duel->opponent[i].hp==0) drawSprite(0,0,32,32,eliminated,x,y);
	}
}

void drawAddTeammate(struct Duel *duel)
{
	int i;
	if(duel->selected<0) duel->selected=0;
	if(duel->selected>=characterCount) duel->selected=0;
	struct Team *team=duel->team;
	int maxWidth=1;
	//printf("Drawing %d characters.  First character: %08x\n",characterCount,(int)character[0].name);
	//float width=intraFontMeasureText(iFont[FONT_BODY],"Add a Mage to Train");
	intraFontPrint(iFont[FONT_BODY],5,13,"Add a Mage to Train");

	for(i=0;i<characterCount;i++) {
		intraFont *font=iFont[FONT_SMALL];
		float width=intraFontMeasureText(font,character[i].name);
		if(width>maxWidth) maxWidth=(int)ceil(width);
	}
	if(maxWidth>80) maxWidth=80;
	int across=480/maxWidth;
	if(across<1) across=1;
	int down=(characterCount+across-1)/across;	// round up
	across=(characterCount+down-1)/down;	// equalize rows and round up
	duel->charactersAcross=across-1;	// hack.  There is something wrong with my math?
	int left=240-(across*(maxWidth+5)-5)/2;
	int top=287/2-(down*(52)-5)/2;
	for(i=0;i<characterCount;i++) {
		intraFont *font=iFont[FONT_SMALL];
		int offset=0;
		int home=0;
		int j;
		for(j=0;j<team->actorCount;j++) {
			home+=(character+i)==team->actor[i].character;
		}

		int x=left+(i/down)*(maxWidth+5);
		int y=top+(i%down)*(52);
		if(home) drawFilledRect(x+maxWidth/2-character[i].icon->imageWidth/2-5,y-5,character[i].icon->imageWidth+10,character[i].icon->imageHeight+10,GU_RGBA(255,0,0,255));
		
		if(duel->selected==i) {
			font=iFont[FONT_SMALLHIGHLIGHT];
			offset=5*cosf(getGiggle());
			int a=(int)(127*cosf(getGiggle()*2)+128);
			//drawSprite(0,0,character[i].icon->imageWidth,character[i].icon->imageHeight,character[i].icon,240,100);
			drawFilledRect(x+maxWidth/2-character[i].icon->imageWidth/2-1,y-1,character[i].icon->imageWidth+2,character[i].icon->imageHeight+2,GU_RGBA(a,a,a,255));
		}
		drawSprite(0,0,character[i].icon->imageWidth,character[i].icon->imageHeight,character[i].icon,x+maxWidth/2-character[i].icon->imageWidth/2,y);
		float width=intraFontMeasureText(font,character[i].name);
		intraFontPrint(font,x+offset+maxWidth/2-width/2,y+character[i].icon->imageHeight+12,character[i].name);
	}
}

void drawChooseTeammate(struct Duel *duel)
{
	struct Team *team=duel->team;
	int i;
	if(team->actorCount<1) return;
	if(duel->selected<0) duel->selected=team->actorCount-1;
	if(duel->selected>=team->actorCount) duel->selected=0;
	int maxWidth=1;
	for(i=0;i<team->actorCount;i++) {
		intraFont *font=iFont[FONT_SMALL];
		float width=intraFontMeasureText(font,team->actor[i].character->name)+2;
		if(width>maxWidth) maxWidth=(int)ceil(width);
	}
	if(maxWidth>80) maxWidth=80;
	int across=480/maxWidth;
	if(across<1) across=1;
	int down=(team->actorCount+across-1)/across;	// round up
	across=(team->actorCount+down-1)/down;	// equalize rows and round up
	duel->charactersAcross=across;
	drawStats(0,0,team->actor+duel->selected);
	int left=240-(across*(maxWidth+5)-5)/2;
	int top=272/2-(down*(55)-5)/2;
	for(i=0;i<team->actorCount;i++) {
		intraFont *font=iFont[FONT_SMALL];
		int offset=0;
		int home=0;
		if(duel->step>0) home+=(team->actor+i)==duel->warrior[0];
		if(duel->step>1) home+=(team->actor+i)==duel->warrior[1];
		if(duel->step>2) home+=(team->actor+i)==duel->warrior[2];

		int x=left+(i/down)*(maxWidth+5);
		int y=top+(i%down)*(55);
		Image *icon=team->actor[i].character->icon;
		if(home) drawFilledRect(x+maxWidth/2-icon->imageWidth/2-5,y-5,icon->imageWidth+10,icon->imageHeight+10,GU_RGBA(255,0,0,255));
		
		if(duel->selected==i) {
			font=iFont[FONT_SMALLHIGHLIGHT];
			offset=5*cosf(getGiggle());
			int a=(int)(127*cosf(getGiggle()*2)+128);
			//drawSprite(0,0,character[i].icon->imageWidth,character[i].icon->imageHeight,character[i].icon,240,100);
			drawFilledRect(x+maxWidth/2-icon->imageWidth/2-1,y-1,icon->imageWidth+2,icon->imageHeight+2,GU_RGBA(a,a,a,255));
		}
		drawSprite(0,0,icon->imageWidth,icon->imageHeight,icon,x+maxWidth/2-icon->imageWidth/2,y);
		const char *name=team->actor[i].name;
		float width=intraFontMeasureText(font,name);
		intraFontPrint(font,x+offset+maxWidth/2-width/2,y+character[i].icon->imageHeight+15,name);
	}
}

void drawSwitchTeammate(struct Duel *duel)
{
	struct Team *team=duel->team;
	int i;
	if(team->actorCount<1) return;
	if(duel->selected<0) duel->selected=(team->actorCount+1)/2-1;
	if(duel->selected>=(team->actorCount+1)/2) duel->selected=0;
	int maxWidth=1;
	for(i=0;i<(team->actorCount+1)/2;i++) {
		intraFont *font=iFont[FONT_SMALL];
		float width=intraFontMeasureText(font,duel->warrior[i]->character->name);
		if(width>maxWidth) maxWidth=(int)ceil(width);
	}
	if(maxWidth>80) maxWidth=80;
	int across=480/maxWidth;
	if(across<1) across=1;
	int down=(team->actorCount+across-1)/across;	// round up
	across=(team->actorCount+down-1)/down;	// equalize rows and round up
	duel->charactersAcross=across;
	drawStats(0,0,duel->warrior[duel->selected]);
	int left=240-(across*(maxWidth+5)-5)/2;
	int top=272/2-(down*(55)-5)/2;
	for(i=0;i<(team->actorCount+1)/2;i++) {
		intraFont *font=iFont[FONT_SMALL];
		int offset=0;
		int home=duel->warrior[i]->hp>0;

		int x=left+(i/down)*(maxWidth+5);
		int y=top+(i%down)*(55);
		Image *icon=duel->warrior[i]->character->icon;
		if(home) drawFilledRect(x+maxWidth/2-icon->imageWidth/2-5,y-5,icon->imageWidth+10,icon->imageHeight+10,GU_RGBA(255,0,0,255));
		
		if(duel->selected==i) {
			font=iFont[FONT_SMALLHIGHLIGHT];
			offset=5*cosf(getGiggle());
			int a=(int)(127*cosf(getGiggle()*2)+128);
			//drawSprite(0,0,character[i].icon->imageWidth,character[i].icon->imageHeight,character[i].icon,240,100);
			drawFilledRect(x+maxWidth/2-icon->imageWidth/2-1,y-1,icon->imageWidth+2,icon->imageHeight+2,GU_RGBA(a,a,a,255));
		}
		drawSprite(0,0,icon->imageWidth,icon->imageHeight,icon,x+maxWidth/2-icon->imageWidth/2,y);
		const char *name=duel->warrior[i]->name;
		float width=intraFontMeasureText(font,name);
		intraFontPrint(font,x+offset+maxWidth/2-width/2,y+duel->warrior[i]->character->icon->imageHeight+15,name);
	}
}

void drawAttack(struct Duel *duel)
{
	drawHud(duel);
	int w=intraFontMeasureText(iFont[FONT_MESSAGE],"You");
	intraFontPrint(iFont[FONT_MESSAGE],240-w/2,25,"You");
	w=intraFontMeasureText(iFont[FONT_MESSAGE],duel->warriorAction->name);
	intraFontPrint(iFont[FONT_MESSAGE],240-w/2,50,duel->warriorAction->name);
}

void drawFailure(struct Duel *duel)
{
	drawHud(duel);
	int w=intraFontMeasureText(iFont[FONT_BODY],"Battle Failed!");
	intraFontPrint(iFont[FONT_BODY],240-w/2,50,"Battle Failed!");
	intraFontPrintf(iFont[FONT_SMALL],50,100,"You were defeated.");
	intraFontPrint(iFont[FONT_SMALL],50,220,"Press O to retry");
}

void drawSuccess(struct Duel *duel)
{
	drawHud(duel);
	int w=intraFontMeasureText(iFont[FONT_BODY],"Success!");
	intraFontPrint(iFont[FONT_BODY],240-w/2,50,"Success!");
	intraFontPrint(iFont[FONT_SMALL],50,100,"You beat your opponent");
//	intraFontPrint(iFont[FONT_SMALL],50,120,"Rank: grasshopper");
//	intraFontPrintf(iFont[FONT_SMALL],50,140,"Timer bonus: %d",getTimeLeft);
	intraFontPrint(iFont[FONT_SMALL],50,220,"Press [] to continue.");
}

void drawSelectAction(struct Duel *duel)
{
	drawHud(duel);
	//int w=intraFontMeasureText(iFont[FONT_BODY],"Select Action");
	//intraFontPrint(iFont[FONT_BODY],240-w/2,15,"Select Action");
	if(duel->menu!=0) 
		intraFontPrint(iFont[FONT_BODY],45,90,"Select Item");
	else 
		intraFontPrint(iFont[FONT_BODY],45,90,"Select Action");

	intraFontSetStyle(iFont[FONT_SMALL],1.0f,GU_RGBA(0xff,0xff,0xff,0xff),GU_RGBA(0,0,0,255),0);	
	intraFontSetStyle(iFont[FONT_SMALLHIGHLIGHT],1.0f,GU_RGBA(0xff,0xff,0xff,0xff),GU_RGBA(0,0,0,255),0);	
	int i,j;
	int top=duel->selected-5;
	int activeCount=0;
	for(i=0;i<actionCount;i++) {
		if(action[i].minLevel<=duel->activeWarrior->level) activeCount++;
		else break;
	}
	if(top+11>activeCount) top=activeCount-11;
	if(top<0) top=0;
	for(j=0;j<11;j++) {
		i=j+top;
		intraFont *font=iFont[FONT_SMALL];
		int offset=0;
		if(duel->selected==i) {
			font=iFont[FONT_SMALLHIGHLIGHT];
			offset=5*cosf(getGiggle());
		}
		if( action[i].minLevel>duel->activeWarrior->level ) break;
		if(action[i].hpCost>duel->activeWarrior->hp || action[i].apCost>duel->activeWarrior->ap) {
			intraFontSetStyle(font,1.0f,GU_RGBA(64,64,64,0xff),GU_RGBA(0,0,0,255),0);	
		}
		intraFontPrint(font,45+offset,110+j*12,action[i].name);
		if(action[i].hpCost>duel->activeWarrior->hp || action[i].apCost>duel->activeWarrior->ap) {
			intraFontSetStyle(font,1.0f,GU_RGBA(0xff,0xff,0xff,0xff),GU_RGBA(0,0,0,255),0);	
		}
	}
}

void drawOpponentAttack(struct Duel *duel)
{
	drawHud(duel);
	int w=intraFontMeasureText(iFont[FONT_MESSAGE],"Opponent");
	intraFontPrint(iFont[FONT_MESSAGE],240-w/2,25,"Opponent");
	w=intraFontMeasureText(iFont[FONT_MESSAGE],duel->opponentAction->name);
	intraFontPrint(iFont[FONT_MESSAGE],240-w/2,50,duel->opponentAction->name);
}

void drawActor(struct Actor *actor)
{
	if(!actor) return;
	if(!actor->character) return;
	if(!actor->character->mesh) return;
	int frameDuration=1000/animMotion[actor->motion].fps;
	float fraction=(actor->frameElapsed%frameDuration)/(float)frameDuration;
	int frame,nextFrame;
	frame=actor->lastFrame;
	nextFrame=actor->frame;

	struct AnimMesh *mesh=actor->character->mesh;
	struct Vertex3DTP *buffer=(struct Vertex3DTP *)sceGuGetMemory( sizeof(struct Vertex3DTP)*mesh->polyCount*3);
	//if(model->cullBackface==0) buffer=(struct Vertex3DTP *)(0x0fffffff&(int)buffer);
	buffer=(struct Vertex3DTP *)((int)buffer&~0x40000000);
	md2GetFractionalFrame(mesh,frame,nextFrame,fraction,buffer,actor->minMax);
	sceKernelDcacheWritebackAll();

	Image *source=mesh->texture;
	if(source) {
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);
	}
	
	sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,mesh->polyCount*3,0,buffer);
	//printf("acting\n");
}


void drawDuelSuspended(struct Duel *duel)
{
	// draw the active objects
	sceGuEnable(GU_LIGHTING);

	sceGuFrontFace(GU_CCW);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuEnable(GU_DEPTH_TEST);
	drawTerrain(duel->terrain);
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	sceGuAlphaFunc(GU_GREATER,3,0xff);
	sceGuEnable(GU_ALPHA_TEST);
	if(getItemCount()>=11) {
		//printf("positioning grass and trees!\n");
		terrainMoveItem(duel->terrain,0,128,128);
		terrainMoveItem(duel->terrain,1,-128,128);
		terrainMoveItem(duel->terrain,2,128,-128);
		terrainMoveItem(duel->terrain,3,-128,-128);
		terrainMoveItem(duel->terrain,4,192,48);
		terrainMoveItem(duel->terrain,5,-192,64);
		terrainMoveItem(duel->terrain,6,48,-192);
		terrainMoveItem(duel->terrain,7,48,192);
		terrainMoveItem(duel->terrain,8,-192,96);
		terrainMoveItem(duel->terrain,9,96,-192);
		terrainMoveItem(duel->terrain,10,192,0);
	}

	renderItem(0);	// draw the tree 2
	renderItem(1);	// draw the tree 4
	renderItem(2);	// draw the tree 5
	renderItem(3);	// draw the tree 6
	renderItem(4);	// draw the grass 1
	renderItem(5);	// draw the grass 2
	renderItem(6);	// draw the grass 3
	renderItem(7);	// draw the grass 4
	renderItem(8);	// draw the grass 5
	renderItem(9);	// draw the grass 6
	renderItem(10);	// draw the grass scatt

	sceGuDisable(GU_ALPHA_TEST);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDisable(GU_LIGHTING);


	ScePspFMatrix4 oldView=view;
	sceGuFrontFace(GU_CW);
	sceGumPushMatrix();
	ScePspFVector3 warriorPos={0,28,-48};	// okay, so really the floor
	sceGumTranslate(&warriorPos);
	gumTranslate(&view,&warriorPos);
	sceGumRotateY(GU_PI);	// facing each other
	gumRotateY(&view,GU_PI);
	drawActor(duel->activeWarrior);
	// draw any particle effects
	drawParticles(duel->warriorParticles);
	drawParticles(duel->warriorParticles+1);
	sceGumPopMatrix();

	view=oldView;
	sceGumPushMatrix();
	ScePspFVector3 opponentPos={0,28,48};	// okay, so really the floor
	sceGumTranslate(&opponentPos);
	gumTranslate(&view,&warriorPos);
	drawActor(duel->activeOpponent);
	// draw any particle effects
	drawParticles(duel->opponentParticles);
	drawParticles(duel->opponentParticles+1);
	//drawTestBillboard();
	sceGumPopMatrix();

	view=oldView;
	//drawParticles(&testEmitter);

	sceGuFrontFace(GU_CCW);
	
}

void drawDuel(struct Duel *duel)
{
	drawDuelSuspended(duel);
	
	// Now draw the overlays, and text.
	//if(duel->timeLeft>0) {
		//intraFontPrintf(iFont[FONT_BODY],300,265,"Time left: %d",duel->timeLeft/1000);
	//}
	//int width=(int)intraFontMeasureText(iFont[FONT_HEADLINE],"HardHat Battle Arena");
	//intraFontPrint(iFont[FONT_HEADLINE],240-width/2,15,"HardHat Battle Arena");
	//intraFontPrintf(iFont[FONT_SMALL],10,250,"Score: %d",duel->score);
	if(duel->mode==DM_ADDTEAMMATE) drawAddTeammate(duel);
	else if(duel->mode==DM_CHOOSETEAMMATE) drawChooseTeammate(duel);
	else if(duel->mode==DM_SWITCHTEAMMATE) drawSwitchTeammate(duel);
	else if(duel->mode==DM_SELECTACTION) drawSelectAction(duel);
	else if(duel->mode==DM_ATTACK) drawAttack(duel);
	else if(duel->mode==DM_OPPONENTATTACK) drawOpponentAttack(duel);
	else if(duel->mode==DM_SUCCESS) drawSuccess(duel);
	else if(duel->mode==DM_FAILURE) drawFailure(duel);
}

void setActorLevel(struct Actor *a,struct Character *character,int level)
{
	memset(a,sizeof(struct Actor),0);
	strcpy(a->name,character->name);
	a->character=character;
	a->level=level;
	a->xp=getNextLevel(level-1);
	a->hpMax=12;
	a->apMax=5;
	a->attack=10;
	a->defense=12;
	int i;
	for(i=1;i<level;i++) {
		a->hpMax+=roll("5d2");
		a->apMax+=2;
		a->attack+=roll("1d2");
		a->defense+=roll("1d2");
	}
	resetActor(a);
}

void chooseWarrior(struct Actor *a,int id,int level)
{
	strcpy(a->name,character[id].name);
	a->xp=0;
	a->attack=10;
	a->defense=14;
	a->boost=0;
	a->hpMax=12;
	a->hp=12;
	a->hpOld=12;
	a->apMax=5;
	a->ap=5;
	a->apOld=5;
	a->oldTimer=0;
	a->character=&character[id];
	a->frame=0;
	a->lastFrame=0;
	a->frameElapsed=0;
	a->motion=MT_STAND;
	a->level=level;
	setActorLevel(a,a->character,level);
}

void addTeammate(struct Team *team,struct Character *character,int level)
{
	if(team->actorCount>=MAXTEAM) {
		const char *message="No empty actor slots for team!";
		addMessage(message,5000);
		printf("%s\n",message);
		return;
	}
	struct Actor *a=team->actor+team->actorCount;
	team->actorCount++;
	setActorLevel(a,character,level);
}

void addOneTeammate(struct Duel *duel)
{
	struct Team *team=duel->team;
	printf(">>> adding one teammate from %d to %d\n",team->targetCount,team->actorCount+1);
	team->targetCount=team->actorCount+1;
}

int handleAddTeammate(struct Duel *duel,enum Buttons button)
{
	struct Team *team=duel->team;
	switch(button) {
	case BT_UP:
		playSfx(SFX_MENUNEXT);
		duel->selected--;
		if(duel->selected<0) duel->selected=characterCount-1;
		break;
	case BT_DOWN:
		playSfx(SFX_MENUNEXT);
		duel->selected++;
		if(duel->selected>=characterCount) duel->selected=0;
		break;
	case BT_LEFT:
		playSfx(SFX_MENUNEXT);
		duel->selected-=duel->charactersAcross;
		if(duel->selected<0) duel->selected=characterCount-1;
		break;
	case BT_RIGHT:
		playSfx(SFX_MENUNEXT);
		duel->selected+=duel->charactersAcross;
		if(duel->selected>=characterCount) duel->selected=0;
		break;
	case BT_CROSS:
		playSfx(SFX_MENUSELECT);

		chooseWarrior(&team->actor[team->actorCount++],duel->selected,duel->opponentLevel);
		duel->warrior[0]=&team->actor[team->actorCount-1];
		duel->warrior[1]=0;
		duel->warrior[2]=0;

		duel->selected=0;
		duel->step=0;
		duel->activeWarrior=duel->warrior[0];

		{
			int ch;
			int unique;
			do {
				ch=(rand()>>16)%characterCount;
				unique=1;	// assume unique
				int i;
				for(i=0;i<team->actorCount;i++) {
					if(strcmp(team->actor->character->name,character[ch].name)==0) unique=0;
				}
			} while(unique==0);
			chooseWarrior(duel->opponent+duel->step,ch,duel->opponentLevel);
		}
		duel->activeOpponent=duel->opponent+0;

		if((team->actorCount+1)/2 > 1) {
			duel->mode=DM_CHOOSETEAMMATE;
			duel->selected=0;
			duel->step=1;
		} else {
			duel->mode=DM_SELECTACTION;
			loadCharacters(0,0);
			playSong((rand()>>16)&1);
		}
		loadCharacters(duel->activeWarrior->character,duel->activeOpponent->character);
		break;
	default:
		break;
	}
	return 0;
}

int handleChooseTeammate(struct Duel *duel,enum Buttons button)
{
	struct Team *team=duel->team;
	switch(button) {
	case BT_UP:
	case BT_LEFT:
		playSfx(SFX_MENUNEXT);
		duel->selected--;
		if(duel->selected<0) duel->selected=duel->team->actorCount-1;
		break;
	case BT_RIGHT:
	case BT_DOWN:
		playSfx(SFX_MENUNEXT);
		duel->selected++;
		if(duel->selected>=duel->team->actorCount) duel->selected=0;
		break;
	case BT_CROSS:
		playSfx(SFX_MENUSELECT);
		if(duel->step<(duel->team->actorCount+1)/2) {
			duel->warrior[duel->step]=duel->team->actor+duel->selected%duel->team->actorCount;
			resetActor(duel->warrior[duel->step]);
			chooseWarrior(duel->opponent+duel->step,(rand()>>16)%characterCount,duel->opponentLevel);
			duel->step++;
			if(duel->step>=(duel->team->actorCount+1)/2) duel->selected=-1;
		} 
		if(duel->step==(team->actorCount+1)/2) {
			duel->mode=DM_SELECTACTION;
			loadCharacters(0,0);
			playSong((rand()>>16)&1);
			duel->selected=0;
			duel->menu=0;
			duel->step=0;
			duel->activeWarrior=duel->warrior[0];
			duel->activeOpponent=duel->opponent+0;
			loadCharacters(duel->activeWarrior->character,duel->activeOpponent->character);
		}
		break;
	case BT_CIRCLE:
		duel->mode=DM_LOST;
		break;
	default:
		break;
	}
	return 0;
}

int handleSwitchTeammate(struct Duel *duel,enum Buttons button)
{
	//struct Team *team=duel->team;
	switch(button) {
	case BT_UP:
	case BT_LEFT:
		playSfx(SFX_MENUNEXT);
		duel->selected--;
		if(duel->selected<0) duel->selected=(duel->team->actorCount+1)/2-1;
		break;
	case BT_RIGHT:
	case BT_DOWN:
		playSfx(SFX_MENUNEXT);
		duel->selected++;
		if(duel->selected>=(duel->team->actorCount+1)/2) duel->selected=0;
		break;
	case BT_CROSS:
		playSfx(SFX_MENUSELECT);
		duel->mode=DM_SELECTACTION;
		duel->selected=0;
		duel->menu=0;
		duel->step=duel->selected;
		duel->activeWarrior=duel->warrior[duel->step];
		//duel->activeOpponent=duel->opponent+0;
		loadCharacters(duel->activeWarrior->character,duel->activeOpponent->character);
		break;
	default:
		break;
	}
	return 0;
}

int handleSelectAction(struct Duel *duel,enum Buttons button)
{
	int activeCount=0;
	int i;
	for(i=0;i<actionCount;i++) {
		if(action[i].minLevel<=duel->activeWarrior->level) activeCount++;
		else break;
	}
	
	switch(button) {
	case BT_UP:
		playSfx(SFX_MENUNEXT);
		duel->selected--;
		if(duel->selected<0) duel->selected=activeCount-1;
		break;
	case BT_DOWN:
		playSfx(SFX_MENUNEXT);
		duel->selected++;
		if(duel->selected>=activeCount) duel->selected=0;
		break;
	case BT_CROSS:
		playSfx(SFX_MENUSELECT);
		if(action[duel->selected].hpCost>duel->activeWarrior->hp || action[duel->selected].apCost>duel->activeWarrior->ap  || action[duel->selected].minLevel>duel->activeWarrior->level) {
			duel->selected++;
			if(duel->selected>=actionCount) duel->selected=0;
			break;
		}
		duel->mode=DM_ATTACK;
		setTimeLeft(4000);
		duel->warriorAction=action+duel->selected;
		//sourceDesc.which=(sourceDesc.which+1)%16;
		{
			struct ParticleDescription *desc=duel->warriorAction->sourceDesc;
			if(desc) newParticle(duel->warriorParticles,desc,0,32,0);
		}
		printf("Attacking now.\n");
		// Game AI for opponent. :-P
		int specialCount=0;
		struct Action *special[64];
		for(i=0;i<actionCount;i++) {
			if(action[i].alignment!=duel->alignment) continue;
			if(action[i].hpCost>duel->activeOpponent->hp || action[i].apCost>duel->activeOpponent->ap || action[i].minLevel>duel->activeOpponent->level ) continue;
			special[specialCount++]=action+i;
		}
		if(specialCount>0 && rand()&65536) {	// fighting style is 50% of the time
			int a=(rand()>>16)%specialCount;
			duel->opponentAction=special[a];
		} else {
			do {
				duel->opponentAction=action+((rand()>>16)%actionCount);
			} while (duel->opponentAction->hpCost>duel->activeOpponent->hp || duel->opponentAction->apCost>duel->activeOpponent->ap || duel->opponentAction->minLevel>duel->activeOpponent->level );
		}

		actorMotion(duel->activeWarrior,duel->warriorAction->warriorMotion);
		break;
	default:
		break;
	}
	return 0;
}

int handleAttack(struct Duel *duel,enum Buttons button)
{
	FILE *file;
	switch(button) {
        case BT_START:
                if(duel->activeWarrior->attack>=999 || duel->activeWarrior->defense>=999) {
                        duel->activeWarrior->level=1;
                        duel->activeWarrior->attack=10;
                        duel->activeWarrior->defense=12;
                        levelUp(duel->activeWarrior);
                        addMessage("Cheat repaired.\n",2000);
                }
                break;
        case BT_TRIANGLE:
                file=fopen("cheats","r");
                if(file) {
                        fclose(file);
                        duel->activeWarrior->attack=999;
                        duel->activeWarrior->defense=999;
                        addMessage("Cheat activated.  Disable with START",2000);
                }
		break;
	case BT_CROSS:
		if(getTimeLeft()>2100) {
			playSfx(SFX_MENUSELECT);
			setTimeLeft(2100);	// make sure we trigger the attack.
		} else if(getTimeLeft()<1999) {
			playSfx(SFX_MENUSELECT);
			setTimeLeft(0);	// skip the fancy animation
		}
		break;
	default:
		break;
	}
	return 0;
}


int handleSuccess(struct Duel *duel,enum Buttons button)
{
	switch(button) {
	case BT_SQUARE:
		playSfx(SFX_MENUSELECT);
		duel->warriorCount=(duel->team->actorCount+1)/2;
		if(duel->step+1<duel->warriorCount) {
			duel->mode=DM_SELECTACTION;
			duel->selected=0;
			duel->menu=0;
			duel->step++;
			duel->activeOpponent=duel->opponent+duel->step;
			loadCharacters(duel->activeWarrior->character,duel->activeOpponent->character);
		} else {
			duel->selected=0;
			duel->warrior[0]=0;
			duel->warrior[1]=0;
			duel->warrior[2]=0;
			duel->step=0;
			duel->mode=DM_GAMEOVER;
		}
		break;
	default:
		break;
	}
	return 0;
}

int handleFailure(struct Duel *duel,enum Buttons button)
{
	switch(button) {
	case BT_CIRCLE:
		playSfx(SFX_MENUSELECT);
		duel->warriorCount=(duel->team->actorCount+1)/2;
		int i;
		for(i=0;i<duel->warriorCount;i++) {
			duel->activeWarrior=duel->warrior[duel->step];
			if(duel->activeWarrior->hp>0) break;
		}
		if(duel->activeWarrior->hp>0) {
			duel->mode=DM_SELECTACTION;
			duel->selected=0;
			duel->menu=0;
			loadCharacters(duel->activeWarrior->character,duel->activeOpponent->character);
		} else {
			duel->mode=DM_LOST;
			duel->step=0;
		}
		break;
	default:
		break;
	}
	return 0;
}

int handleDuel(struct Duel *duel,enum Buttons button)
{
	if(duel->mode==DM_ADDTEAMMATE) return handleAddTeammate(duel,button);
	else if(duel->mode==DM_CHOOSETEAMMATE) return handleChooseTeammate(duel,button);
	else if(duel->mode==DM_SWITCHTEAMMATE) return handleSwitchTeammate(duel,button);
	else if(duel->mode==DM_SELECTACTION) return handleSelectAction(duel,button);
	else if(duel->mode==DM_ATTACK) return handleAttack(duel,button);
	else if(duel->mode==DM_OPPONENTATTACK) return handleAttack(duel,button);
	else if(duel->mode==DM_SUCCESS) return handleSuccess(duel,button);
	else if(duel->mode==DM_FAILURE) return handleFailure(duel,button);
	return 0; //duel->mode!=DM_GAMEOVER?DS_ACTIVE:DS_WIN;
}

int getActionId(struct Action *act)
{
	if(!act) return 0;
	return act->id;
}

struct Action *lookUpActionId(int id)
{
	if(id<1 || id>actionCount) {
		return 0;
	}
	return action+id-1;
}
