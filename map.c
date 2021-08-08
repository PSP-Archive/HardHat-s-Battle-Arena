#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>

#include "main.h"
#include "ase.h"
#include "particle.h"
#include "map.h"
#include "duel.h"

//enum PlaceType { PT_WHITE,PT_YELLOW,PT_RED,PT_BLUE,PT_BLACK,PT_GREEN, PT_CHEST,PT_CRATE,PT_GRASS,PT_TREE,PT_ARIDTREE } type;
const char *placeTypeName[]={ "White","Yellow","Red","Blue","Black","Green","Chest","Crate","Grass","Tree","Arid Tree" };
int placeItemId[12]={11,12,13,14,15,16, 17,18, 4,2,3};
int grassItemId[7]={4,5,6,7,8,9,10};
int aridTreeItemId[2]={3,1};
int treeItemId[3]={0,1,2};

const char *shardName[]={"Diamond Shard","Yellow Onyx Shard","Ruby Shard","Blue Garnet Shard","Black Obsidian Shard","Green Emerald Shard" };
const char *itemName[]={"Mini Healing Potion","Healing Potion","Mega Healing Potion",
	"Strengthing Potion","Defensive Potion",
	"Mini Action Point Salve","Action Point Salve","Mega Action Point Salve",
	"Mini Pill of Experience","Pill of Experience",	"Mega Pill of Experience",
	};
extern int shardId;
extern int potionId;
extern int salveId;
extern int pillId;

struct AnimMesh *brickMesh;	
const char *brickTextureName[]={"models/brick/doctor.png","models/brick/yellow.png","models/brick/red.png","models/brick/pirate.png",
"models/brick/comm.png","models/brick/green.png"};
const char *brickIconName[]={"models/brick/doctor_ii.png","models/brick/yellow_ii.png","models/brick/red_ii.png","models/brick/pirate_ii.png",
"models/brick/comm_ii.png","models/brick/green_ii.png"};

struct PlaceAvatar {
	int itemId;
	float rotateY;
};

#define PF_VISIBLE 1	// Is it available yet?
#define PF_VISITED 2	// Is it new?
#define PF_MASTERED 4	// Has it been beaten/opened/smashed
#define PF_TOURNAMENT 8	// Is there a tournament available there?
struct Place {
	float x,y,z;
	enum PlaceType type;
	int level;	// what level of creatures you'll find there.
	int flags;
	enum PlaceType keyRequired;	// only used by chests
	char contains[48];		// what's in it.
	struct PlaceAvatar avatar;
	float dist;	// how far away from the the welcome centre this is, for sorting purposes.
};

extern struct AnimMotion animMotion[];
extern const char *animMotionName[];

#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_DOWN 4
#define DIR_LEFT 8

struct MapActor {
	char name[128];	// name of the creature
	struct AnimMesh *mesh;
	Image *texture;
	Image *icon;
	float x,y,z;
	int direction;	// force applied by the dpad to dx/dy
	float dx,dy;	// direction/speed
	float ax,ay;	// analog stick direction/speed
	float angle;

	// animation part
	enum MotionType motion;	// automatically goes back to STANDING when done an animation
	enum MotionType oldMotion;	// just for debugging output.
	int lastFrame;
	int frame;
	int frameElapsed;	
};

struct Map {
	// important
	struct Place *place;
	int placeCount;
	int scale;
	int activePlace;
	MapState state;

	// transient
	struct MapActor actor;
	float cameraAngle;
	int angleDir;
	struct MapActor guide[6];	// helpful person who guides you on your journey.
	
	short *elev;
	int width;
	int height;
	struct Terrain *terrain;
	struct ParticleEmitter *particleEmitter;
	struct ParticleEmitter particleSelectedEmitter;
	int guideMode;	// are we showing advice from a local guide.
	int popupMenu;
	// exploding crate
	int explodingCrateTimer;
	float explodingCrate[3];
	struct AseScene *explodingCrateScene;
	// found item
	int foundItemTimer;
	int foundItemId;
	float foundItem[3];
};
struct ParticleDescription mapDesc={45000,20,1000,0.5f,{0,25,0},1.0f,{0,-20,0},0.0f/*6.28f*/,40,5,0,0.0f,0xffffff80,0xffe0e0e0};
struct ParticleDescription mapSelectedDesc={45000,20,500,0.5f,{0,50,0},1.0f,{0,-20,0},0.0f/*6.28f*/,40,10,7,0.0f,0xffff0080,0xff8000ff};
struct ParticleDescription mapChestDesc={45000,40,500,0.5f,{0,50,0},1.0f,{0,-20,0},0.0f/*6.28f*/,40,5,5,0.0f,0xff008080,0xff00ffff};
struct ParticleDescription mapCrateDesc={45000,40,500,0.5f,{0,50,0},1.0f,{0,-20,0},0.0f/*6.28f*/,40,5,3,0.0f,0xff406080,0xff007fcf};

struct Guide {
	int level;
	PlaceType place;
	char *toolow;
	char *toohigh;
	char *notmastered;
	char *mastered;
} guide[42];	// really 36
int guideCount=0;

int cmpPlace(const void *one,const void *two)
{
	struct Place *a=(struct Place *)one,*b=(struct Place *)two;
	//printf("comparing %08x to %08x\n",(int)a,(int)b);
	//printf("a->level=%d, b->level=%d\n",a->level,b->level);

	if(a->type>PT_CRATE && b->type<=PT_CRATE) return 1;
	if(a->type<=PT_CRATE && b->type>PT_CRATE) return -1;

	if(a->level<b->level && a->level!=0) return -1;
	if(a->level>b->level && b->level!=0) return 1;

	//printf("a->type=%d, b->type=%d\n",a->type,b->type);
	if(a->type<b->type) return -1;	
	if(a->type>b->type) return 1;

	//printf("a->type=%f, b->type=%f\n",a->dist,b->dist);
	// same type.
	return a->dist==b->dist?0:a->dist<b->dist?-1:1;
}

void teleportActor(struct MapActor *actor,struct Place *p)
{
	actor->x=p->x-32;
	actor->y=p->y;
	actor->z=p->z;
	float *min=getItemMin(p->avatar.itemId);
	float *max=getItemMax(p->avatar.itemId);
	if(min && actor->z<p->z+max[2]) actor->x=p->x+min[0]-32;
}

void loadGuide()
{
	FILE *file;
	file=fopen("data/guide.txt","r");
	if(!file) return;
	int i=0;
	char buf[2048];
	struct Guide *g=guide;
	while( fgets(buf,2047,file)) {
		buf[2047]=0;
		if(strchr(buf,'\n')) strchr(buf,'\n')[0]=0;
		if(buf[0]==0) continue;
		if(buf[0]=='[') {
			if(i>0) g++;
			i++;
			guideCount++;
		} else if(buf[0]=='#') {
			continue;
		} else if( strncmp(buf,"level=",6)==0) {
			g->level=atol(buf+6);
		} else if( strncmp(buf,"toolow=",7)==0) {
			g->toolow=strdup(buf+7);
		} else if( strncmp(buf,"toohigh=",8)==0) {
			g->toohigh=strdup(buf+8);
		} else if( strncmp(buf,"notmastered=",12)==0) {
			g->notmastered=strdup(buf+12);
		} else if( strncmp(buf,"mastered=",9)==0) {
			g->mastered=strdup(buf+9);
		} else {
			printf("guide.txt: Couldn't understand line: '%s'\n",buf);
		}
	}
	printf("Read guide texts for %d different levels\n",i);
	fclose(file);
}

struct Map *initMap()
{
	struct Map *map=(struct Map *)calloc(sizeof(struct Map),1);
	
	if(!map->explodingCrateScene) map->explodingCrateScene=loadAseScene("box");
	map->terrain=terrainDetailInit("data/mapelev.png","data/mapcolor.png");
	// init the map locations.
	Image *loc=loadPng("data/maptraining.png");
	Image *elev=loadPng("data/mapelev.png");
	if(!loc) return 0;
	int cx=0,cy=0;
	int x,y;
	int count=0;
	int decCount=0;
	for(y=0;y<loc->imageHeight;y++) {
		for(x=0;x<loc->imageWidth;x++) {
			Color c=loc->data[x+y*loc->textureWidth];
			if(c==0xff808080) continue;
			if(c==0xff008000 || c==0xff800080) decCount++;
			count++;
			if(c==0xffffffff) {
				cx=x;
				cy=y;
			}
		}
	}
	map->placeCount=count;
	if(count<1) return 0;
	map->place=(struct Place *)calloc(sizeof(struct Place),count);
	// now we know how many locations there are, and also where on the island we start.
	map->height=loc->imageHeight;
	map->width=loc->imageWidth;
	map->scale=32;
	map->elev=(short *)malloc(map->width*map->height*sizeof(short));
	count=0;
	for(y=0;y<loc->imageHeight;y++) {
		for(x=0;x<loc->imageWidth;x++) {
			Color c=loc->data[x+y*loc->textureWidth];
			int height=elev->data[x+elev->textureWidth*y]&255;
			height=(height-128);
			map->elev[x+map->width*y]=height;
			if(c==0xff808080) continue;
			map->place[count].flags=0;	// new, invisible, no tournament yet.
			map->place[count].level=0;	// last step.
			enum PlaceType type=PT_WHITE;
			switch(c) {
			case 0xffffffff: type=PT_WHITE; break;
			case 0xff0000ff: type=PT_RED; break;
			case 0xff00ff00: type=PT_GREEN; break;
			case 0xff00ffff: type=PT_YELLOW; break;
			case 0xffff0000: type=PT_BLUE; break;
			case 0xff000000: type=PT_BLACK; break;

			case 0xff008080: type=PT_CRATE; break;
			case 0xff800000: type=PT_CHEST; break;
			case 0xff000080: type=PT_ARIDTREE; break;
			case 0xff008000: type=PT_TREE; break;
			case 0xff800080: type=PT_GRASS; break;
			}
			
			//printf("color %08x mapped to place type %d (\"%s\")\n",c,type,placeTypeName[type]);
			
			map->place[count].type=type;
			if(map->place[count].type>PT_WHITE) map->place[count].flags|=PF_VISIBLE|PF_VISITED;
			if(map->place[count].type==PT_WHITE) map->place[count].flags|=PF_VISIBLE;
			if(map->place[count].type==PT_BLACK) map->place[count].flags|=PF_TOURNAMENT;
			map->place[count].contains[0]=0;
			map->place[count].x=(x-loc->imageWidth/2)*map->scale;
			map->place[count].z=(y-loc->imageHeight/2)*map->scale*2;
			map->place[count].y=height;
			map->place[count].dist=sqrtf((x-(float)cx)*(x-cx)+(y-(float)cy)*(y-cy));
			map->place[count].avatar.itemId=placeItemId[type];
			map->place[count].avatar.rotateY=0;
			const char *contains="";
			if((rand()>>16)&1) {
				contains=shardName[(rand()>>16)%6];
			} else {
				contains=itemName[(rand()>>16)%11];
			}
			strcpy(map->place[count].contains,contains);
			if(type==PT_GRASS) {
				static int grassId=0;
				map->place[count].avatar.itemId=grassItemId[(grassId++)%6];
				map->place[count].avatar.rotateY=rand()/(float)RAND_MAX*GU_PI*2;
			} else if(type==PT_TREE) {
				static int treeId=0;
				map->place[count].avatar.itemId=treeItemId[(treeId++)%3];
				map->place[count].avatar.rotateY=rand()/(float)RAND_MAX*GU_PI*2;
			} else if(type==PT_ARIDTREE) {
				static int aridTreeId=0;
				map->place[count].avatar.itemId=aridTreeItemId[(aridTreeId++)%2];
				map->place[count].avatar.rotateY=rand()/(float)RAND_MAX*GU_PI*2;
			}
			count++;
		}
	}
	if(elev) freeImage(elev);
	if(loc) freeImage(loc);
	printf("Sorting place %08x, with %d items\n",(int)map->place,map->placeCount);
	qsort(map->place,map->placeCount,sizeof(struct Place),cmpPlace);
	int typeLevel[]={1,3,5,7,9,11,0,0,0,0,0,0,0,0,0,0};
	int i;
	for(i=0;i<map->placeCount;i++) {
		map->place[i].level=typeLevel[map->place[i].type];
		if(map->place[i].type<=PT_GREEN) typeLevel[map->place[i].type]+=12;
	}
	qsort(map->place,map->placeCount,sizeof(struct Place),cmpPlace);
#if 0
	FILE *out=fopen("log","w");
	for(i=0;i<map->placeCount;i++) {
		fprintf(out,"place %d: type='%s', level=%d\n",i,placeTypeName[map->place[i].type],map->place[i].level);
	}
	fclose(out);
#endif
	map->place[0].flags|=PF_VISIBLE;
	strcpy(map->actor.name,"Hero");
	teleportActor(&map->actor,map->place+0);
	for(i=0;i<6;i++) {
		strcpy(map->guide[i].name,placeTypeName[i]);
		strcat(map->guide[i].name," Mage Master");
		map->guide[i].angle=GU_PI;
		map->guide[i].texture=loadPng(brickTextureName[i]);
		if(!map->guide[i].texture) printf("--- Could not locate '%s'.  What shall I do?\n",brickTextureName[i]);
		swizzleFast(map->guide[i].texture);
		map->guide[i].icon=loadPng(brickIconName[i]);
		swizzleFast(map->guide[i].icon);
		teleportActor(map->guide+i,map->place+i);
		map->guide[i].z-=32;
		if(i==PT_RED) {
			map->guide[i].z+=32;
			map->guide[i].x+=32;
		}
	}
	
	map->state=MS_ACTIVE;
	if(!map->particleEmitter) {
		map->particleEmitter=(struct ParticleEmitter *)calloc(sizeof(struct ParticleEmitter),map->placeCount);
	}
	
	// load in the Guide text from a file.
	if(guideCount==0) loadGuide();
	//sceKernelDelayThread(100000000);
	return map;
}

float getMapElev(struct Map *map,float x,float y)
{
	if(!map) return 0;
	int gridx=floor(x/map->scale),gridy=floor(y/(map->scale*2));
	float fx=x/map->scale-gridx,fy=y/(map->scale*2)-gridy;
	gridx+=map->width/2;
	gridy+=map->height/2;
	if(gridx<0 || gridx+1>=map->width) gridx=0;
	if(gridy<0 || gridy+1>=map->height) gridy=0;
	int a=map->elev[gridx+map->width*gridy],
		b=map->elev[gridx+1+map->width*gridy],
		c=map->elev[gridx+map->width*(gridy+1)],
		d=map->elev[gridx+1+map->width*(gridy+1)];
	float rc=d*fx*fy+c*(1-fx)*fy+b*fx*(1-fy)+a*(1-fx)*(1-fy);
	//printf("Elev for %f,%f is %f;",x,y,rc);
	//printf(" %d,%d==%d (%f,%f)\n",gridx,gridy,map->elev[gridx+map->width*gridy],fx,fy);
	return rc;
}

void freeMap(struct Map *map)
{
	if(!map) return;
	if(map->terrain) freeTerrain(map->terrain);
	map->terrain=0;
	if(map->place) free(map->place);
	map->place=0;
	if(map->elev) free(map->elev);
	map->elev=0;
	free(map);
}

int collideSphereRect(float radius,float x,float y,float rect[4])
{
	if(x+radius<rect[0]) return 0;
	if(y+radius<rect[1]) return 0;
	if(x-radius>rect[2]) return 0;
	if(y-radius>rect[3]) return 0;
	//so, we are close or intersecting.
	
	return 1;
}

struct Place *collideSphereMap(struct Map *map,float radius,float x,float y)
{
	int i;
	for(i=0;i<map->placeCount;i++) {
		struct Place *p=map->place+i;
		if(p->type==PT_GRASS) continue;
		if(!(p->flags & PF_VISIBLE)) continue;
		if(i<6) {	// colide with guide
			struct MapActor *g=map->guide+i;
			float rect[4]={g->x-12,g->z-12,g->x+12,g->z+12};
			int result=collideSphereRect(radius,x,y,rect);
			if(result) {
				//printf("Guide collision detected.\n");
				return p;
			}
		}		
		if(p->type==PT_TREE || p->type==PT_ARIDTREE) {
			float rect[4];
			rect[0]=p->x-2;
			rect[1]=p->z-2;
			rect[2]=p->x+2;
			rect[3]=p->z+2;
			int result=collideSphereRect(radius,x,y,rect);
			if(result) {
				//printf("Tree collision detected.\n");
				return p;
			}
		} else {
			float rect[4];
			float *min=getItemMin(p->avatar.itemId),*max=getItemMax(p->avatar.itemId);
			
			if(!min || !max) continue;
			rect[0]=p->x+min[0];
			rect[1]=p->z+min[2];
			rect[2]=p->x+max[0];
			rect[3]=p->z+max[2];
			int result=collideSphereRect(radius,x,y,rect);
			if(result) {
				//printf("%s collision detected.\n",placeTypeName[p->type]);
				return p;
			}
		}
	}
	return 0;	// it's all good.
}

void slidingPlane(float *xx,float *yy,float tx,float ty,float radius,struct Place *p)
{
	float rect[4];
	float x=*xx,y=*yy;
	if(p->type==PT_TREE || p->type==PT_ARIDTREE) {
		rect[0]=p->x-2;
		rect[1]=p->z-2;
		rect[2]=p->x+2;
		rect[3]=p->z+2;
	} else {
		float *min=getItemMin(p->avatar.itemId),*max=getItemMax(p->avatar.itemId);
		
		if(!min || !max) return;
		rect[0]=p->x+min[0];
		rect[1]=p->z+min[2];
		rect[2]=p->x+max[0];
		rect[3]=p->z+max[2];
	}
	// now find which plane to slide along.
	printf("x: %.2f rect[0]=%.2f rect[2]=%.2f radius: %.2f tx=%.2f; ",x,rect[0],rect[2],radius,tx);
	if(x+radius<rect[0] && x+radius+tx>rect[0]) x=rect[0]-radius;
	else if(x-radius>rect[2] && x-radius+tx<rect[2]) x=rect[2]+radius;
	else x+=tx;
	printf("output x: %.2f\n",x);
	printf("y: %.2f rect[1]=%.2f rect[3]=%.2f radius: %.2f ty=%.2f; ",y,rect[1],rect[3],radius,ty);
	if(y+radius<rect[1] && y+radius+ty>rect[1]) y=rect[1]-radius;
	else if(y-radius>rect[3] && y-radius+ty<rect[3]) y=rect[3]+radius;
	else y+=ty;
	printf("output y: %.2f\n",y);

	xx[0]=x;
	yy[0]=y;
}


//#define DEBUG_ANGLE 1
#ifdef DEBUG_ANGLE
float debugDX,debugDY,debugTX,debugTY, debugNewAngle;
#endif

void updateMapActor(struct Map *map,struct MapActor *actor,int elapsed)
{
	actor->dx=actor->dx*0.80+0.20*((actor->direction&DIR_LEFT?-100:0)+(actor->direction&DIR_RIGHT?100:0));
	actor->dy=actor->dy*0.80+0.20*((actor->direction&DIR_UP?-100:0)+(actor->direction&DIR_DOWN?100:0));
	if(actor->dx>-0.05f && actor->dx<0.05f) actor->dx=0;	// round tiny values.
	if(actor->dy>-0.05f && actor->dy<0.05f) actor->dy=0;	// round tiny values.
	float dx=(actor->dx+actor->ax)*elapsed/1000.0f;
	float dy=(actor->dy+actor->ay)*elapsed/1000.0f;
	float tx=dx*cosf(map->cameraAngle)+dy*sinf(map->cameraAngle);
	float ty=-dx*sinf(map->cameraAngle)+dy*cosf(map->cameraAngle);
#ifdef DEBUG_ANGLE
	debugDX=dx;
	debugDY=dy;
	debugTX=tx;
	debugTY=ty;
#endif

	float y=getMapElev(map,actor->x+tx,actor->z+ty);
	struct Place *collision=collideSphereMap(map,12,actor->x+tx,actor->z+ty);
	if(y>-128 && !collision) {		// no walking on water! should really slide I guess.
		actor->x+=tx;
		actor->z+=ty;
		actor->y=y;
	} else if(collision) {
		// apply sliding plane.
		//slidingPlane(&actor->x,&actor->z,tx,ty,20,collision);
		actor->y=getMapElev(map,actor->x,actor->z);
	}
	if(dx!=0 || dy!=0) {
		float newAngle;
		newAngle=atan2f(-(float)tx,-(float)ty);		
#ifdef DEBUG_ANGLE
		debugNewAngle=newAngle;
#endif
		while(newAngle<=actor->angle-GU_PI) newAngle+=2*GU_PI;
		while(newAngle>actor->angle+GU_PI) newAngle-=2*GU_PI;
		actor->angle=actor->angle*0.95f+newAngle*0.05f;
		while(newAngle<=map->cameraAngle-GU_PI) newAngle+=2*GU_PI;
		while(newAngle>map->cameraAngle+GU_PI) newAngle-=2*GU_PI;
		float cameraDelta=newAngle-map->cameraAngle;
		if(cameraDelta<0) cameraDelta= -cameraDelta;
		if(cameraDelta<GU_PI-0.1f) {
			map->cameraAngle=map->cameraAngle*0.99f+newAngle*0.01f;
		}
		// renormalize actor->angle and map->cameraAngle:
		while(actor->angle<=-GU_PI) actor->angle+=2*GU_PI;
		while(actor->angle>GU_PI) actor->angle-=2*GU_PI;
		while(map->cameraAngle<=-GU_PI) map->cameraAngle+=2*GU_PI;
		while(map->cameraAngle>GU_PI) map->cameraAngle-=2*GU_PI;
	}

	if(actor->oldMotion!=actor->motion) {
		//printf("character %s now doing motion %s.\n",actor->name,animMotionName[actor->motion]);
		actor->oldMotion=actor->motion;
	}

	actor->frameElapsed+=elapsed;
	if(dx==0 && dy==0) {
		if(actor->motion!=MT_STAND) {
			actor->motion=MT_STAND;
			actor->frameElapsed=0;
		}
	} else {
		if(actor->motion!=MT_RUN) {
			actor->motion=MT_RUN;
			actor->frameElapsed=0;
		}
	}
	
	// animate the MD2
	struct AnimMotion *motion=animMotion+actor->motion;
	int frameMod=actor->frameElapsed/(1000/motion->fps);
	frameMod+=motion->startFrame;
	if(frameMod>motion->endFrame) {
		frameMod=motion->startFrame;
		actor->frameElapsed=0;
		// we always cycle whatever motion.
	}
	if(frameMod!=actor->frame) {
		actor->lastFrame=actor->frame;
		actor->frame=frameMod;
	}
}

void drawMapActor(struct MapActor *actor)
{
	if(!actor) return;
	if(!actor->mesh) {
		if(brickMesh) {
			actor->mesh=brickMesh;
		} else {
			actor->mesh=md2Load("models/brick/tris.md2");
			brickMesh=actor->mesh;
		}
		if(!actor->texture) {
			actor->texture=loadPng("models/brick/blue.png");
		}
		if(actor->texture) swizzleFast(actor->texture);
	}
	if(!actor->mesh) return;
	int frameDuration=1000/animMotion[actor->motion].fps;
	float fraction=(actor->frameElapsed%frameDuration)/(float)frameDuration;
	int frame,nextFrame;
	frame=actor->lastFrame;
	nextFrame=actor->frame;

	struct AnimMesh *mesh=actor->mesh;
	struct Vertex3DTP *buffer=(struct Vertex3DTP *)sceGuGetMemory( sizeof(struct Vertex3DTP)*mesh->polyCount*3);
	//if(model->cullBackface==0) buffer=(struct Vertex3DTP *)(0x0fffffff&(int)buffer);
	buffer=(struct Vertex3DTP *)((int)buffer&~0x40000000);
	float minMax[6];
	md2GetFractionalFrame(mesh,frame,nextFrame,fraction,buffer,minMax);
	sceKernelDcacheWritebackAll();

	Image *source=actor->texture;
	//printf("Drawing actor %s with texture %s\n",actor->name,source->filename);
	if(source) {
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);
	}
	sceGumMatrixMode(GU_MODEL);
	sceGumPushMatrix();
	/* int i;
	for(i=0;i<6;i++) printf("minMax[%d]=%.2f; ",i,minMax[i]);
	printf("\n"); */
	ScePspFVector3 trans={actor->x,actor->y+20,actor->z};
	sceGumTranslate(&trans);
	sceGumRotateY(actor->angle);
	sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,mesh->polyCount*3,0,buffer);
	sceGumPopMatrix();
	//printf("acting\n");
}

MapState updateMap(struct Map *map,int elapsed)
{
	updateMapActor(map,&map->actor,elapsed);
	if(map->angleDir<0) map->cameraAngle-=elapsed*GU_PI/4/1000.0f*2;
	if(map->cameraAngle>GU_PI) map->cameraAngle-=2*GU_PI;
	if(map->angleDir>0) map->cameraAngle+=elapsed*GU_PI/4/1000.0f*2;
	if(map->cameraAngle<-GU_PI) map->cameraAngle+=2*GU_PI;
	
	int i;
	for(i=0;i<6;i++) {
		updateMapActor(map,map->guide+i,elapsed);
	}
	float ax=map->actor.x,ay=map->actor.y,az=map->actor.z;
	// places
	float nearestDist2=0;
	int oldActivePlace=map->activePlace;
	for(i=0;i<map->placeCount;i++) {
		struct Place *p=map->place+i;
		if(!(p->flags&PF_VISIBLE)) continue;
		if(p->type>=PT_GRASS) continue;
		float dx=ax-p->x,dy=ay-p->y,dz=az-p->z;
		float dist2=dx*dx+dy*dy+dz*dz;
		if((nearestDist2==0 || dist2<nearestDist2) && p->type<=PT_CRATE) { //PT_GREEN) {
			map->activePlace=i;
			nearestDist2=dist2;
		}
		if(dist2<512*512) {
			// nearby one
			if(!(p->flags&PF_VISITED)) {
				if(!map->particleEmitter[i].desc) newParticle(map->particleEmitter+i,&mapDesc,0,0,0);
				updateParticles(map->particleEmitter+i,elapsed);
			} else {
				if(map->particleEmitter[i].desc) {
					newParticle(map->particleEmitter+i,0,0,0,0);	// clear it out.
				}
			}
		}
	}
	if(map->place[map->activePlace].type!=map->place[oldActivePlace].type) {
		struct ParticleDescription *desc=&mapSelectedDesc;
		if(map->place[map->activePlace].type==PT_CHEST) desc=&mapChestDesc;
		if(map->place[map->activePlace].type==PT_CRATE) desc=&mapCrateDesc;
		newParticle(&map->particleSelectedEmitter,desc,0,0,0);
	}
	if(updateParticles(&map->particleSelectedEmitter,elapsed)) {
		struct ParticleDescription *desc=&mapSelectedDesc;
		if(map->place[map->activePlace].type==PT_CHEST) desc=&mapChestDesc;
		if(map->place[map->activePlace].type==PT_CRATE) desc=&mapCrateDesc;
		newParticle(&map->particleSelectedEmitter,desc,0,0,0);
	}
	/* if(map->coin) {
		if(updateAseSceneMax(map->coin,elapsed,map->coin->lastFrame))
			resetAseScene(map->coin,map->coin->firstFrame);
	} */
	if(map->explodingCrateTimer>0) {
		map->explodingCrateTimer-=elapsed;
		if(map->explodingCrateTimer<0) map->explodingCrateTimer=0;
		updateAseSceneMax(map->explodingCrateScene,elapsed,map->explodingCrateScene->lastFrame);
	}
	if(map->foundItemTimer>0) {
		map->foundItemTimer-=elapsed;
		if(map->foundItemTimer<0) map->foundItemTimer=0;
	}
	
	if(map->state==MS_ACTIVE) {
		struct MapActor *a=&map->actor;
		float x=a->x+sinf(map->cameraAngle)*200;
		float z=a->z+cosf(map->cameraAngle)*200;
		float y=getMapElev(map,x,z)+90;
		if(y<a->y+150) y=a->y+90;
		cameraSetFromTo(x,y,z,a->x,a->y,a->z);
		//printf("setting camera to go from %f,%f,%f to %f,%f,%f\n",x,y,z,a->x,a->y,a->z);
	}
	return map->state;
}

extern const char *tournamentName[6];

void drawMapOverlay(struct Map *map,int activePlaceClose)
{
	char buf[256];
	//sprintf(buf,"Centers: %d/%d",available,6);
	//intraFontPrint(iFont[FONT_BODY],5,205,buf);
	//sprintf(buf,"Crates and Chests: %d, %d/%d",crateTotal,chestOpen,chestTotal);
	//intraFontPrint(iFont[FONT_BODY],5,220,buf);
	sprintf(buf,"Tournaments: %d/%d",map->place[PT_BLACK].level/12+1,6);
	intraFontPrint(iFont[FONT_BODY],5,235,buf);
	// Draw stuff about the currently selected destination.
	if(map->activePlace>=0 && map->activePlace<6 && activePlaceClose) {
		struct Place *p=map->place+map->activePlace;
		sprintf(buf,"Training Center %s",placeTypeName[map->activePlace]);
		intraFontPrint(iFont[FONT_BODY],300,190,buf);
		sprintf(buf,"Mastered: %s",p->flags&PF_MASTERED?"yes":"no");
		intraFontPrint(iFont[FONT_BODY],300,205,buf);
		sprintf(buf,"%s",p->flags&PF_TOURNAMENT?tournamentName[(p->level/12)%6]:"");
		intraFontPrint(iFont[FONT_BODY],300,220,buf);
		sprintf(buf,"Type: %s (%d)",placeTypeName[p->type],p->type);
		intraFontPrint(iFont[FONT_BODY],300,235,buf);
		sprintf(buf,"Level: %d",p->level);
		intraFontPrint(iFont[FONT_BODY],300,250,buf);
	} else {
		/*
		struct Place *p=map->place+map->activePlace;
		sprintf(buf,"Type: %s (%d)",placeTypeName[p->type],p->type);
		intraFontPrint(iFont[FONT_BODY],300,235,buf);
		sprintf(buf,"Location: %d",map->activePlace);
		intraFontPrint(iFont[FONT_BODY],300,250,buf);
		*/
	}
	if(map->popupMenu) {
		sprintf(buf,"Use arrow keys to choose which training centre to visit.");
	} else {
		char *opt="";
		if(activePlaceClose && map->activePlace<6) opt="X to talk.";
		else if(activePlaceClose && map->activePlace>6) opt="X to open.";
		intraFontPrint(iFont[FONT_BODY],80,40,opt);
		sprintf(buf,"START to Save. [] to teleport");
	}
	intraFontPrint(iFont[FONT_SMALL],15,260,buf);
}

enum GuideStatus { GUIDE_TOOLOW, GUIDE_TOOHIGH, GUIDE_NOTMASTERED, GUIDE_MASTERED };
typedef enum GuideStatus GuideStatus;

GuideStatus getGuideStatus(struct Map *map)
{
	int who=map->activePlace;
	struct Place *p=map->place+who;
	int level=p->level;
	if(getMaxTeamLevel()<level-2) return GUIDE_TOOHIGH;
	else if(getMaxTeamLevel()<=level && p->flags&PF_MASTERED) return GUIDE_MASTERED;
	else if(getMaxTeamLevel()<=level) return GUIDE_NOTMASTERED;
	else return GUIDE_TOOLOW;
}

char *getGuideLine(struct Map *map)
{
	int who=map->activePlace;
	int level=map->place[who].level;
	int g=level/2;
	char *message="Good luck on your quest.";
	int max=getMaxTeamLevel();
	if(g<guideCount) {
		//printf("Max team level is %d, and guide is %d\n",max,g);
		if(max<level-2) {
			message=guide[g].toohigh;
			//printf("too high: '%s'\n",message);
		} else if(max<=level && map->place[who].flags&PF_MASTERED) {
			message=guide[g].mastered;
			//printf("mastered: '%s'\n",message);
		} else if(max<=level) {
			message=guide[g].notmastered;
			//printf("not mastered: '%s'\n",message);
		} else {
			message=guide[g].toolow;
			//printf("too low: '%s'\n",message);
		}
	} else {
		if(max<level-2) {
			message="The other combatants are out of your league here.";
		} else if(max<=level && map->place[who].flags&PF_MASTERED) {
			message="Good work.  Continue training?";
		} else if(max<=level) {
			message="We can help you train here.";
		} else {
			message="This training centre has no challenge left.";
		}
	}
	//printf("Message: %s\n",message);
	return message;
}

int getGuideLineCount(struct Map *map)
{
	int lines=1;
	char *message=getGuideLine(map);
	int i;
	for(i=0;i<strlen(message);i++) {
		if(message[i]=='|') lines++;
	}
	return lines;
}

char *getCurrentGuideLine(struct Map *map)
{
	static char message[1024];
	int line=map->guideMode-1;
	if(line<0) return "";
	char *s=getGuideLine(map);
	while( s[0] && line>0) {
		if(s[0]=='|') line--;
		s++;
	}
	if(line>0) map->guideMode=0;	// out of range
	strcpy(message,s);
	if(strchr(message,'|')) strchr(message,'|')[0]=0;
	return message;
}

void drawGuideOverlay(struct Map *map)
{
	// Now draw the current dialog.
	int who=map->activePlace;
	if(who>5) return;
	drawFilledRect(0,190,480,272-190,GU_RGBA(0,0,0,0x80));
	Image *icon=map->guide[who].icon;
	drawSprite(0,0,icon->imageWidth,icon->imageHeight,icon,2,190-icon->imageHeight/2);
	char *message=getCurrentGuideLine(map);
	intraFontPrintColumn(iFont[FONT_MESSAGE],68,190+18,480-68,message);
	
	switch(getGuideStatus(map)) {
	case GUIDE_TOOHIGH:
	case GUIDE_TOOLOW:
		message="X to continue";
		break;
	default:
		if(map->guideMode<getGuideLineCount(map)) {
			message="X to continue or O to exit";
		} else {
			message="X to battle or O to exit";
		}
	}
	intraFontPrint(iFont[FONT_BODY],50,25,message);
}

void drawMap(struct Map *map)
{
	sceGuEnable(GU_LIGHTING);

	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	sceGuFrontFace(GU_CCW);
	drawTerrain(map->terrain);

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();

	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	drawMapActor(&map->actor);	
	sceGuEnable(GU_LIGHTING);
	int i;
	int available=0;
	int tournamentCentre=0;
	int masteredTournament=0;
	int activePlaceClose=0;
	int crateTotal=0;
	int crateOpen=0;
	int chestTotal=0;
	int chestOpen=0;
	float ax=map->actor.x,ay=map->actor.y,az=map->actor.z;
	sceGuAlphaFunc(GU_GREATER,3,0xff);
	sceGuEnable(GU_ALPHA_TEST);
	for(i=0;i<map->placeCount;i++) {
		struct Place *p=map->place+i;
		if(!(p->flags&PF_VISIBLE)) continue;
		if(p->flags&PF_TOURNAMENT) tournamentCentre++;
		if(p->flags&PF_MASTERED && p->flags&PF_TOURNAMENT) masteredTournament++;
		if(p->type==PT_CHEST) {
			chestTotal++;
			if(p->flags&PF_MASTERED) chestOpen++;
		}
		if(p->type==PT_CRATE) {
			crateTotal++;
			if(p->flags&PF_MASTERED) crateOpen++;
		}
		available++;
		float dx=ax-p->x,dy=ay-p->y,dz=az-p->z;
		float dist2=dx*dx+dy*dy+dz*dz;
		if(dist2<512*512) {
			if(p->type<=PT_GREEN) {
				sceGuDisable(GU_LIGHTING);
				sceGuColor(0xffffffff);
				sceGuTexScale(1,1);
				drawMapActor(map->guide+(int)p->type);
			}
			// draw this one
			sceGumPushMatrix();
			ScePspFVector3 trans={p->x,p->y,p->z};
			sceGumTranslate(&trans);
			sceGumPushMatrix();
			if(p->avatar.rotateY!=0) sceGumRotateY(p->avatar.rotateY);
			/* if(map->coin) {
				Image *oldCoin=map->coin->material->image;
				if(map->coinTexture[p->type+(p->flags&PF_MASTERED?6:0)])
					map->coin->material->image=map->coinTexture[p->type+(p->flags&PF_MASTERED?6:0)];
				drawAseScene(map->coin);
				map->coin->material->image=oldCoin;
			} */
			int item=p->avatar.itemId;
			setItemPos(item,0,0,0);
			renderItem(item);
			sceGumPopMatrix();
			
			if(!(p->flags&PF_VISITED)) {
				if(map->particleEmitter) {
					drawParticles(map->particleEmitter+i);
				}
			}
			if(i==map->activePlace && dist2<96*96) {
				if(map->particleSelectedEmitter.desc) {
					drawParticles(&map->particleSelectedEmitter);
				} else {
					newParticle(&map->particleSelectedEmitter,&mapSelectedDesc,0,0,0);
				}
				activePlaceClose=1;
			}
			sceGumPopMatrix();
		}
	}
	if(!activePlaceClose) {
		map->guideMode=0;
	}
	if(map->explodingCrateTimer>0) {
		sceGumPushMatrix();
		ScePspFVector3 trans={map->explodingCrate[0],map->explodingCrate[1],map->explodingCrate[2]};
		sceGumTranslate(&trans);
		sceGumRotateX(-GU_PI/2);
		ScePspFVector3 scale={0.7,0.7,0.7};
		sceGumScale(&scale);
		drawAseScene(map->explodingCrateScene);
		sceGumPopMatrix();
	}
	if(map->foundItemTimer>0) {
		sceGumPushMatrix();
		ScePspFVector3 trans={map->foundItem[0],map->foundItem[1]+15,map->foundItem[2]};
		sceGumTranslate(&trans);

		//float *min=getItemMin(map->foundItemId), *max=getItemMax(map->foundItemId);
		//ScePspFVector3 trans2={-(min[0]+max[0])/2,-(min[1]+max[1])/2,-(min[2]-max[2])/2};
		//sceGumTranslate(&trans2);
		sceGumRotateY(map->foundItemTimer/1000.0f*2*GU_PI);
		setItemPos(map->foundItemId,0,0,0);
		//ScePspFVector3 scale={3,3,3};
		//sceGumScale(&scale);
		renderItem(map->foundItemId);
		sceGumPopMatrix();
	}
	
	sceGuDisable(GU_ALPHA_TEST);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDisable(GU_LIGHTING);


	//int w=intraFontMeasureText(iFont[FONT_BODY],"Welcome to Sushi Island");
	//intraFontPrint(iFont[FONT_BODY],240-w/2,25,"Welcome to Sushi Island");

	if(map->guideMode>0) {
		drawGuideOverlay(map);
	} else {
		char buf[256];
		//sprintf(buf,"Centers: %d/%d",available,6);
		//intraFontPrint(iFont[FONT_BODY],5,205,buf);
		sprintf(buf,"Crates and Chests: %d, %d/%d",crateTotal,chestOpen,chestTotal);
		intraFontPrint(iFont[FONT_BODY],5,220,buf);
		drawMapOverlay(map,activePlaceClose);
	}
	
#ifdef DEBUG_ANGLE
	sprintf(buf,"dx,dy: %.2f,%.2f TX,TY: %.2f,%.2f",map->actor.dx,map->actor.dy,debugTX,debugTY);
	intraFontPrint(iFont[FONT_SMALL],15,45,buf);
	sprintf(buf,"camera %.3f; actor %.3f; newAngle %.3f",map->cameraAngle,map->actor.angle,debugNewAngle);
	intraFontPrint(iFont[FONT_SMALL],15,60,buf);
	sprintf(buf,"DX,DY: %.2f,%.2f",debugDX,debugDY);
	intraFontPrint(iFont[FONT_SMALL],15,75,buf);
#endif
}

PlaceType getActivePlace(struct Map *map)
{
	return map->place[map->activePlace].type;
}

int getActiveLevel(struct Map *map)
{
	return map->place[map->activePlace].level;
}

int mapCompletePercent(struct Map *map)
{
	int per=0,cent=73;	// difficult of the last green level
	int i;
	for(i=0;i<6;i++) {
		if(per<map->place[i].level && map->place[i].flags&PF_MASTERED) per=map->place[i].level;
	}
	if(per>cent) per=cent;
	return 100*per/cent;
}

int applyDuelResult(struct Map *map,DuelState ds)
{
	int result=0;
	map->place[map->activePlace].flags|=PF_VISITED;
	struct Place *curr=map->place+map->activePlace;
	struct Place *old=0;
	if(map->activePlace>0)
		old=map->place+map->activePlace-1;
	else if(map->activePlace==0 && curr->level>1) 
		old=map->place+5;
	if(old && ds==DS_WIN) {
		old->level=curr->level+10;
		old->flags&=~PF_MASTERED;
	}

	if(map->activePlace+1<map->placeCount && map->place[map->activePlace].type<=PT_GREEN) {
		map->place[map->activePlace+1].flags|=PF_VISIBLE;
	}
	if(ds==DS_WIN) {
		if((map->place[map->activePlace].flags&PF_MASTERED)==0 && map->place[map->activePlace].type==PT_BLACK) {
			printf("*** Black tournament victory.  Time for spoils.\n");
			// we rebuild all crates!
			int i;
			for(i=0;i<map->placeCount;i++) {
				if(map->place[i].type==PT_CRATE) {
					map->place[i].flags|=PF_VISIBLE;	// more chocolately goodness!
					map->place[i].flags&=~PF_MASTERED;	// more chocolately goodness!
				}
			}
			result=1;
		}
		map->place[map->activePlace].flags|=PF_MASTERED;
		if(map->activePlace+1<map->placeCount && (map->place[map->activePlace].flags&PF_MASTERED)) {
			map->place[(map->activePlace+1)%6].flags&=~PF_VISITED;
		}
	}
	map->state=MS_ACTIVE;
	return result;
}

int mapSaveSize(struct Map *map)
{
	return sizeof(struct Place)*map->placeCount+sizeof(int)*3;
}

void saveMapBuffer(struct Map *map,char *buffer,int size)
{
	memcpy(buffer,&map->activePlace,sizeof(int));
	buffer+=4;
	memcpy(buffer,&map->scale,sizeof(int));
	buffer+=4;
	memcpy(buffer,&map->placeCount,sizeof(int));
	buffer+=4;
	memcpy(buffer,map->place,sizeof(struct Place)*map->placeCount);
}

void loadMapBuffer(struct Map *map,char *buffer,int size)
{
	if(((int *)buffer)[2]*sizeof(struct Place)+sizeof(int)*3!=size) {
		printf("Buffer is corrupt\n");
		return;
	}
	memcpy(&map->activePlace,buffer,sizeof(int));
	buffer+=4;
	// now, teleport the actor to the new active place.
	struct Place *p=map->place+map->activePlace;
	struct ParticleDescription *desc=&mapSelectedDesc;
	if(map->particleSelectedEmitter.desc!=desc) {
		newParticle(&map->particleSelectedEmitter,desc,0,0,0);
	}
	map->actor.x=p->x-32;
	map->actor.y=p->y;
	map->actor.z=p->z;
	float *min=getItemMin(p->avatar.itemId);
	float *max=getItemMax(p->avatar.itemId);
	map->guideMode=0;
	if(min && map->actor.z<p->z+max[2]) map->actor.x=p->x+min[0]-32;
	
	memcpy(&map->scale,buffer,sizeof(int));
	buffer+=4;
	memcpy(&map->placeCount,buffer,sizeof(int));
	buffer+=4;
	free(map->place);
	map->place=(struct Place *)malloc(sizeof(struct Place)*map->placeCount);
	memcpy(map->place,buffer,sizeof(struct Place)*map->placeCount);
}

int handleMapDown(struct Map *map,Buttons button)
{
	if(map->popupMenu) {
		map->actor.direction=0;
		return 0;
	}
	switch(button) {
	case BT_LTRIGGER:
		map->angleDir=1;
		break;
	case BT_RTRIGGER:
		map->angleDir=-1;
		break;
	case BT_UP:
		map->actor.direction|=DIR_UP;
		//printf("Going up.\n");
		break;
	case BT_DOWN:
		map->actor.direction|=DIR_DOWN;
		//printf("Going down.\n");
		break;
	case BT_LEFT:
		map->actor.direction|=DIR_LEFT;
		//printf("Going left.\n");
		break;
	case BT_RIGHT:
		map->actor.direction|=DIR_RIGHT;
		//printf("Going right.\n");
		break;
	case BT_SQUARE:
		map->popupMenu=1;
		break;
	default:
		break;
	}
	return 0;	
}

extern int chestId;

int handleMap(struct Map *map,enum Buttons button) 
{
	if(map->popupMenu) {
		map->actor.direction=0;
		switch(button) {
		case BT_SQUARE:
			map->popupMenu=0;
			return 0;
		case BT_LEFT:
		case BT_UP:
			map->activePlace--;
			if(map->activePlace>5) map->activePlace=0;		// we where near a chest or something.
			if(map->activePlace<0) map->activePlace=5;
			break;
		case BT_RIGHT:
		case BT_DOWN:
			map->activePlace++;
			if(map->activePlace>5) map->activePlace=0;
			break;
		default:
			return 0;
		}
		// now, teleport the actor to the new active place.
		struct Place *p=map->place+map->activePlace;
		struct ParticleDescription *desc=&mapSelectedDesc;
		if(map->particleSelectedEmitter.desc!=desc) {
			newParticle(&map->particleSelectedEmitter,desc,0,0,0);
		}
		map->actor.x=p->x-32;
		map->actor.y=p->y;
		map->actor.z=p->z;
		float *min=getItemMin(p->avatar.itemId);
		float *max=getItemMax(p->avatar.itemId);
		map->guideMode=0;
		if(min && map->actor.z<p->z+max[2]) map->actor.x=p->x+min[0]-32;
		return 0;
	}
	struct Place *p=map->place+map->activePlace;
	float x=map->actor.x-p->x;
	float y=map->actor.y-p->y;
	float z=map->actor.z-p->z;
	float dist2;
	switch(button) {
	case BT_LTRIGGER:
		if(map->angleDir>0) map->angleDir=0;
		break;
	case BT_RTRIGGER:
		if(map->angleDir<0) map->angleDir=0;
		break;
	case BT_CROSS:
		dist2=x*x+y*y+z*z;
		if(dist2<96*96) {
			playSfx(SFX_MENUSELECT);
			struct Place *p=map->place+map->activePlace;
			if(map->activePlace<6 && map->guideMode<getGuideLineCount(map)) {
				map->guideMode++;
			} else if(map->activePlace<6 && map->guideMode>0) {
				switch(getGuideStatus(map)) {
				case GUIDE_TOOLOW:
				case GUIDE_TOOHIGH:
					map->guideMode=0;
					break;
				default:
					map->guideMode=1;
					setTimeLeft(0);
					map->state=(map->place[map->activePlace].flags&PF_TOURNAMENT)==0 || (map->place[map->activePlace].flags&PF_MASTERED)?MS_DUEL:MS_TOURNAMENT;
					printf("map state now %d\n",map->state);
				}
				//map->activePlace=0;
			} else if(p->type==PT_CHEST) {
				if(!(p->flags&PF_MASTERED)) {
					p->flags|=PF_MASTERED;
					p->avatar.itemId=chestId+2;	// mark it as opened.

					float *max=getItemMax(chestId+2);
					float *min=getItemMin(chestId+2);
					float rect[4]={p->x+min[0],p->z+min[2],p->x+max[0],p->z+max[2]};
					while(collideSphereRect(12,map->actor.x,map->actor.z,rect)) {
						// hack to move the player out of the way of the chest.
						//printf("nudging player at %.2f,%.2f; chest rect %.2f,%.2f - %.2f,%.2f\n",map->actor.x,map->actor.z,rect[0],rect[1],rect[2],rect[3]);
						map->actor.x-=0.07;
						map->actor.z+=0.1;
					}

					map->foundItem[0]=p->x;
					map->foundItem[1]=p->y+40;
					map->foundItem[2]=p->z;
					map->foundItemTimer=2000;
					map->foundItemId=shardId;
					if(strstr(p->contains,"Shard")) {
						int result=(int)mapNameToItemType(p->contains);
						//printf("mapped %s to %d\n",p->contains,result);
						map->foundItemId+=result-(int)IT_WHITE;
					}
					if(strstr(p->contains,"Potion")) map->foundItemId=potionId;
					if(strstr(p->contains,"Pill")) map->foundItemId=pillId;
					if(strstr(p->contains,"Salve")) map->foundItemId=salveId;

					char buf[256];
					//printf("contains: %s\n",p->contains);
					sprintf(buf,"Found %s in chest!",p->contains);
					addMessage(buf,1000);
					addInventory(p->contains);
					playSfx(SFX_POWERUP);
				}
			} else if(p->type==PT_CRATE) {
				p->flags&=~PF_VISIBLE;
				if(map->explodingCrateScene) {
					resetAseScene(map->explodingCrateScene,0);
					map->explodingCrateTimer=2000;
				}
				map->explodingCrate[0]=p->x;
				map->explodingCrate[1]=p->y;
				map->explodingCrate[2]=p->z;

				map->foundItem[0]=p->x;
				map->foundItem[1]=p->y+40;
				map->foundItem[2]=p->z;
				map->foundItemTimer=2000;
				map->foundItemId=shardId;
					if(strstr(p->contains,"Shard")) {
						int result=(int)mapNameToItemType(p->contains);
						//printf("mapped %s to %d\n",p->contains,result);
						map->foundItemId+=result-(int)IT_WHITE;
					}
				if(strstr(p->contains,"Potion")) map->foundItemId=potionId;
				if(strstr(p->contains,"Pill")) map->foundItemId=pillId;
				if(strstr(p->contains,"Salve")) map->foundItemId=salveId;

				char buf[256];
				//printf("contains: %s\n",p->contains);
				sprintf(buf,"Found %s in crate!",p->contains);
				addMessage(buf,1000);
				addInventory(p->contains);
				playSfx(SFX_POWERUP);
			}
		}
		break;
	case BT_CIRCLE:
		playSfx(SFX_MENUSELECT);
		if(map->guideMode>0) map->guideMode=0;
		break;
	case BT_UP:
		map->actor.direction&=~DIR_UP;
		break;
	case BT_DOWN:
		map->actor.direction&=~DIR_DOWN;
		break;
	case BT_LEFT:
		map->actor.direction&=~DIR_LEFT;
		break;
	case BT_RIGHT:
		map->actor.direction&=~DIR_RIGHT;
		break;
	default:
		break;
	}
	return 0;
}

int analogMap(struct Map *map,int lx,int ly)
{
	//printf("Analog %d,%d\n",lx,ly);
	static int oldx,oldy;
	// make analog stick less noisy.
	if(oldx>=80 && oldx<=175 && lx>=80 && lx<=175 &&
	oldy>=80 && oldy<=175 && ly>=80 && ly<=175) 
		return 0;
	oldx=lx;
	oldy=ly;
	// put in range from -100 to +100.
	map->actor.ax=lx-100;
	if(map->actor.ax>=0 && map->actor.ax<=56) map->actor.ax=0;
	else if(map->actor.ax>56) map->actor.ax-=56;
	map->actor.ay=ly-100;
	if(map->actor.ay>=0 && map->actor.ay<=56) map->actor.ay=0;
	else if(map->actor.ay>56) map->actor.ay-=56;
	// make the deadzone bigger
	if(map->actor.ax>-20 && map->actor.ax<20) map->actor.ax=0;
	if(map->actor.ay>-20 && map->actor.ay<20) map->actor.ay=0;
	return 0;
}
