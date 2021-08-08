/* Main */
#ifndef MAIN_H
#define MAIN_H
#ifdef _PSP
#include <pspgum.h>
#else
#ifndef ScePspFVector3
typedef struct ScePspFVector3 { float x,y,z; } ScePspFVector3;
typedef struct ScePspFVector4 { float x,y,z,w; } ScePspFVector4;
#endif
typedef struct ScePspFMatrix4 { ScePspFVector4 x,y,z,w; } ScePspFMatrix4;
#endif

#ifdef _PSP
#include <unistd.h>
#include "intraFont.h"
#endif

// main.c
extern unsigned int __attribute__((aligned(16))) gulist[162144];
extern void *drawBuffer;
extern int exitRequest;
int running();
struct Vertex3DCNP {
	unsigned long color;
	float nx,ny,nz;
	float x,y,z;
};
struct Vertex3DTNP {
	float u,v;
	float nx,ny,nz;
	float x,y,z;
};
struct Vertex3DCP {
	unsigned long color;
	float x,y,z;
};
struct Vertex3DTCP {
	float u,v;
	unsigned long color;
	float x,y,z;
};
struct Vertex3DTCNP {
	float u,v;
	unsigned long color;
	float nx,ny,nz;
	float x,y,z;
};
struct Vertex3DTP {
	float u,v;
	float x,y,z;
};
struct Vertex3DTPfast {
	short u,v;
	short x,y,z;
};
struct Vertex3DCPfast {
	unsigned long color;
	short x,y,z;
};
struct Vertex3DTfast {
	short u,v;
};
struct Vertex3DPfast {
	short x,y,z;
};

enum Buttons {
	BT_UP,BT_DOWN,BT_LEFT,BT_RIGHT,
	BT_TRIANGLE,BT_CIRCLE,BT_SQUARE,BT_CROSS,
	BT_LTRIGGER,BT_RTRIGGER,
	BT_START,BT_SELECT,BT_HOLD,
	BT_NONE
};
typedef enum Buttons Buttons;
#ifdef _PSP
extern intraFont *iFont[6];
#endif

// image.c
typedef unsigned int Color;
typedef struct Image
{
        int textureWidth;  // the real width of data, 2^n with n>=0
        int textureHeight;  // the real height of data, 2^n with n>=0
        int imageWidth;  // the image width
        int imageHeight;	// the image height
        int isSwizzled;	// Is the image swizzled?
        int vram;		// Is the image in vram or not?
        int format;		// default is GU_COLOR_8888
        Color* data;
        Color* palette;	// used for 4 bpp and 8bpp modes.
        char filename[128];	// for debug purposes
} Image;
Image *loadPng(const char *filename);
void freeImage(Image *image);
void resetVRam();
void reportVRam();
extern int swizzleToVRam;
void swizzleFast(Image *source);
void saveImagePng(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha);
void saveImageTarga(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha);
enum FontId {
	FONT_HEADLINE,FONT_BODY,FONT_BODYHIGHLIGHT,FONT_MESSAGE,FONT_SMALL,FONT_SMALLHIGHLIGHT
};
void initFastFont();
void extentMessage(int *w,int *h, enum FontId fontId, const char *message);
void drawMessage(int x,int y, enum FontId fontId, const char *message);
void drawMessageAlpha(int x,int y, enum FontId fontId, const char *message,int alpha);
void drawMessageFormat(int x,int y,enum FontId fontId, const char *message);
Image *loadCell(const char *fname);
void drawCell(int x,int y,const char *fname);
void freeCells();
Image *newImage(int width,int height);

// sound.c
enum Sfx {
	SFX_MENUNEXT,SFX_MENUSELECT,SFX_TARGET,SFX_CHAOS,SFX_PAIN,SFX_EXPLODE,SFX_POWERUP,SFX_WIN,SFX_LOSE,SFX_BOSSDAMAGE,MAX_SFX
};
enum Song {
	SONG_INTRO,SONG_EVEN,SONG_ODD,SONG_MENU
};
void initSound();
void updateSound();
void playSfx(enum Sfx id);
void playSound(int id);
void playSong(int id);
void playWav(const char *fname);
void freeWavs();
void songVolume(int vol);

// gutools.c
void drawSprite(int sx, int sy, int width, int height, Image* source, int dx, int dy);
void drawSpriteAlpha(int sx, int sy, int width, int height, Image* source, int dx, int dy,int alpha);
void drawFilledRect(int x, int y, int width, int height, unsigned color);

// md2fast.h
struct AnimFrame {
	float scale[3];
	float translate[3];
	struct Vertex3DPfast *vert;
};
struct AnimMesh {
	Image *texture;
	int frameCount;
	int polyCount;
	int skinWidth,skinHeight;
	struct AnimFrame *frames;
	struct Vertex3DTfast *uv;
};
struct AnimMesh *md2Load(const char *fname);
void md2GetFractionalFrame(struct AnimMesh *mesh,int frame,int nextFrame,float fraction,struct Vertex3DTP *buffer,float *minMax);
void md2Free(struct AnimMesh *mesh);

// battlearena.c
struct AnimMotion {
	int startFrame;
	int endFrame;
	int fps;
};
enum MotionType { MT_STAND, MT_RUN, MT_ATTACK, MT_PAIN_A, MT_PAIN_B, MT_PAIN_C,
MT_JUMP, MT_FLIP, MT_SALUTE, MT_FALLBACK, MT_WAVE, MT_POINT, MT_CROUCH_STAND, 
MT_CROUCH_WALK, MT_CROUCH_ATTACK, MT_CROUCH_PAIN, MT_CROUCH_DEATH, 
MT_DEATH_FALLBACK, MT_DEATH_FALLFORWARD, MT_DEATH_FALLBACKSLOW, MT_BOOM };
extern const char *animMotionName[];
extern struct AnimMotion animMotion[];
void startTitle();
void newGame();
void update(unsigned long elapsed);
int handleJoy(enum Buttons button,int up);
int handleAnalog(int lx,int ly);
void draw();
int getTimeLeft();
void setTimeLeft(int duration);
void showFreeMem();
void addMessage(const char *message,int timer);
extern ScePspFMatrix4 projection;
extern ScePspFMatrix4 view;
float getGiggle();
int getMaxTeamLevel();
void addInventory(const char *name);
// menu.c
extern struct Options {
	int invert;		// invert y: 1=on 0=off
	float speed;	// defaults to 2.4f
	int music;		// volume 0 to 10
	int sfx;		// 1=on 0=off
} options;
//void setMode(enum GameMode mode);
void drawMenu();
void loadOptions();
void cameraSetFromToUp(float fx,float fy,float fz,float tx,float ty,float tz,float ux,float uy,float uz);
void cameraSetFromTo(float fx,float fy,float fz,float tx,float ty,float tz);
void cameraSetFrom(float fx,float fy,float fz);
void cameraGetFromTo(float *fx,float *fy,float *fz,float *tx,float *ty,float *tz);
extern ScePspFVector3 from,to,up;

// wavefront.c
int loadItemImage(const char *fname);
void loadItems();
void freeItems();
int getItemCount();
void setItemPos(int i,float x,float y,float z);
void renderItem(int i);
float *getItemMin(int i);
float *getItemMax(int i);

// terrain.c
struct Terrain;
struct Terrain *terrainInit(const char *fname);
struct Terrain *terrainDetailInit(const char *elevFname,const char *colorFname);
void drawTerrain(struct Terrain *terrain);
void freeTerrain(struct Terrain *terrain);
void  terrainMoveItem(struct Terrain *terrain,int item,int x,int z);
void setVertexColor(struct Terrain *terrain,unsigned long color);

// clip.c
struct Mesh {
	struct Vertex3DTNP *vertImage;
	int vertCount;
	Image *image;
	int wideCount;	// number of vertexes with long sides, for clipping
	float boundingRect[6];
};
typedef struct Mesh Mesh;

// savejpeg.c
void saveImageJpeg(const char* filename, Color* data, int width, int height, int lineSize);

#endif
