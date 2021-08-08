#ifdef _PSP
#include<pspctrl.h>
#include<pspgu.h>
#include<pspgum.h>
#else
//#ifndef ScePspFVector3
//typedef struct ScePspFVector3 { float x,y,z; } ScePspFVector3;
//typedef struct ScePspFVector4 { float x,y,z,w; } ScePspFVector4;
//#endif
#endif
#include "main.h"

struct AseMaterial {
	char name[256];
	unsigned long color;
	Image *image;
};

struct AseAnimationNode {
	int ticks;
	int flags;  // position update? orientation update?
	ScePspFVector3 pos;	// position
	ScePspFVector4 rot;	// orientation in axis/angle format
	ScePspFMatrix4 mat;	// matrix
};
#define AAN_POS 1   // position is updated
#define AAN_ROT 2   // rotation is updated
#define AAN_MAT 4	// matrix is replaced

struct AseAnimation {
	struct AseAnimationNode *node;	// must be temorally sorted
	int nodeCount;	// How many nodes there are in the animation
};

struct AseObject {
    char name[64];
    float matrix[16];	// at time 0.
    float unmatrix[16];	// inverse of default matrix
    int vertCount;
    Image *image;
    struct Vertex3DCNP *vert;
    struct Vertex3DTNP *vertImage;
    struct AseAnimation anim;
    // dynamic animation informaton
    //struct AseAnimationNode *lastNode;
    //struct AseAnimationNode *nextNode;
};

struct AseScene {
	char name[256];
	struct AseObject *object;
	int objectCount;
	struct AseMaterial *material;
	int materialCount;
	// general animation information
	int ticksPerFrame;	// SCENE_FRAMESPEED
	int firstFrame;	// SCENE_FIRSTFRAME
	int lastFrame;	// SCENE_LASTFRAME
	int fps;	// SCENE_FRAMESPEED
	int *keyframe;	// measured in ticks.
	int keyframeCount;	// the number of keyframes in the animation.
	// dynamic animation information
	int ticks;
};

/** Loads an AseScene, and all image assets that go with it.
 * \param fname the filename to load
 * \returns the scene with all images loaded or NULL on failure
 */
struct AseScene *loadAseScene(const char *fname);
/** Frees a previously loaded scene and all associated images
 * \param scene the scene to deallocate
 */
void freeAseScene(struct AseScene *scene);
/** updates the animation for the scene, to be ready to draw
 * \param scene the scene to update
 * \param elapsed the number of 1/1000ths of a second that elapsed since the last update/reset
 * \param maxFrame the terminus for the animation. once that frame is reached the animation stops
 * \returns 1 if maxFrame is reached, or 0 otherwise.
 */
int updateAseSceneMax(struct AseScene *scene,int elapsed,int maxFrame);
/** updates the animation for the scene, to be ready to draw from a specific frame of the animation
 * \param scene the scene to update
 * \param frame the new frame to reset to
 */
void resetAseScene(struct AseScene *scene,int frame);
/** draws the current frame of the animation for the scene based on the current clock and interpolation
 * \param scene the scene to draw
 */
void drawAseScene(struct AseScene *scene);
