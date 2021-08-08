#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"main.h"

typedef struct MD2Header {
	int ident;
	int version;
	
	int skinwidth;
	int skinheight;
	
	int framesize;
	
	int numSkins;
	int numVertices;
	int numSt;
	int numTris;
	int numGlcmds;
	int numFrames;
	
	int offsetSkins;
	int offsetSt;
	int offsetTris;
	int offsetFrames;
	int offsetGlcmds;
	int offsetEnd;
} MD2Header;

typedef struct MD2Skin
{
	char name[64];
} MD2Skin;

typedef struct MD2TexCoord
{
	short s,t;
} MD2TexCoord;

typedef struct MD2Triangle
{
	unsigned short vertex[3];	// Index in compressed verticies
	unsigned short st[3];		// Texture coords.
} MD2Triangle;

typedef struct MD2Vertex
{
	unsigned char v[3];
	unsigned char normalIndex;
} MD2Vertex;

typedef struct MD2Frame
{
	float scale[3];
	float translate[3];
	char name[16];
	struct MD2Vertex *verts;
} MD2Frame;

typedef struct MD2Model
{
	int skinCount;
	int texCoordCount;
	int triangleCount;
	int frameCount;
	int glcmdCount;
	int vertexCount;
	
	int skinWidth,skinHeight;
	
	struct MD2Skin *skins;
	struct MD2TexCoord *texCoords;
	struct MD2Triangle *triangles;
	struct MD2Frame *frames;
	int *glcmds;
} MD2Model;

int md2alloc=0;

void frameToAnimMesh(MD2Model *m,struct AnimMesh *mesh,int frame)
{
	MD2Frame *f=m->frames+frame;
	struct AnimFrame *dest=mesh->frames+frame;
	int i;

	//printf("%d: Scale: %.2f,%.2f,%.2f Translate: %.2f,%.2f,%.2f\n",frame,f->scale[0],f->scale[1],f->scale[2],f->translate[0],f->translate[1],f->translate[2]);
	dest->scale[0]=f->scale[0];
	dest->scale[1]=f->scale[2];
	dest->scale[2]=f->scale[1];
	dest->translate[0]=f->translate[0];
	dest->translate[1]=-f->translate[2];
	dest->translate[2]=f->translate[1];
	dest->vert=(struct Vertex3DPfast *)malloc(sizeof(struct Vertex3DPfast)*mesh->polyCount*3);
	md2alloc+=sizeof(struct Vertex3DPfast)*mesh->polyCount*3;
	int vertCount=mesh->polyCount;
	for(i=0;i<vertCount;i++) {
		// Decompress a triangle from the frame.
		int j;
		for(j=0;j<3;j++) {
			int v0=m->triangles[i].vertex[j];
			dest->vert[i*3+j].x=f->verts[v0].v[0];
			dest->vert[i*3+j].y=-(f->verts[v0].v[2]);
			dest->vert[i*3+j].z=f->verts[v0].v[1];
		}
	}
}

struct AnimMesh *md2Load(const char *filename)
{
	struct AnimMesh *mesh=0;
	MD2Model m;
	memset(&m,0,sizeof(MD2Model));
	FILE *file=fopen(filename,"rb");
	
	if(!file) return 0;

	MD2Header header;
	fread(&header,sizeof(header),1,file);
	
	// Copy the header attributes.
	m.frameCount=header.numFrames;
	m.glcmdCount=header.numGlcmds;
	m.vertexCount=header.numVertices;
	m.skinCount=header.numSkins;
	m.texCoordCount=header.numSt;
	m.triangleCount=header.numTris;
	
	m.skinWidth=header.skinwidth;
	m.skinHeight=header.skinheight;
	
	fseek(file,header.offsetSkins,SEEK_SET);
	m.skins=(MD2Skin *)malloc(sizeof(MD2Skin)*header.numSkins);
	md2alloc+=sizeof(MD2Skin)*header.numSkins;
	fread(m.skins,sizeof(MD2Skin),header.numSkins,file);
	
	fseek(file,header.offsetSt,SEEK_SET);
	m.texCoords=(MD2TexCoord *)malloc(sizeof(MD2TexCoord)*header.numSt);
	md2alloc+=sizeof(MD2TexCoord)*header.numSt;
	fread(m.texCoords,sizeof(MD2TexCoord),header.numSt,file);
	
	fseek(file,header.offsetTris,SEEK_SET);
	m.triangles=(MD2Triangle *)malloc(sizeof(MD2Triangle)*header.numTris);
	md2alloc+=sizeof(MD2Triangle)*header.numTris;
	fread(m.triangles,sizeof(MD2Triangle),header.numTris,file);
	
	fseek(file,header.offsetFrames,SEEK_SET);
	m.frames=(MD2Frame *)malloc(sizeof(MD2Frame)*header.numFrames);
	md2alloc+=sizeof(MD2Frame)*header.numFrames;
	int i;
	for(i=0;i<m.frameCount;i++) {
		fread(m.frames+i,sizeof(MD2Frame)-4,1,file);
		m.frames[i].verts=(MD2Vertex *)malloc(sizeof(MD2Vertex)*m.vertexCount);
		md2alloc+=sizeof(MD2Vertex)*m.vertexCount;
		fread(m.frames[i].verts,sizeof(MD2Vertex),m.vertexCount,file);
	}
	
	fseek(file,header.offsetGlcmds,SEEK_SET);
	m.glcmds=(int *)malloc(sizeof(int)*header.numGlcmds);
	md2alloc+=sizeof(int)*header.numGlcmds;
	fread(m.glcmds,sizeof(int),header.numGlcmds,file);
	
	fclose(file);
	// Convert to AnimMesh format
	printf("PARSEmd2alloc=%d\n",md2alloc);
	
	mesh=(struct AnimMesh *)malloc(sizeof(struct AnimMesh));
	md2alloc+=sizeof(struct AnimMesh);
//	mesh->texture=loadCell(m.skins[0].name);
	mesh->texture=0;
	mesh->frameCount=m.frameCount;
	mesh->polyCount=m.triangleCount;
	mesh->skinWidth=m.skinWidth;
	mesh->skinHeight=m.skinHeight;
	mesh->frames=(struct AnimFrame *)malloc(mesh->frameCount*sizeof(struct AnimFrame));
	md2alloc+=mesh->frameCount*sizeof(struct AnimFrame);
	for(i=0;i<mesh->frameCount;i++) {
		frameToAnimMesh(&m,mesh,i);
	}
	mesh->uv=(struct Vertex3DTfast *)malloc(3*mesh->polyCount*sizeof(struct Vertex3DTfast));
	md2alloc+=3*mesh->polyCount*sizeof(struct Vertex3DTfast);
	for(i=0;i<mesh->polyCount;i++) {
		int j;
		for(j=0;j<3;j++) {
			mesh->uv[i*3+j].u=m.texCoords[m.triangles[i].st[j]].s;
			mesh->uv[i*3+j].v=m.texCoords[m.triangles[i].st[j]].t;
		}
	}
	printf("ALLOCmd2alloc=%d\n",md2alloc);
	
	// Clean up
	if(m.skins) {
		free(m.skins);
		m.skins=0;
		md2alloc-=sizeof(MD2Skin)*header.numSkins;
	}
	if(m.texCoords) {
		free(m.texCoords);
		m.texCoords=0;
		md2alloc-=sizeof(MD2TexCoord)*header.numSt;
	}
	if(m.triangles) {
		free(m.triangles);
		m.triangles=0;
		md2alloc-=sizeof(MD2Triangle)*header.numTris;
	}
	for(i=0;i<m.frameCount;i++) {
		if(m.frames[i].verts) {
			free(m.frames[i].verts);
			md2alloc-=sizeof(MD2Vertex)*m.vertexCount;
		}
	}
	if(m.frames) {
		free(m.frames);
		m.frames=0;
		md2alloc-=sizeof(MD2Frame)*header.numFrames;
	}
	if(m.glcmds) {
		free(m.glcmds);
		m.glcmds=0;
		md2alloc-=sizeof(int)*header.numGlcmds;
	}
	printf("CLEANUPmd2alloc=%d\n",md2alloc);
	
	return mesh;
}

void md2Free(struct AnimMesh *mesh)
{
	if(!mesh) return;
	if(mesh->texture) freeImage(mesh->texture);
	mesh->texture=0;
	if(mesh->frames) {
		int i;
		for(i=0;i<mesh->frameCount;i++) {
			if(mesh->frames[i].vert) {
				free(mesh->frames[i].vert);
				mesh->frames[i].vert=0;
				md2alloc-=sizeof(struct Vertex3DPfast)*mesh->polyCount*3;
			}
		}
		free(mesh->frames);
		md2alloc-=mesh->frameCount*sizeof(struct AnimFrame);
		mesh->frames=0;
	}
	if(mesh->uv) {
		free(mesh->uv);
		mesh->uv=0;
		md2alloc-=3*mesh->polyCount*sizeof(struct Vertex3DTfast);
	}
	free(mesh);
	md2alloc-=sizeof(struct AnimMesh);
	printf("FREEmd2alloc=%d\n",md2alloc);
}

// requires a buffer big enough to store sizeof(Vertex3DTPfast)*mesh->polyCount*3
void md2GetFractionalFrame(struct AnimMesh *mesh,int frame,int nextFrame,float fraction,struct Vertex3DTP *vert,float *minMax)
{
	int i,j;
	for(i=0;i<6;i++) {
		minMax[i]=0;
	}
	if(frame<0 || frame>=mesh->frameCount) {
		printf("frame (%d) out of bounds.\n",frame);
		frame=0;
	}
	if( nextFrame<0 || nextFrame>=mesh->frameCount) {
		printf("nextFrame (%d) out of bounds.\n",nextFrame);
		nextFrame=0;
	}
	float one_fraction=1.0f-fraction;
	float s1[3],t1[3],s2[3],t2[3];
	for(i=0;i<3;i++) {
		s1[i]=mesh->frames[frame].scale[i];
		t1[i]=mesh->frames[frame].translate[i];
		s2[i]=mesh->frames[nextFrame].scale[i];
		t2[i]=mesh->frames[nextFrame].translate[i];
	}
	float invSkinWidth=1.0f/mesh->skinWidth,invSkinHeight=1.0f/mesh->skinHeight;
	for(i=0;i<mesh->polyCount;i++) {
		for(j=0;j<3;j++) {
			int index=i*3+j;
			float x1,y1,z1,x2,y2,z2,x,y,z;
			x1=mesh->frames[frame].vert[index].x*s1[0]+t1[0];
			y1=mesh->frames[frame].vert[index].y*s1[1]+t1[1];
			z1=mesh->frames[frame].vert[index].z*s1[2]+t1[2];
			x2=mesh->frames[nextFrame].vert[index].x*s2[0]+t2[0];
			y2=mesh->frames[nextFrame].vert[index].y*s2[1]+t2[1];
			z2=mesh->frames[nextFrame].vert[index].z*s2[2]+t2[2];
			x=x1*one_fraction+x2*fraction;
			y=y1*one_fraction+y2*fraction;
			z=z1*one_fraction+z2*fraction;
			// Reorient right side up?
			vert[index].x=-z;
			vert[index].y=-y;
			vert[index].z=-x;
			if(index==0 || -z<minMax[0]) minMax[0]=-z;
			if(index==0 || -z>minMax[3]) minMax[3]=-z;
			if(index==0 || -y<minMax[1]) minMax[1]=-y;
			if(index==0 || -y>minMax[4]) minMax[4]=-y;
			if(index==0 || -x<minMax[2]) minMax[2]=-x;
			if(index==0 || -x>minMax[5]) minMax[5]=-x;
			vert[index].u=mesh->uv[index].u*invSkinWidth;
			//vert[index].v=1.0f-mesh->uv[index].v*invSkinHeight;
			vert[index].v=mesh->uv[index].v*invSkinHeight;
		}
	}
}

// requires a buffer big enough to store sizeof(Vertex3DTPfast)*mesh->polyCount*6
void md2UpdateDoubleFrame(struct AnimMesh *mesh,int frame,struct Vertex3DTPfast *vert)
{
	int i,j;
	if(frame<0 || frame>=mesh->frameCount) {
		printf("frame (%d) out of bounds.\n",frame);
		frame=0;
	}
	for(i=0;i<mesh->polyCount;i++) {
		for(j=0;j<3;j++) {
			int dest=i*6+j;
			int src=i*3+j;
			vert[dest].x=mesh->frames[frame].vert[src].x;
			vert[dest].y=mesh->frames[frame].vert[src].y;
			vert[dest].z=mesh->frames[frame].vert[src].z;
			vert[dest].u=mesh->uv[src].u;
			vert[dest].v=mesh->uv[src].v;
		}
	}
}
