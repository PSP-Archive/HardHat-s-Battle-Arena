#ifndef DUEL_H
#define DUEL_H
#include "main.h"
//#include "map.h"

// duel.c
enum PlaceType { PT_WHITE,PT_YELLOW,PT_RED,PT_BLUE,PT_BLACK,PT_GREEN, PT_CHEST,PT_CRATE,PT_GRASS,PT_TREE,PT_ARIDTREE, PT_BATTLE,PT_TOURNAMENT } type;
typedef enum PlaceType PlaceType;
struct Duel;
enum DuelType { DT_ONEONONE, DT_HOTSEAT, DT_ADHOC, DT_ROYAL, DT_TOURNAMENT };
typedef enum DuelType DuelType;
enum DuelState { DS_ACTIVE, DS_WIN, DS_LOST, DS_ABORT };
typedef enum DuelState DuelState;
typedef enum ItemClass { IC_SHARD,IC_ITEM,IC_SPELL } ItemClass;
typedef enum ItemType { 
	IT_WHITE,IT_YELLOW,IT_RED,IT_BLUE,IT_BLACK,IT_GREEN,
	IT_HP1,IT_HP2,IT_HP3,
	IT_ATK,IT_DEF,
	IT_AP1,IT_AP2,IT_AP3,
	IT_XP1,IT_XP2,IT_XP3,

	IT_NONE,
} ItemType;
ItemType mapNameToItemType(const char *name);
ItemClass mapNameToItemClass(const char *name);

struct Character {
	char name[48];
	Image *icon;
	struct AnimMesh *mesh;
	//struct Action *action[8];
};
extern struct Character *character;
extern int characterCount;

struct Inventory {
	char name[48];
	int count;
};
void teamAddToInventory(struct Inventory *inv,int invCount, const char *name);

struct Actor {
	// this is the part that is saved in a game save
	char name[48];	// name of the creature
	int level;	// level 1
	int xp;		// level up every 8000 points?
	int hpMax;	// hpMax is a dice roll for each level
	int apMax;	// apMax is a dice roll for each 5 levels
	int attack;	// attack is a dice roll for each level
	int defense;	// defense is a dice roll for each level

	struct Action *action[6];
	int actionCount;

	// this is the part that is battle specific
	struct Character *character;
	int boost;	// used up when you attack, heal, or jump/crouch
	int hp;
	int ap;
	
	// animation part
	enum MotionType motion;	// automatically goes back to STANDING when done an animation
	enum MotionType oldMotion;	// just for debugging output.
	int lastFrame;
	int frame;
	int frameElapsed;
	float minMax[6];

	// hud animation
	int hpOld,apOld;
	int oldTimer;
};

#define MAXTEAM 6
enum TeamEditMode { TEM_OVERALLSPELL, TEM_OVERALLITEM, TEM_OVERALLSHARD, TEM_ACTORSPELL, TEM_REPLACESPELL, TEM_SELECTACTOR };

struct Team {
	struct Actor actor[MAXTEAM];
	int actorCount;
	int targetCount;
	struct Inventory shard[8];
	int shardCount;
	struct Inventory item[32];
	int itemCount;
	struct Inventory spell[32];
	int spellCount;

	// transient
	enum TeamEditMode mode;
	int selectedActor;
	int selectedItem;
};

struct Duel *newDuel();
void initDuel(struct Duel *duel,PlaceType place,int enemyLevel,DuelType type,struct Team *team);
DuelState updateDuel(struct Duel *duel,int elapsed);
void updateDuelSuspended(struct Duel *duel,int elapsed);
void drawHud(struct Duel *duel);
void drawDuel(struct Duel *duel);
void drawDuelSuspended(struct Duel *duel);
int handleDuel(struct Duel *duel,enum Buttons button);
struct Action *lookUpActionId(int id);
int getActionId(struct Action *);
void resetActor(struct Actor *actor);

struct Team *newTeam();
void addTeammate(struct Team *team,struct Character *character,int level);
int teamSaveSize(struct Team *team);
void saveTeam(struct Team *team,char *buffer,int length);
void loadTeam(struct Team *team,char *buffer,int length);
void freeTeam(struct Team *team);
void loadCharacters(struct Character *one,struct Character *two);
void addOneTeammate(struct Duel *duel);
#endif
