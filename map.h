#ifndef MAP_H
#define MAP_H
#include "main.h"
#include "duel.h"
// map.c
struct Map;
enum MapState { MS_ACTIVE, MS_DUEL, MS_ROYAL, MS_TOURNAMENT, MS_MENU };
typedef enum MapState MapState;
struct Map *initMap();
void drawMap(struct Map *map);
float getMapElev(struct Map *map,float x,float y);
void freeMap(struct Map *map);
void drawMap(struct Map *map);
int handleMap(struct Map *map,Buttons button);
int handleMapDown(struct Map *map,Buttons button);
int analogMap(struct Map *map,int lx,int ly);
MapState updateMap(struct Map *map,int elapsed);
PlaceType getActivePlace(struct Map *map);
int getActiveLevel(struct Map *map);
int mapCompletePercent(struct Map *map);
int applyDuelResult(struct Map *map,DuelState ds);
int mapSaveSize(struct Map *map);
void saveMapBuffer(struct Map *map,char *buffer,int size);
void loadMapBuffer(struct Map *map,char *buffer,int size);
#endif
