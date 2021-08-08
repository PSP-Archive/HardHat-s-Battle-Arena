#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspgum.h>
#include <pspgu.h>
#include "main.h"
#include "map.h"
#include "duel.h"

struct ItemList {
	char *name;
	ItemClass itemClass;
	ItemType itemType;
} itemList[]={
	{"Diamond Shard",IC_SHARD,IT_WHITE},
	{"Yellow Onyx Shard",IC_SHARD,IT_YELLOW},
	{"Ruby Shard",IC_SHARD,IT_RED},
	{"Blue Garnet Shard",IC_SHARD,IT_BLUE},
	{"Black Obsidian Shard",IC_SHARD,IT_BLACK},
	{"Green Emerald Shard",IC_SHARD,IT_GREEN},
	{"Mini Healing Potion",IC_ITEM,IT_HP1},
	{"Healing Potion",IC_ITEM,IT_HP2},
	{"Mega Healing Potion",IC_ITEM,IT_HP3},
	{"Strengthing Potion",IC_ITEM,IT_ATK},
	{"Defensive Potion",IC_ITEM,IT_DEF},
	{"Mini Action Point Salve",IC_ITEM,IT_AP1},
	{"Action Point Salve",IC_ITEM,IT_AP2},
	{"Mega Action Point Salve",IC_ITEM,IT_AP3},
	{"Mini Pill of Experience",IC_ITEM,IT_XP1},
	{"Pill of Experience",IC_ITEM,IT_XP2},
	{"Mega Pill of Experience",IC_ITEM,IT_XP3}
};
int itemListCount=6+11;

ItemType mapNameToItemType(const char *name)
{
	int i;
	for(i=0;i<itemListCount;i++) {
		if(strcmp(itemList[i].name,name)==0) {
			printf("Mapped %s to type %d\n",name,itemList[i].itemType);
			return itemList[i].itemType;
		}
	}
	return IT_NONE;	// who knows
}
ItemClass mapNameToItemClass(const char *name)
{
	int i;
	for(i=0;i<itemListCount;i++) {
		if(strcmp(itemList[i].name,name)==0) {
			printf("Mapped %s to class %d\n",name,itemList[i].itemClass);
			return itemList[i].itemClass;
		}
	}
	return IC_SPELL;	// who knows
}


struct Team *newTeam()
{
	struct Team *team=(struct Team *)calloc(sizeof(struct Team),1);
	team->targetCount=1;
	return team;
}


void teamAddToInventory(struct Inventory *inv,int invCount, const char *name)
{
	int empty=0;
	int i;
	for(i=0;i<invCount;i++) {
		if(inv[i].name[0]==0 && empty==0) {
			empty=i;
			printf("add: Empty slot: %d\n",empty);
		}
		if(strcmp(name,inv[i].name)==0) {
			inv[i].count++;
			printf("add: Matched %s. count now %d\n",name,inv[i].count);
			return;
		}
	}
	// add to first empty slot.
	strcpy(inv[empty].name,name);
	inv[empty].count=1;
	printf("add: Adding %s. to spot %d\n",name,empty);
}

int teamUseInventory(struct Inventory *inv,int invCount, const char *name)
{
	int i;
	for(i=0;i<invCount;i++) {
		if(strcmp(name,inv[i].name)==0 && inv[i].count>0) {
			inv[i].count--;
			printf("use: Matched %s. count now %d\n",name,inv[i].count);
			return 1;
		}
	}
	printf("use: Couldn't find %s.\n",name);
	return 0;
}

const char *tabName[]={"Spells","Items","Shards","Mages"};
void drawTeamInventory(struct Team *team)
{
	struct Inventory *inv;
	int invCount;
	int i;
	int j;
	
	drawFilledRect(0,0,480,272,GU_RGBA(128,96,0,255));
	// Now draw the tabs.
	int x=0;
	int y=10;
	for(i=0;i<4;i++) {
		intraFont *font=iFont[FONT_SMALL];
		unsigned long color=GU_RGBA(140,112,0,255);
		if(i==team->mode) {
			font=iFont[FONT_SMALLHIGHLIGHT];
			color=GU_RGBA(112,140,0,255);
		}
		float w=intraFontMeasureText(font,tabName[i]);
		drawFilledRect(x,y-8,w+4,12,color);
		x+=2;
		intraFontPrint(font,x,y,tabName[i]);
		x+=w+4;
	}
	x=0;
	y+=15;
	
	switch(team->mode) {
	case TEM_OVERALLSPELL:
	case TEM_OVERALLITEM:
	case TEM_OVERALLSHARD:
		if(team->mode==TEM_OVERALLSPELL) {
			inv=team->spell;
			invCount=team->spellCount;
		} else if(team->mode==TEM_OVERALLITEM) {
			inv=team->item;
			invCount=team->itemCount;
		} else if(team->mode==TEM_OVERALLSHARD) {
			inv=team->shard;
			invCount=team->shardCount;
		}
		int top=team->selectedItem-6;
		if(top+20>invCount) top=invCount-12;
		if(top<0) top=0;
		for(j=0;j<12;j++) {
			i=top+j;
			intraFont *font=iFont[FONT_BODY];
			unsigned long color=GU_RGBA(140,112,0,255);
			if(i==team->selectedItem) {
				font=iFont[FONT_BODYHIGHLIGHT];
				color=GU_RGBA(112,140,0,255);
			}
			float w=240;
			drawFilledRect(x,y-8,w+4,12,color);
			char buf[64];
			sprintf(buf,"%4d",inv[i].count);
			intraFontPrint(font,x+2,y,buf);
			intraFontPrint(font,x+35,y,inv[i].name);
			y+=12;
		}
		break;
	case TEM_ACTORSPELL:
	case TEM_REPLACESPELL:
	case TEM_SELECTACTOR:
		break;
	}
}

void updateTeamInventory(struct Team *team,int elapsed)
{
	
}

void handleTeamInventory(struct Team *team,Buttons button)
{
	switch(button) {
	case BT_UP:		// previous in list
		team->selectedItem--;
		if(team->selectedItem<0) {
			if(team->mode==TEM_OVERALLSPELL) {
				team->selectedItem=team->spellCount-1;
			} else if(team->mode==TEM_OVERALLITEM) {
				team->selectedItem=team->itemCount-1;
			} else if(team->mode==TEM_OVERALLSHARD) {
				team->selectedItem=team->shardCount-1;
			} else if(team->mode==TEM_ACTORSPELL) {
				team->selectedItem=team->actor[team->selectedActor].actionCount-1;
			} else if(team->mode==TEM_REPLACESPELL) {
				team->selectedItem=team->spellCount-1;
			} else if(team->mode==TEM_SELECTACTOR) {
				team->selectedItem=team->actorCount-1;
			} else {
				team->selectedItem=0;
			}
		}
		break;
	case BT_DOWN:	// next in list
		team->selectedItem++;
		if(team->mode==TEM_OVERALLSPELL && team->selectedItem>=team->spellCount) {
			team->selectedItem=0;
		} else if(team->mode==TEM_OVERALLITEM && team->selectedItem>=team->itemCount) {
			team->selectedItem=0;
		} else if(team->mode==TEM_OVERALLSHARD && team->selectedItem>=team->shardCount) {
			team->selectedItem=0;
		} else if(team->mode==TEM_ACTORSPELL && team->selectedItem>=team->actor[team->selectedActor].actionCount) {
			team->selectedItem=0;
		} else if(team->mode==TEM_REPLACESPELL && team->selectedItem>=team->spellCount) {
			team->selectedItem=0;
		} else if(team->mode==TEM_SELECTACTOR && team->selectedItem>=team->actorCount) {
			team->selectedItem=0;
		} else {
			team->selectedItem=0;
		}
		break;
	case BT_CROSS:	// edit
		break;
	case BT_SQUARE:	// use
		break;
	case BT_CIRCLE:	// exit
		break;
	case BT_LTRIGGER:	// switch teammate
		if(team->mode==TEM_ACTORSPELL) {
			team->selectedActor--;
			if(team->selectedActor<0) {
				team->selectedActor=team->actorCount-1;
				team->selectedItem=0;
				team->mode=TEM_OVERALLSHARD;
			}
		} else if(team->mode==TEM_OVERALLSPELL) {
			team->mode=TEM_ACTORSPELL;
			team->selectedActor=team->actorCount-1;
			team->selectedItem=0;
		} else if(team->mode<=TEM_OVERALLSHARD) {
			team->selectedItem=0;
			team->mode--;
		}
		break;
	case BT_RTRIGGER:	// switch teammate
		if(team->mode==TEM_ACTORSPELL) {
			team->selectedActor++;
			if(team->selectedActor>=team->actorCount) {
				team->selectedActor=0;
				team->selectedItem=0;
				team->mode=TEM_OVERALLSPELL;
			}
		} else if(team->mode==TEM_OVERALLSHARD) {
			team->mode=TEM_ACTORSPELL;
			team->selectedActor=0;
			team->selectedItem=0;
		} else if(team->mode<TEM_OVERALLSHARD) {
			team->mode++;
			team->selectedActor=0;
			team->selectedItem=0;
		}
		break;
	default:
		break;
	}	
}

int teamSaveSize(struct Team *team)
{
	return (32+sizeof(int)*12)*MAXTEAM+sizeof(int)*4+sizeof(struct Inventory)*128;
}

void saveTeam(struct Team *team,char *buffer,int length)
{
	((int *)buffer)[0]=team->actorCount<MAXTEAM?team->actorCount:MAXTEAM;
	((int *)buffer)[1]=32+sizeof(int)*12;	// actor record size.
	((int *)buffer)[2]=team->targetCount<MAXTEAM?team->targetCount:MAXTEAM;
	buffer+=32;
	int i;
	for(i=0;i<team->actorCount && i<MAXTEAM;i++) {
		struct Actor *a=team->actor+i;
		memset(buffer,0,32);
		strcpy(buffer,a->name);
		buffer+=32;
		int *v=(int *)buffer;
		v[0]=a->level;
		v[1]=a->xp;
		v[2]=a->hpMax;
		v[3]=a->apMax;
		v[4]=a->attack;
		v[5]=a->defense;
		v[6]=getActionId(a->action[0]);
		v[7]=getActionId(a->action[1]);
		v[8]=getActionId(a->action[2]);
		v[9]=getActionId(a->action[3]);
		v[10]=getActionId(a->action[4]);
		v[11]=getActionId(a->action[5]);
		buffer+=12*sizeof(int);
	}
	memcpy(buffer,team->spell,sizeof(struct Inventory)*32);
	buffer+=sizeof(struct Inventory)*32;
	memcpy(buffer,team->item,sizeof(struct Inventory)*32);
	buffer+=sizeof(struct Inventory)*32;
	memcpy(buffer,team->shard,sizeof(struct Inventory)*8);
	buffer+=sizeof(struct Inventory)*8;
}

void loadTeam(struct Team *team,char *buffer,int length)
{
	memset(team,sizeof(struct Team),0);
	team->actorCount=((int *)buffer)[0];
	int recordLen=((int *)buffer)[1];
	team->targetCount=((int *)buffer)[2];
	buffer+=32;
	if(team->actorCount<=MAXTEAM && recordLen>=32+sizeof(int)*6) {
		int i;
		for(i=0;i<team->actorCount;i++) {
			struct Actor *a=team->actor+i;
			memset(a,0,sizeof(struct Actor));
			memcpy(a->name,buffer,32);
			a->name[31]=0;
			int j;
			for(j=0;j<characterCount;j++) {
				if(strcmp(a->name,character[j].name)==0) {
					a->character=character+j;
					printf("Matched teammate %d ('%s') with character %d\n",i,a->name,j);
					break;
				}
			}
			if(!a->character) {
				// that character isn't available.
				a->character=character+((rand()>>16)%characterCount);
				printf("Choosing random replacement character for teammate %d\n",i);
				if(strlen(a->name)==0) {
					strcpy(a->name,a->character->name);
					printf("Overiding name of teammate %d to %s\n",i,a->name);
				}
			}
			buffer+=32;
			int *v=(int *)buffer;
			a->level=v[0];
			a->xp=v[1];
			a->hpMax=v[2];
			a->apMax=v[3];
			a->attack=v[4];
			a->defense=v[5];
			a->action[0]=lookUpActionId(v[6]);
			a->action[1]=lookUpActionId(v[7]);
			a->action[2]=lookUpActionId(v[8]);
			a->action[3]=lookUpActionId(v[9]);
			a->action[4]=lookUpActionId(v[10]);
			a->action[5]=lookUpActionId(v[11]);
			printf("Stats: level %d XP=%d HP=%d AP=%d atk=%d def=%d\n",a->level,a->xp,a->hpMax,a->apMax,a->attack,a->defense);

			a->frame=0;
			a->lastFrame=0;
			resetActor(a);
			buffer+=12*sizeof(int);
			buffer+=recordLen-(32+sizeof(int)*12);
			printf("adjusting loader for %d bytes.\n",recordLen-(32+sizeof(int)*6));
		}
	}
	memcpy(team->spell,buffer,sizeof(struct Inventory)*32);
	buffer+=sizeof(struct Inventory)*32;
	int i;
	for(i=0;i<32;i++) {
		if(team->spell[i].name[0]!=0 || team->spell[i].count!=0) {
			team->spellCount=i+1;
			printf("Spell %d: %d of %s\n",i,team->spell[i].count,team->spell[i].name);
		}
	}
	memcpy(team->item,buffer,sizeof(struct Inventory)*32);
	buffer+=sizeof(struct Inventory)*32;
	for(i=0;i<32;i++) {
		if(team->item[i].name[0]!=0 || team->item[i].count!=0) {
			team->itemCount=i+1;
			printf("Item %d: %d of %s\n",i,team->item[i].count,team->item[i].name);
		}
	}
	memcpy(team->shard,buffer,sizeof(struct Inventory)*8);
	buffer+=sizeof(struct Inventory)*8;
	for(i=0;i<8;i++) {
		if(team->shard[i].name[0]!=0 || team->shard[i].count!=0) {
			team->shardCount=i+1;
			printf("Shard %d: %d of %s\n",i,team->shard[i].count,team->shard[i].name);
		}
	}
}

void freeTeam(struct Team *team)
{
	free(team);
}

