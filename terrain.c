#include <stdio.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"

struct VertexTerrain {
	char u,v;
	short nx,ny,nz;
	short x,y,z;
};

struct Terrain {
	Image *texture;
	struct Vertex3DTCNP *vert;
	int count;
	int w,h;
};

void terrainCalcNode(Image *elev,Image *col,int x,int z,struct Vertex3DTCNP *vert,int scale)
{
	vert->x=(x-elev->imageWidth/2)*scale;
	vert->z=(z-elev->imageHeight/2)*scale*2;
	if(x<0) x=0;
	if(z<0) z=0;
	if(x>elev->imageWidth-1) x=elev->imageWidth-1;
	if(z>elev->imageHeight-1) z=elev->imageHeight-1;
	int pixel=x+z*elev->textureWidth;
	int color=(elev->data[pixel]>>8)&255;
	vert->y=(color-128);
	vert->u=(x&1)<<1;
	vert->v=(z&1)<<1;
	if(col) vert->color=col->data[pixel];
	else vert->color=0xffffffff;
}

void terrainSetPos(Image *elev,int item,int x,int z,int scale)
{
	struct Vertex3DTCNP vert;
	terrainCalcNode(elev,NULL,x/32+elev->imageWidth/2,z/64+elev->imageHeight/2,&vert,scale);
	//printf("Moving item %d to %.2f,%.2f,%.2f\n",item,vert.x,vert.y,vert.z);
	setItemPos(item,vert.x,vert.y,vert.z);
}


struct Vertex3DTCNP *terrainGetVert(struct Terrain *terrain,int x,int z)
{
	x/=32;
	z/=64;
	x+=terrain->w/2;
	z+=terrain->h/2;
	if(x<0) x=0;
	if(z<0) z=0;
	if(x>terrain->w-1) x=terrain->w-1;
	if(z>terrain->h-1) z=terrain->h-1;
	int index=(x+z*terrain->w)*2;
	return terrain->vert+index;
}

void  terrainMoveItem(struct Terrain *terrain,int item,int x,int z)
{
	struct Vertex3DTCNP *vert=terrainGetVert(terrain,x,z);
	setItemPos(item,vert->x,vert->y,vert->z);
}

void terrainFillGrid(Image *elev,Image *color,struct Terrain *terrain,int scale)
{
	int i;
	int j;
	int corner;
	struct Vertex3DTCNP *next;
	int w=elev->imageWidth;
	int h=elev->imageHeight;
	terrain->w=w;
	terrain->h=h;
	terrain->count=h*(w*2);
	next=(struct Vertex3DTCNP *)malloc(terrain->count*sizeof(struct Vertex3DTCNP));
	terrain->vert=next;
	for(j=0;j<h;j++) {
		for(i=0;i<w;i++) {
			for(corner=0;corner<2;corner++) {
				int x=i;
				int z=j+corner;
				terrainCalcNode(elev,color,x,z,next,scale);

				ScePspFVector3 v1,v2;
				struct Vertex3DTCNP a,b;
				terrainCalcNode(elev,color,x-1,z,&a,scale);
				terrainCalcNode(elev,color,x+1,z,&b,scale);
				v1.x=a.x-b.x;
				v1.y=a.y-b.y;
				v1.z=a.z-b.z;
				terrainCalcNode(elev,color,x,z-1,&a,scale);
				terrainCalcNode(elev,color,x,z+1,&b,scale);
				v2.x=a.x-b.x;
				v2.y=a.y-b.y;
				v2.z=a.z-b.z;
				gumCrossProduct((ScePspFVector3 *)&next->nx,&v2,&v1);
				gumNormalize((ScePspFVector3 *)&next->nx);
				//printf("normal: %.2f,%.2f,%.2f\n",next->nx,next->ny,next->nz);
				next++;
			}
		}
	}
}

struct Terrain *terrainInit(const char *elevationFile)
{
	const int scale=32;
	
	struct Terrain *terrain=(struct Terrain *)malloc(sizeof(struct Terrain));
	terrain->texture=loadPng("data/detail4.png");
	swizzleFast(terrain->texture);

	Image *elev=loadPng(elevationFile);
	if(!elev) elev=newImage(16,16);

	terrainFillGrid(elev,NULL,terrain,scale);
	
	//printf("should I be positioning grass and trees!\n");
	//freeItems();
	//loadItems();
	if(getItemCount()>=11) {
		//printf("positioning grass and trees!\n");
		terrainSetPos(elev,0,128,128,scale);
		terrainSetPos(elev,1,-128,128,scale);
		terrainSetPos(elev,2,128,-128,scale);
		terrainSetPos(elev,3,-128,-128,scale);
		terrainSetPos(elev,4,192,48,scale);
		terrainSetPos(elev,5,-192,64,scale);
		terrainSetPos(elev,6,48,-192,scale);
		terrainSetPos(elev,7,48,192,scale);
		terrainSetPos(elev,8,-192,96,scale);
		terrainSetPos(elev,9,96,-192,scale);
		terrainSetPos(elev,10,192,0,scale);
	}
	freeImage(elev);
	return terrain;
}

struct Terrain *terrainDetailInit(const char *elevationFile,const char *colorFile)
{
	const int scale=32;
	
	struct Terrain *terrain=(struct Terrain *)malloc(sizeof(struct Terrain));
	terrain->texture=loadPng("data/detail.png");
	swizzleFast(terrain->texture);

	Image *color=loadPng(colorFile);
	Image *elev=loadPng(elevationFile);
	if(!color) color=newImage(16,16);
	if(!elev) elev=newImage(16,16);

	terrainFillGrid(elev,color,terrain,scale);
	
	//printf("should I be positioning grass and trees!\n");
	//freeItems();
	//loadItems();
#if 0
	if(getItemCount()>=11) {
		//printf("positioning grass and trees!\n");
		terrainSetPos(elev,0,128,128,scale);
		terrainSetPos(elev,1,-128,128,scale);
		terrainSetPos(elev,2,128,-128,scale);
		terrainSetPos(elev,3,-128,-128,scale);
		terrainSetPos(elev,4,192,48,scale);
		terrainSetPos(elev,5,-192,64,scale);
		terrainSetPos(elev,6,48,-192,scale);
		terrainSetPos(elev,7,48,192,scale);
		terrainSetPos(elev,8,-192,96,scale);
		terrainSetPos(elev,9,96,-192,scale);
		terrainSetPos(elev,10,192,0,scale);
	}
#endif
	freeImage(elev);
	freeImage(color);
	return terrain;
}

void drawTerrain(struct Terrain *terrain)
{
	// drawing terrain.
	if(!terrain) return;
	Image *source=terrain->texture;
	if(source) {
		sceGuTexScale(1.0f,1.0f);
		sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGB);
		sceGuEnable(GU_LIGHTING);
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight,source->textureWidth, source->data);
	}
	int i;
	//static int blah=0;
	//blah++;
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_LIGHTING);
	for(i=0;i<terrain->h;i++) {
		//if(blah&64)
		sceGumDrawArray(GU_TRIANGLE_STRIP,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_COLOR_8888|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,terrain->w*2,0,terrain->vert+i*terrain->w*2);
	}

	static struct Vertex3DTP *horizon=0,*ground;
	if(!horizon) horizon=malloc(sizeof(struct Vertex3DTP)*128*16*2);
	if(!ground) ground=malloc(sizeof(struct Vertex3DTP)*128*16*2);
	static struct Image *sky=0,*sea=0;
#if 0
	if(!sea) {
//		sea=loadPng("data/reef_down.png");
		swizzleFast(sea);
		struct Vertex3DTP *v=ground;
		int i,j,k;
#if 0
		for(j=0;j<16;j++) {
			for(i=0;i<128;i++) {
				float angle=i*(GU_PI*2/127);
				float x=cosf(angle);
				float z=sinf(angle);
				for(k=0;k<2;k++) {
					float radius=3000.0f*((j+k+5)/20.0f);
					v->u=x*((j+k+5)/20.0f)/2.0f+0.5f;
					v->v=z*((j+k+5)/20.0f)/2.0f+0.5f;
					v->x=x*radius;
					v->y=-128;
					v->z=z*radius;
					if((i%32)<2) printf("%d,%d,%d: %.2f,%.2f,%.2f (%.3f,%.3f)\n",i,j,k,v->x,v->y,v->z,v->u,v->v);
					v++;
				}
			}
		}
#else
		for(j=0;j<16;j++) {
			for(i=0;i<128;i++) {
				float angle=i*(GU_PI*2/127);
				float x,z;
				x=i/63.5f-1.0f;
				z=j/7.5f-1.0f;
				for(k=0;k<2;k++) {
					float radius=3000.0f;
					v->v=(x+1.0f)/2.0f;
					v->u=(z+k/7.5f+1.0f)/2.0f;
					v->x=x*radius;
					v->y=-127;
					v->z=(z+k/7.5f)*radius;
					if((i%32)<4) printf("%d,%d,%d: %.2f,%.2f,%.2f (%.3f,%.3f)\n",i,j,k,v->x,v->y,v->z,v->u,v->v);
					v++;
				}
			}
		}
#endif
		sceKernelDcacheWritebackAll();
	}
#endif
	if(!sky) {
		sky=loadPng("data/reef.png");
		swizzleFast(sky);
		struct Vertex3DTP *v=horizon;
		int i,j,k;
		for(j=0;j<16;j++) {
			for(i=0;i<128;i++) {
				float angle=i*(GU_PI*2/127);
				float x=cosf(angle);
				float z=sinf(angle);
				for(k=0;k<2;k++) {
					float radius=3000.0f*cosf((j+k-7.5f)/19.5f*GU_PI);
					v->u=angle*(1.0f/GU_PI/2.0f);
					v->v=(j+k)/17.0f;
					v->x=x*radius;
					v->y=(256-(j+k)*32)*4;
					v->z=z*radius;
					v++;
				}
			}
		}
		sceKernelDcacheWritebackAll();
	}
	sceGuDisable(GU_LIGHTING);
	if(sky && horizon) {
		source=sky;
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight,source->textureWidth, source->data);
		for(i=0;i<12;i++) {
			sceGumDrawArray(GU_TRIANGLE_STRIP,GU_VERTEX_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,128*2,0,horizon+128*2*i);
		}
	}
	if(sea && ground) {
		source=sea;
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight,source->textureWidth, source->data);
		for(i=0;i<16;i++) {
			sceGumDrawArray(GU_TRIANGLE_STRIP,GU_VERTEX_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,128*2,0,ground+128*2*i);
		}
	}
	sceGuEnable(GU_LIGHTING);

#ifdef CLIP_TERRAIN
	//printf("Terrain rendered clipped in %d cases.\n",clipped);
#endif
}

void setVertexColor(struct Terrain *terrain,unsigned long color)
{
	struct Vertex3DTCNP *v=terrain->vert;
	int i,j;
	for(j=0;j<terrain->h;j++) {
		for(i=0;i<terrain->w;i++) {
			v->color=color;
			v++;
			v->color=color;
			v++;
		}
	}
}

void freeTerrain(struct Terrain *terrain)
{
	if(terrain->texture) freeImage(terrain->texture);
	terrain->texture=0;
	if(terrain->vert) free(terrain->vert);
	terrain->vert=0;
	free(terrain);
}
