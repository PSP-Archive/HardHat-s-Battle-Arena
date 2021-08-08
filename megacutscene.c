#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pspgu.h>
#include<pspgum.h>
#include "main.h"
#include "ase.h"
#include "particle.h"

/* Language for describing a cut scene:
- explicit timing
for milliseconds <action>
- declaration
actor name md2file skin
set objfile
prop name asefile x y z roty
particle name duration spawnrate spawnduration radius scaleamount (directionx,y,z) shape (windx,y,z) directionvariance startcolor endcolor
terrain name elevimage colorimage
- camera action
camera look-at from=x,y,z to=x,y,z
camera pan-to from=x,y,z to=x,y,z
- actor action
appearat name x,y,z
walkto name x,y,z
doaction name action
- misc action
show name
hide name
delete name
dialog iconfile "text"
particle-effect name particlename x,y,z
 */

struct CSActor {
	struct AnimMesh *animMesh;
	// for MD2
	int frameElapsed;
	int lastFrame;
	int frame;
	enum MotionType motion;

	int motionLooped;
	float tx,ty,tz,trotx,troty,trotz;	// target position
};

struct CSDialog {
	char *text;
	int elapsed;
	Image *icon;
};

typedef enum CSNodeType { CNT_MD2, CNT_OBJ, CNT_ASE, CNT_TERRAIN, CNT_PARTICLE, CNT_DIALOG } CSNodeType;
struct CutSceneNode {
	char *name;
	float x,y,z;
	float rotx,roty,rotz;
	int visible;	// can we be seen
	CSNodeType type;
	//union CSNodeContents {
	struct CSActor *actor;
	int item;	// the loaded item id.
	struct AseScene *aseScene;
	struct ParticleEmitter *emitter;
	struct Terrain *terrain;
	struct CSDialog *dialog;
	//} contents;
	//int renderFlags;
};

typedef enum SequenceType { ST_DELAY, ST_JUMPTO, ST_MOVETO, ST_CAMERA, ST_DIALOG, ST_SHOW, ST_HIDE } SequenceType;
struct CutSceneSequence {
	struct CutSceneSequence *simultaneous;
	int simultanousCount;
	
	SequenceType type;
	char *name;	// name of the node to create, hide, etc.
	char *text;
	char *path1;
	char *path2;
	float x,y,z,rx,ry,rz,tz,ty,tz;
	int delay;
};

struct CutScene {
	struct CutSceneNode *node;
	int nodeCount;
	struct CSSequence *sequence;
	int sequenceCount;
	int elapsed;
	int duration;	// for bookkeeping purposes mainly.
};

struct CutSceneNode *newCutSceneNode(struct CutScene *cs,CSNodeType type)
{
	if(cs->nodeCount==0) {
		cs->node=malloc(sizeof(struct CutSceneNode));
	} else {
		cs->node=realloc(cs->node,sizeof(struct CutSceneNode)*cs->nodeCount+1);
	}
	struct CutSceneNode *result=cs->node+cs->nodeCount;
	cs->nodeCount++;
	memset(result,0,sizeof(struct CutSceneNode));
	result->type=type;
	return result;
}

struct CutSceneNode *newCSSequence(struct CutScene *cs,SequenceType type)
{
	if(cs->sequenceCount==0) {
		cs->sequence=malloc(sizeof(struct CutSceneSequence));
	} else {
		cs->sequence=realloc(cs->sequence,sizeof(struct CutSceneSequence)*cs->sequenceCount+1);
	}
	struct CutSceneSequence *result=cs->sequence+cs->sequenceCount;
	cs->sequenceCount++;
	memset(result,0,sizeof(struct CutSceneSequence));
	result->type=type;
	return result;
}

struct CutScene *initCutScene(const char *fname)
{
	struct CutScene *cs=(struct CutScene *)calloc(sizeof(struct CutScene),1);
	FILE *file=fopen(fname,"r");
	char line[256];
	freeItems();
	while( fgets(line,255,file)) {
		line[255]=0;
		char command[256];

		if(strchr(line,'\n')) strchr(line,'\n')[0]=0;
		strcpy(command,line);
		if(strchr(command,' ')) strchr(command,' ')[0]=0;
		//for milliseconds <action>
		if(strcmp(command,"for")==0) {
			
		} else
		// - declaration
		// actor name md2file skin
		if(strcmp(command,"actor")==0) {
			struct CutSceneNode *node=newCutSceneNode(cs,CNT_OBJ);
			char *name=strchr(line,' ');
			if(name) name++;
			if(name) {
				char *filename=strchr(name,' ');
				if(filename) {
					filename[0]=0;
					node->name=strdup(name);
					filename++;
					char *skinname=strchr(filename,' ');
					if(skinname) {
						filename[0]=0;
						node->name=strdup(name);
						filename++;
						node->actor=loadCSActor(name,filename,skinname);
					} else {
						printf("actor: No skinname found\n");
					}
				} else {
					printf("actor: No filename found.\n");
				}
			} else {
				printf("actor: No name found.\n");
			}
		} else
		// set name objfile
		if(strcmp(command,"set")==0) {
			struct CutSceneNode *node=newCutSceneNode(cs,CNT_OBJ);
			char *name=strchr(line,' ');
			if(name) name++;
			if(name) {
				char *filename=strchr(name,' ');
				if(filename) {
					filename[0]=0;
					node->name=strdup(name);
					filename++;
					node->item=loadItemImage(filename);
				} else {
					printf("set: No filename found.\n");
				}
			} else {
				printf("set: No name found\n");
			}
		} else
		// prop name asefile x y z roty
		if(strcmp(command,"prop")==0) {
			char name[256]="",filename[256]="";
			float x=0,y=0,z=0,roty=0;
			int count=sscanf(line+strlen(command),"%s%s%f%f%f%f",name,filename,&x,&y,&z,&roty);
			if(count<6) {
				printf("prop: Missing parameters from '%s' -- got %d instead of 6 parameters.\n",line,count);
			}
			struct CutSceneNode *node=newCutSceneNode(CNT_ASE);
			node->name=strdup(name);
			node->aseScene=aseSceneLoad(filename);
			node->x=x;
			node->y=y;
			node->z=z;
			node->roty=roty;
		} else
		// particle name duration spawnrate spawnduration radius scaleamount (directionx,y,z) shape (windx,y,z) directionvariance startcolor endcolor
		if(strcmp(command,"particle")==0) {
			char name[256]="",filename[256]="";
			int duration,spawnrate,spawnduration;
			float radius,scaleamount;
			float dirx,diry,dirz;
			int shape;	// from 0 to 15
			float windx,windy,windz;
			float directionVariance;
			unsigned int startcolor,endcolor;
			int count=sscanf(line+strlen(command),"%s%d%d%d%f%f(%f%f%f)%d(%f%f%f)%f#%x#%x",name,&duration,&spawnrate,&radius,
			&scaleamount,&dirx,&diry,&dirz,&shape,&windx,&windy,&windz,&directionVariance,&startcolor,&endcolor);
			if(count<16) {
				printf("particle: Missing parameters from '%s' -- got %d instead of 16 parameters.\n",line,count);
				continue;
			}
			struct CutSceneNode *node=newCutSceneNode(CNT_ASE);
			node->name=strdup(name);
			node->emitter=calloc(sizeof(struct ParticleEmitter)+sizeof(struct ParticleDesc),1);
			node->x=x;
			node->y=y;
			node->z=z;
			node->roty=roty;
		} else
		// terrain name elevimage colorimage
		if(strcmp(command,"terrain")==0) {
			struct CutSceneNode *node=newCutSceneNode(cs,CNT_OBJ);
			char *name=strchr(line,' ');
			if(name) name++;
			if(name) {
				char *filename=strchr(name,' ');
				if(filename) {
					filename[0]=0;
					node->name=strdup(name);
					filename++;
					char *skinname=strchr(filename,' ');
					if(skinname) {
						skinname[0]=0;
						skinname++;
						node->=newTerrainDetail(filename,skinname);
					} else {
						printf("actor: No skinname found\n");
					}
				} else {
					printf("actor: No filename found.\n");
				}
			} else {
				printf("actor: No name found.\n");
			}
		} else
		// - camera action
		//camera close-up-on|medium-shot-on|long-shot-on name
		//camera look-at fromx,y,z tox,y,z
		//camera pan-to fromx,y,z tox,y,z
		if(strcmp(command,"camera")==0) {
			char subcommand[256]="";
			sscanf(line+strlen(command),"%s",subcommand);
			if(strcmp(subcommand,"look-at")==0) {
				float from[3],to[3];
				int count=sscanf(line+strlen(command),"%s%f,%f,%f %f,%f,%f",subcommand,from,from+1,from+2,to,to+1,to+2);
				if(count<7) {
					printf("Incorrect number of parameters to camera look-at\n");
					continue;
				}
				struct CutSceneSequence *seq=newCutSceneSequence(cs,ST_CAMERA);
				//cameraSetFromTo(from[0],from[1],from[2],to[0],to[1],to[2]);
				seq->name="camera";
				seq->delay=0;
				seq->x=from[0];
				seq->y=from[1];
				seq->z=from[2];
				seq->tx=to[0];
				seq->ty=to[1];
				seq->tz=to[2];
			} else if(strcmp(subcommand,"pan-to")==0) {
				float from[3],to[3];
				int count=sscanf(line+strlen(command),"%s%f,%f,%f %f,%f,%f",subcommand,from,from+1,from+2,to,to+1,to+2);
				if(count<7) {
					printf("Incorrect number of parameters to camera look-at\n");
					continue;
				}
				struct CutSceneSequence *seq=newCutSceneSequence(cs,ST_CAMERA);
				//cameraSetFromTo(from[0],from[1],from[2],to[0],to[1],to[2]);
				seq->name="camera";
				seq->delay=4000;
				seq->x=from[0];
				seq->y=from[1];
				seq->z=from[2];
				seq->tx=to[0];
				seq->ty=to[1];
				seq->tz=to[2];
			}
		} else
		//- actor action
		//appearat name x,y,z
		if(strcmp(command,"appearat")==0) {
			
		} else
		//walkto name x,y,z
		if(strcmp(command,"walkto")==0) {
			
		} else
		//doaction name action
		if(strcmp(command,"doaction")==0) {
			
		} else
		//- misc action
		//show name
		if(strcmp(command,"show")==0) {
			
		} else
		//hide name
		if(strcmp(command,"hide")==0) {
			
		} else
		//delete name
		if(strcmp(command,"delete")==0) {
			
		} else
		//dialog iconfile "text"
		if(strcmp(command,"dialog")==0) {
			
		} else
		//particle-effect name x,y,z
		if(strcmp(command,"particle-effect")==0) {
			
		} else {
			printf("Command not recognized: '%s'\n",command);
		}
	}
	fclose(file);
	return cs;
}

int updateCutScene(struct CutScene *cs,int elapsed)
{
	cs->elapsed+=elapsed;
	return cs->elapsed>=cs->duration;	// scene done.	
}

void drawCutScene(struct CutScene *cs)
{
	int i;
	for(i=0;i<cs->nodeCount;i++) {
		struct CutSceneNode *node=cs->node+i;
		sceGumPushMatrix();
		ScePspFVector3 trans={node->x,node->y,node->z};
		ScePspFVector3 rot={node->rotx,node->roty,node->rotz};
		sceGumTranslate(&trans);
		sceGumRotate(&rot);
		//typedef enum CSNodeType { CNT_MD2, CNT_OBJ, CNT_ASE, CNT_TERRAIN, CNT_PARTICLE, CNT_DIALOG } CSNodeType;
		switch(node->type) {
		case CNT_MD2:
			drawCSActor(node->actor);
			break;
		case CNT_OBJ:
			renderItem(node->item);
			break;
		case CNT_ASE:
			drawAseScene(node->aseScene);
			break;
		case CNT_TERRAIN:
			drawTerrain(node->terrain);
			break;
		case CNT_PARTICLE:
			drawParticles(node->emitter);
			break;
		case CNT_DIALOG:
			drawCSDialog(node->dialog);
			break;
		}
		sceGumPopMatrix();
	}
}

int handleCutScene(struct CutScene *cs,Buttons button)
{
	if(button==BT_CROSS || button==BT_START) {
		cs->elapsed=cs->duration;
		return 1;
	}
	return 0;
}

static void freeSequence(struct CutSceneSequence *sequence,int sequenceCount)
{
	int i;
	for(i=0;i<sequenceCount;i++) {
		if(sequence[i].simultaneous) {
			freeSequence(sequence[i].simultaneous,sequence[i].simultaniousCount);
			sequence[i].simultanious=0;
		}
	}
	free(sequence);
}

void freeCutScene(struct CutScene *cs)
{
	// First all of the nodes.
	int i;
	for(i=0;i<cs->nodeCount;i++) {
		struct CutSceneNode *node=cs->node+i;
		if(node->aseScene) freeAseScene(node->aseScene); node->aseScene=0;
		if(node->actor) freeCSActor(node->actor); node->actor=0;
		if(node->dialog) {
			if(node->dialog->text) free(node->dialog->text); node->dialog->text=0;
			free(node->dialog); node->dialog=0;
		}
		if(node->emitter) {
			newParticle(node->emitter,0,0,0,0);
			free(node->emitter);
			node->emitter=0;
		}
		if(node->name) free(node->name); node->name=0;
		if(node->terrain) freeTerrain(node->terrain); node->terrain=0;
	}
	freeItems();	// free any OBJ files and their textures
	if(cs->node) free(cs->node);
	if(cs->sequence) freeSequence(cs->sequence,cs->sequenceCount);
	cs->sequence=0;
	free(cs);
}
