#include <pspgu.h>
#include <pspgum.h>

#include "main.h"

void drawSprite(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);

	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);
	
	sceGuDisable(GU_DEPTH_TEST);
	int j = 0;
	while (j < width) {
		struct BlitVertex {
        unsigned short u,v;
        short x,y,z;
		} *vertices = (struct BlitVertex*) sceGuGetMemory(2 * sizeof(struct BlitVertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 1;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 1;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	sceGuEnable(GU_DEPTH_TEST);
	sceGuTexScale(1.0f, 1.0f);
}

void drawSpriteAlpha(int sx, int sy, int width, int height, Image* source, int dx, int dy, int alpha)
{
	//sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);

	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);
	
	sceGuDisable(GU_DEPTH_TEST);
	int j = 0;
	while (j < width) {
		struct BlitVertexAlpha {
        unsigned short u,v;
        u32 color;
        short x,y,z;
		} *vertices = (struct BlitVertexAlpha*) sceGuGetMemory(2 * sizeof(struct BlitVertexAlpha));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 1;
		vertices[0].color = GU_RGBA(255,255,255,alpha);
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 1;
		vertices[1].color = GU_RGBA(255,255,255,alpha);
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	sceGuEnable(GU_DEPTH_TEST);
	sceGuTexScale(1.0f, 1.0f);
}

void drawFilledRect(int x, int y, int width, int height, unsigned color)
{
    struct RectVertex {
		unsigned int color;
		short x,y,z;
	} *vert =(struct RectVertex *) sceGuGetMemory(2 * sizeof(struct RectVertex));
    vert[0].x = x;
    vert[0].y = y;
    vert[0].z = 0;
    vert[0].color = color;
    vert[1].x = x+width;
    vert[1].y = y+height;
    vert[1].z = 0;
    vert[1].color = color;
    sceGuDisable(GU_TEXTURE_2D);
    sceGuEnable(GU_BLEND);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2,0,vert);
    sceGuEnable(GU_TEXTURE_2D);
}	

