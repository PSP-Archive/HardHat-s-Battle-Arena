#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>

#ifdef _PSP
#include<pspctrl.h>
#include<pspgu.h>
#include<pspgum.h>
#endif
#include "main.h"

extern int showClipping;
extern int showScore;

struct MaterialGroup {
	int first;	// vertex
	int last; // vertex
	Image *image;
	int wideCount;	// measured in vertexes.
	float boundingRect[6];
};

int cmpMaterialGroupImage(const void *one,const void *two)
{
	const struct MaterialGroup *a=(struct MaterialGroup *)one,*b=(struct MaterialGroup *)two;
	
	return a->image==b->image?0:a->image<b->image?-1:1;
}

struct Item {
    char name[64];
    float matrix[16];
    int groupCount;
    struct MaterialGroup *group;
    int vertCount;
    struct Image *image;
    struct Vertex3DCNP *vert;
    struct Vertex3DTNP *vertImage;
    int polyCount;
    unsigned short *poly;
    float min[3];
    float max[3];	// handy for collision detection.
};
#define MAXITEMS 64
struct Item item[MAXITEMS];
int nextItem;

struct Vertex3DTNP *subdivVertsImage(struct Vertex3DTNP *in,int *inCount,float maxSide)
{
	// itterate through the verts, until there are none with sides longer than max
	if(in==0) return in;
	printf("In: %d verts (%d poly)\n",*inCount,(*inCount)/3);
	int i;
	int extra=1;
	int maxsq=maxSide*maxSide;
	while(extra) {
		int vertCount=*inCount;
		extra=0;
		for(i=0;i<vertCount;i+=3) {
			float lensq[3];
			int j;
			for(j=0;j<3;j++) {
				int from=i+j;
				int to=i+((j+1)%3);

				float dx,dy,dz;
				dx=in[from].x-in[to].x;
				dy=in[from].y-in[to].y;
				dz=in[from].z-in[to].z;
				lensq[j]=dx*dx+dy*dy+dz*dz;
			}
			if(lensq[0]>maxsq || lensq[1]>maxsq || lensq[2]>maxsq) extra++;
		}
		//printf("subdividing %d polygons in this pass.\n",extra);
		//printf("in was: %08x; ",(int)in);
		struct Vertex3DTNP *out=in;
		in=(struct Vertex3DTNP *)realloc(in,sizeof(struct Vertex3DTNP)*(vertCount+extra*3));
		//printf("in is: %08x; ",(int)in);
		if(!in) {
			printf("Crashed...out of memory during a subdiv.\n");
			return out;
		}
		int pos=vertCount;
		float maxlensq=0;
		for(i=0;i<vertCount;i+=3) {
			float lensq[3];
			int j;
			for(j=0;j<3;j++) {
				int from=i+j;
				int to=i+((j+1)%3);

				float dx,dy,dz;
				dx=in[from].x-in[to].x;
				dy=in[from].y-in[to].y;
				dz=in[from].z-in[to].z;
				lensq[j]=dx*dx+dy*dy+dz*dz;
				if(lensq[j]>maxlensq) {
					maxlensq=lensq[j];
				}
			}
			if(lensq[0]>maxsq || lensq[1]>maxsq || lensq[2]>maxsq) {
				// divide longest side.
				int side=2;
				if(lensq[0]>lensq[1] && lensq[0]>lensq[2]) {
					side=0;
				} else if(lensq[1]>lensq[2]) {
					side=1;
				} else {
					side=2;
				}
				int from=side;
				int to=((side+1)%3);
				in[pos]=in[i];
				in[pos+1]=in[i+1];
				in[pos+2]=in[i+2];
				in[i+to].x=(in[i+from].x+in[i+to].x)/2.0f;
				in[i+to].y=(in[i+from].y+in[i+to].y)/2.0f;
				in[i+to].z=(in[i+from].z+in[i+to].z)/2.0f;
				in[i+to].u=(in[i+from].u+in[i+to].u)/2;
				in[i+to].v=(in[i+from].v+in[i+to].v)/2;
				in[pos+from]=in[i+to];
				pos+=3;
			}
		}
		*inCount=pos;
	}
	printf("Out: %d verts (%d poly)\n",*inCount,(*inCount)/3);
	return in;
}

inline float wideVertsImage(const struct Vertex3DTNP *v)
{
	int j;
	float wide=0;

	for(j=0;j<3;j++) {
		int from=j;
		int to=((j+1)%3);

		float dx,dy,dz,lensq;
		dx=v[from].x-v[to].x;
		dy=v[from].y-v[to].y;
		dz=v[from].z-v[to].z;
		lensq=dx*dx+dy*dy+dz*dz;
		if(lensq>wide) wide=lensq;
	}

	return wide;
}

int sortVertsImage(const void *one,const void *two)
{
	const struct Vertex3DTNP *a=(const struct Vertex3DTNP *)one;
	const struct Vertex3DTNP *b=(const struct Vertex3DTNP *)two;

	float aLenSq=wideVertsImage(a);
	float bLenSq=wideVertsImage(b);
	return aLenSq==bLenSq?0:aLenSq<bLenSq?1:-1;
}

int loadItemImage(const char *fname)
{
	char path[256];
	struct Vertex3DTP *vertImage=0;
	struct Material {
		char name[128];
		unsigned long color;
		Image *image;
	} material[40];
	int nextMaterial=0;
	int fCount=0;
	int gCount=0;
	int itemId=0;

	//printf("read texture for '%s'\n",fname);
	sprintf(path,"data/%s.png",fname);
	item[nextItem].image=loadPng(path);
	if(!item[nextItem].image) {
		sprintf(path,"models/%s/%s.png",fname,fname);
		item[nextItem].image=loadPng(path);
	}
	if(!item[nextItem].image) item[nextItem].image=newImage(8,8);
	//swizzle_fast(item[nextItem].image);
	swizzleToVRam=0;
	swizzleFast(item[nextItem].image);
	if(!item[nextItem].image) printf("*** Could not load texture '%s'\n",path);

	//printf("read materials for '%s'\n",fname);
	sprintf(path,"models/%s.mtl",fname);
//	gzFile file=gzopen(path,"r");
	FILE *file=fopen(path,"r");
		strcpy(item[nextItem].name,fname);
	if(file) {
		// Read in the materials.
		char line[256];
		line[255]=0;
		int mat=nextMaterial;

//		while( gzgets(file,line,255) ) {
		while( fgets(line,255,file) ) {
			char cmd[64];

			cmd[0]=0;
			sscanf(line,"%s",cmd);
			if(strchr(line,'\r')) strchr(line,'\r')[0]=0;
			if(strchr(line,'\n')) strchr(line,'\n')[0]=0;

			if(strcmp(cmd,"newmtl")==0 ) {
				mat=nextMaterial;
				nextMaterial++;
				strcpy(material[mat].name,line+7);
				//printf("Located material '%s'\n",material[mat].name);
			} else if( strcmp(cmd,"Kd")==0) {
				float rf=0,gf=0,bf=0;
				unsigned long r,g,b;
				sscanf(line,"Kd%f%f%f",&rf,&gf,&bf);
				r=rf*255;
				g=gf*255;
				b=bf*255;
				material[mat].color=(r)|(g<<8)|(b<<16)|(255<<24);
				//printf("%s is %d/%d/%d\n",material[mat].name,(int)r,(int)g,(int)b);
			} else if( strcmp(cmd,"map_Kd")==0) {
				char path[256];
				char *s=line;
				while (s[0]==' ' || s[0]=='\t') s++;	// skip white space
				s+=6;
				while(s[0]==' ' || s[0]=='\t') s++;		// skip white space
				sprintf(path,"models/%s/%s",fname,s);
				//printf("Line says '%s'\n",line);
				//printf("Image '%s'\n",path);
				material[mat].image=loadPng(path);
				// should search one more place.
				if(material[mat].image->textureWidth>64 || material[mat].image->textureHeight>64) swizzleToVRam=1; else swizzleToVRam=0;
				swizzleFast(material[mat].image);
				if(!material[mat].image) printf("Couldn't locate '%s'\n",path);
			}

		}
		fclose(file);
//		gzclose(file);
		printf("read %d materials for %s\n",nextMaterial,fname);
	}

	printf("read obj for '%s'\n",fname);
	sprintf(path,"models/%s.obj",fname);
	//sprintf(path,"data/intro/%s.obj.gz",fname);
//	file=gzopen(path,"r");
	file=fopen(path,"r");

	if(!file) printf("Couldn't find %s\n",path);
	if(file) {
		// Count the verts.
		char line[256];
		line[255]=0;
		int i;
		int v=0;
		int vt=0;
		int f=0;
		int g=0;

//		while( gzgets(file,line,255) ) {
		while( fgets(line,255,file) ) {
//printf("Pass 1: %s",line);
			switch(line[0]) {
			case 'v':
				if(line[1]==' ') v++;
				if(line[1]=='t') vt++;
				break;
			case 'f':
				f+=3;
				break;
			case 'g':
				if(strlen(line)>2) g++;
				break;
			}
		}
		printf("Found %d image verts, %d texture verts, %d groups and %d faces.\n",v,vt,g,f);
		strcpy(item[nextItem].name,fname);
		item[nextItem].groupCount=g;
		gCount=g;
		item[nextItem].group=(struct MaterialGroup *)calloc(sizeof(struct MaterialGroup),g);
		item[nextItem].polyCount=f;
		item[nextItem].poly=(unsigned short *)calloc(sizeof(short),f);
		item[nextItem].vertCount=v;
		vertImage=(struct Vertex3DTP *)calloc(sizeof(struct Vertex3DTP),v<vt?vt:v);
		item[nextItem].vertImage=(struct Vertex3DTNP *)calloc(sizeof(struct Vertex3DTNP),f);
		fCount=f;
		//printf("item[nextItem].vert=%08x\n",(int)item[nextItem].vert);
		for(i=0;i<16;i++) {
			item[nextItem].matrix[i]=(i%4)==(i/4)?1:0;
		}
		//gzclose(file);
		fclose(file);
		file=0;
	}
//	file=gzopen(path,"r");
	file=fopen(path,"r");
	if(file) {
		// actual verts
		char line[256];
		line[255]=0;
		int v=0;
		int vt=0;
		int g=0;
		int f=0;
		int m=0;
		float min[3]={0,0,0},max[3]={0,0,0};

//		while( gzgets(file,line,255) ) {
		while( fgets(line,255,file)) {
			float tu,tv,tw;
			float x,y,z;
			int f1,f2,f3;	// face vertex
			int n1,n2,n3;	// normal
			int t1,t2,t3;	// texture
			char name[64];
			if(strrchr(line,13)) strrchr(line,13)[0]=0;
			if(strrchr(line,10)) strrchr(line,10)[0]=0;

			if(sscanf(line,"v%f%f%f",&x,&y,&z)==3) {
#if 0
			    x*=(1/8.0f);
			    y*=(1/8.0f);
			    z*=(1/8.0f);
#endif
				if(x<min[0]) min[0]=x;
				if(y<min[1]) min[1]=y;
				if(z<min[2]) min[2]=z;
				if(x>max[0]) max[0]=x;
				if(y>max[1]) max[1]=y;
				if(z>max[2]) max[2]=z;
				if(v==0) { min[0]=x; min[1]=y; min[2]=z; max[0]=x; max[1]=y; max[2]=z; }
				vertImage[v].x=x;
				vertImage[v].y=y;
				vertImage[v].z=z;
				v++;
				//if(v>item[nextItem].vertCount) {
					//printf("At indexed vert %d of %d\n",v,item[nextItem].vertCount);
				//}
			} else if(sscanf(line,"vt%f%f%f",&tu,&tv,&tw)==3 || sscanf(line,"vt%f%f",&tu,&tv)==2) {
				vertImage[vt].u=tu;
				vertImage[vt].v=1.0f-tv;
				vt++;
				//if(v>item[nextItem].vertCount) {
					//printf("At indexed vert %d of %d\n",v,item[nextItem].vertCount);
				//}
			} else if(line[0]=='g') {
				if(strlen(line)<3 && v>0) {
					// end of object
					//printf("End of object: ");
					item[nextItem].group[g].last=f;
					// consolidate multiple objects with same material:
					if(g>0 && item[nextItem].group[g-1].image==item[nextItem].group[g].image) {
						g--;
						item[nextItem].group[g].last=f;
						printf("consolidated: ");
					} else {
						printf("Got group %d %s at face %d\n",g,strlen(line)>2?line+2:"",f);
					}
					// now sort them biggest to smallest
					int pc=(item[nextItem].group[g].last-item[nextItem].group[g].first)/3;
					qsort(item[nextItem].vertImage+item[nextItem].group[g].first,pc,sizeof(struct Vertex3DTNP)*3,sortVertsImage);
					int k;
					int wideCount=0;
					float boundingRect[6]={0,0,0,0,0,0};
					if(pc>0) {
						struct Vertex3DTNP *v=item[nextItem].vertImage+item[nextItem].group[g].first;
						boundingRect[0]=boundingRect[3]=v->x;
						boundingRect[1]=boundingRect[4]=v->y;
						boundingRect[2]=boundingRect[5]=v->z;
					}
					for(k=item[nextItem].group[g].first;k<item[nextItem].group[g].last;k+=3) {
						float wide=wideVertsImage(item[nextItem].vertImage+k);
						if(wide>250*250) wideCount++;
						int j;
						for(j=0;j<3;j++) {
							struct Vertex3DTNP *v=item[nextItem].vertImage+k+j;
							if(boundingRect[0]>v->x) boundingRect[0]=v->x;
							if(boundingRect[3]<v->x) boundingRect[3]=v->x;
							if(boundingRect[1]>v->y) boundingRect[1]=v->y;
							if(boundingRect[4]<v->y) boundingRect[4]=v->y;
							if(boundingRect[2]>v->z) boundingRect[2]=v->z;
							if(boundingRect[5]<v->z) boundingRect[5]=v->z;
						}
					}
					item[nextItem].group[g].wideCount=wideCount*3;
					for(k=0;k<6;k++) item[nextItem].group[g].boundingRect[k]=boundingRect[k];
					printf("Of %d verts, %d are wide.\n",(item[nextItem].group[g].last-item[nextItem].group[g].first)/3,wideCount);
					g++;
					item[nextItem].groupCount=g;
				} else {
					// start of object.
					item[nextItem].group[g].first=f;
					item[nextItem].group[g].last=f;
					//item[nextItem].group[g].image=0;
				}
			} else if(sscanf(line,"usemtl %s",name)>0) {
				for(m=0;m<nextMaterial;m++) {
					if(strcmp(name,material[m].name)==0) break;
				}
				if(m==nextMaterial) {
					printf("Material error.  Couldn't find %s\n",name);
					m=0;	// bad file
				}
				printf("Applying material %s (%d) to group %d (face %d)\n",name,m,g,f);

				item[nextItem].group[g].image=material[m].image;
				//while(gv<v) {
				//	vert[gv++].color=material[m].color;
				//}
			} else if(sscanf(line,"f%d/%d/%d%d/%d/%d%d/%d/%d",&f1,&t1,&n1,&f2,&t2,&n2,&f3,&t3,&n3)==9||
				sscanf(line,"f%d/%d%d/%d%d/%d",&f1,&t1,&f2,&t2,&f3,&t3)==6) {
				if(f1<0) f1=v-f1+1;
				if(f2<0) f2=v-f2+1;
				if(f3<0) f3=v-f3+1;
				if(t1<0) t1=vt-t1+1;
				if(t2<0) t2=vt-t2+1;
				if(t3<0) t3=vt-t3+1;
				item[nextItem].vertImage[f].x=vertImage[f1-1].x;
				item[nextItem].vertImage[f].y=vertImage[f1-1].y;
				item[nextItem].vertImage[f].z=vertImage[f1-1].z;
				item[nextItem].vertImage[f].u=vertImage[t1-1].u;
				item[nextItem].vertImage[f].v=vertImage[t1-1].v;
				item[nextItem].poly[f++]=f1-1;
				item[nextItem].vertImage[f].x=vertImage[f2-1].x;
				item[nextItem].vertImage[f].y=vertImage[f2-1].y;
				item[nextItem].vertImage[f].z=vertImage[f2-1].z;
				item[nextItem].vertImage[f].u=vertImage[t2-1].u;
				item[nextItem].vertImage[f].v=vertImage[t2-1].v;
				item[nextItem].poly[f++]=f2-1;
				item[nextItem].vertImage[f].x=vertImage[f3-1].x;
				item[nextItem].vertImage[f].y=vertImage[f3-1].y;
				item[nextItem].vertImage[f].z=vertImage[f3-1].z;
				item[nextItem].vertImage[f].u=vertImage[t3-1].u;
				item[nextItem].vertImage[f].v=vertImage[t3-1].v;
				item[nextItem].poly[f++]=f3-1;
				float v1[3],v2[3];
				float nx,ny,nz;
				v1[0]=item[nextItem].vertImage[f-2].x-item[nextItem].vertImage[f-3].x;
				v1[1]=item[nextItem].vertImage[f-2].y-item[nextItem].vertImage[f-3].y;
				v1[2]=item[nextItem].vertImage[f-2].z-item[nextItem].vertImage[f-3].z;
				v2[0]=item[nextItem].vertImage[f-1].x-item[nextItem].vertImage[f-3].x;
				v2[1]=item[nextItem].vertImage[f-1].y-item[nextItem].vertImage[f-3].y;
				v2[2]=item[nextItem].vertImage[f-1].z-item[nextItem].vertImage[f-3].z;
				// A x B = <Ay*Bz - Az*By, Az*Bx - Ax*Bz, Ax*By - Ay*Bx>
				nx=v1[1]*v2[2]-v1[2]*v2[1];
				ny=v1[2]*v2[0]-v1[0]*v2[2];
				nz=v1[0]*v2[1]-v1[1]*v2[0];
				float nw=sqrtf(nx*nx+ny*ny+nz*nz);
				if(nw==0) nw=1;
				nx=nx/nw;
				ny=ny/nw;
				nz=nz/nw;
				item[nextItem].vertImage[f-3].nx=nx;
				item[nextItem].vertImage[f-3].ny=ny;
				item[nextItem].vertImage[f-3].nz=nz;
				item[nextItem].vertImage[f-2].nx=nx;
				item[nextItem].vertImage[f-2].ny=ny;
				item[nextItem].vertImage[f-2].nz=nz;
				item[nextItem].vertImage[f-1].nx=nx;
				item[nextItem].vertImage[f-1].ny=ny;
				item[nextItem].vertImage[f-1].nz=nz;
			}
		}
		fclose(file);
//		gzclose(file);

#if 0
		if(item[nextItem].group[g].last==item[nextItem].group[g].first) {
			item[nextItem].group[g].last=f;
			// consolidate multiple objects with same material:
			if(g>0 && item[nextItem].group[g-1].image==item[nextItem].group[g].image) {
				g--;
				item[nextItem].group[g].last=f;
				printf("consolidated: ");
			} else {
				printf("Got group %d %s at face %d\n",g,strlen(line)>2?line+2:"",f);
			}
			// now sort them biggest to smallest
			int pc=(item[nextItem].group[g].last-item[nextItem].group[g].first)/3;
			qsort(item[nextItem].vertImage+item[nextItem].group[g].first,pc,sizeof(struct Vertex3DTNP)*3,sortVertsImage);
			int k;
			int wideCount=0;
			float boundingRect[6]={0,0,0,0,0,0};
			if(pc>0) {
				struct Vertex3DTNP *v=item[nextItem].vertImage+item[nextItem].group[g].first;
				boundingRect[0]=boundingRect[3]=v->x;
				boundingRect[1]=boundingRect[4]=v->y;
				boundingRect[2]=boundingRect[5]=v->z;
			}
			for(k=item[nextItem].group[g].first;k<item[nextItem].group[g].last;k+=3) {
				float wide=wideVertsImage(item[nextItem].vertImage+k);
				if(wide>250*250) wideCount++;
				int j;
				for(j=0;j<3;j++) {
					struct Vertex3DTNP *v=item[nextItem].vertImage+k+j;
					if(boundingRect[0]>v->x) boundingRect[0]=v->x;
					if(boundingRect[3]<v->x) boundingRect[3]=v->x;
					if(boundingRect[1]>v->y) boundingRect[1]=v->y;
					if(boundingRect[4]<v->y) boundingRect[4]=v->y;
					if(boundingRect[2]>v->z) boundingRect[2]=v->z;
					if(boundingRect[5]<v->z) boundingRect[5]=v->z;
				}
			}
			item[nextItem].group[g].wideCount=wideCount*3;
			for(k=0;k<6;k++) item[nextItem].group[g].boundingRect[k]=boundingRect[k];
			printf("Of %d verts, %d are wide.\n",(item[nextItem].group[g].last-item[nextItem].group[g].first)/3,wideCount);
			g++;
			item[nextItem].groupCount=g;
		}
#endif

		printf("Min: %.2f,%.2f,%.2f; Max: %.2f,%.2f,%.2f\n",min[0],min[1],min[2],max[0],max[1],max[2]);
		float centre[3];
		centre[0]=(max[0]+min[0])/2;
		centre[1]=(max[1]+min[1])/2;
		centre[2]=(max[2]+min[2])/2;
/*
		int i;
		for(i=0;i<fCount;i++) {
			item[nextItem].vertImage[i].x-=centre[0];
			item[nextItem].vertImage[i].y-=centre[1];
			item[nextItem].vertImage[i].z-=centre[2];
		}
*/
		item[nextItem].min[0]=min[0];
		item[nextItem].min[1]=min[1];
		item[nextItem].min[2]=min[2];
		item[nextItem].max[0]=max[0];
		item[nextItem].max[1]=max[1];
		item[nextItem].max[2]=max[2];
		free(vertImage);
		vertImage=0;
		free(item[nextItem].poly);
		item[nextItem].poly=0;
		itemId=nextItem;
		nextItem++;
	}
	return itemId;
}

int trophyId[6];
int chestId;
int shardId;
int potionId;
int salveId;
int pillId;

void loadItems()
{
	if(nextItem>0) return;	// Only once.
	printf("Loading items...\n");
	swizzleToVRam=0;
	loadItemImage("enviro/tree2");
	loadItemImage("enviro/tree4");
	loadItemImage("enviro/tree5");
	loadItemImage("enviro/tree2");
	//loadItemImage("enviro/tree6");
	loadItemImage("enviro/grass1");
	loadItemImage("enviro/grass2");
	loadItemImage("enviro/grass3");
	loadItemImage("enviro/grass4");
	loadItemImage("enviro/grass5");
	loadItemImage("enviro/grass6");
	loadItemImage("enviro/grass_scatt");	//11

	int extras[9];
	extras[0]=loadItemImage("buildings/bluffshouse");	// 12 should be white "whitewash"
	extras[1]=loadItemImage("buildings/yellowhut");	// should be yellow "aridhouse"
	extras[4]=loadItemImage("buildings/lavahouse");	// 16 should be red
	extras[2]=loadItemImage("buildings/bluehut");	//loadItemImage("building/lakehouse");	// should be blue
	extras[3]=loadItemImage("buildings/fieldstone");	// should be black
	extras[5]=loadItemImage("buildings/greenhut");// 17	loadItemImage("building/logcabin");	// should be green
	chestId=extras[6]=loadItemImage("chest");
	extras[7]=loadItemImage("box");
	extras[8]=loadItemImage("chestopened");
	
	trophyId[0]=loadItemImage("trophies/FlatsTrophy");
	trophyId[1]=loadItemImage("trophies/SandTrophy");
	trophyId[2]=loadItemImage("trophies/WaterTrophy");
	trophyId[3]=loadItemImage("trophies/StoneTrophy");
	trophyId[4]=loadItemImage("trophies/FireTrophy");
	trophyId[5]=loadItemImage("trophies/FlowerTrophy");
	
	salveId=loadItemImage("items/salve");
	potionId=loadItemImage("items/potion");
	shardId=loadItemImage("items/shardWhite");
	loadItemImage("items/shardYellow");
	loadItemImage("items/shardRed");
	loadItemImage("items/shardBlue");
	loadItemImage("items/shardBlack");
	loadItemImage("items/shardGreen");
	pillId=loadItemImage("items/pill");
	
	printf("$$$$ Loaded extra models: %d,%d,%d,%d,  %d,%d,%d,%d\n",extras[0],extras[1],extras[2],extras[3],extras[4],extras[5],extras[6],extras[7]);

	if(nextItem>0) {
		//initCollision(item[0].vertImage,item[0].polyCount);
	}
#if 0	// subdiv stuff currently crashes
	if(item[0].vertImage && item[0].groupCount>0) {
			// explode out the verts into seperate lists for each texture, then subdiv them, then reassemble...
			int g;
			struct GroupVerts {
				Image *image;
				struct Vertex3DTNP *vertImage;
				int vertCount;
			} *gv=malloc(sizeof(struct GroupVerts)*item[0].groupCount);
			int totalCount=0;
			for(g=0;g<item[0].groupCount;g++) {
				int count=item[0].group[g].last-item[0].group[g].first;
				gv[g].image=item[0].group[g].image;
				gv[g].vertImage=malloc(sizeof(struct Vertex3DTNP)*count);
				gv[g].vertCount=count;
				memcpy(gv[g].vertImage,item[0].vertImage+item[0].group[g].first,sizeof(struct Vertex3DTNP)*count);
				gv[g].vertImage=subdivVertsImage(gv[g].vertImage,&gv[g].vertCount,600);
				totalCount+=gv[g].vertCount;
			}
			// should sort by texture, and merge duplicates
			//qsort(gv,sizeof(struct GroupVerts),item[0].groupCount,cmpGroupVerts);
			// now assemble the result
			free(item[0].vertImage);
			item[0].vertImage=malloc(sizeof(struct Vertex3DTNP)*totalCount);
			if(!item[0].vertImage) {
				printf("Impossible! Subdiv is out of memory.\n");
				return;
			}
			int pos=0;
			for(g=0;g<item[0].groupCount;g++) {
				memcpy(item[0].vertImage+pos,gv[g].vertImage,sizeof(struct Vertex3DTNP)*gv[g].vertCount);
				item[0].group[g].first=pos;
				item[0].group[g].last=pos+gv[g].vertCount;
				free(gv[g].vertImage);
				gv[g].vertImage=0;
			}
			free(gv);
			gv=0;
	}
#endif
	printf("Loading items complete\n");
}

void freeItems()
{
	int i;
	for(i=0;i<nextItem;i++) {
		if(item[i].image) freeImage(item[i].image);
		item[i].image=0;

		qsort(item[i].group,item[i].groupCount,sizeof(struct MaterialGroup),cmpMaterialGroupImage);
		Image *last=0;
		int j;
		for(j=0;j<item[i].groupCount;j++) {
			struct MaterialGroup *g=item[i].group+j;
			if(g->image!=0 && g->image!=last) {
				freeImage(g->image);
				last=g->image;
				g->image=0;
			}
		}

		if(item[i].group) free(item[i].group);
		item[i].group=0;
		if(item[i].vertImage) free(item[i].vertImage);
		item[i].vert=0;
		if(item[i].vert) free(item[i].vert);
		item[i].vert=0;
		if(item[i].poly) free(item[i].poly);
		item[i].poly=0;
	}
	nextItem=0;
#if 0
	for(i=0;i<nextMaterial;i++) {
		if(material[i].image) {
			freeImage(material[i].image);
			material[i].image=0;
			material[i].name[0]=0;
		}
	}
	nextMaterial=0;
#endif
}

int getItemCount()
{
	return nextItem;
}

void renderItem(int i)
{
	if(i>=nextItem || i<0) {
		printf("Item %d is out of range of %d items.\n",i,nextItem);
		return;
	}
//	if(i==17||i==18) printf("item %d: %.2f,%.2f,%.2f\n",i,item[i].matrix[12],item[i].matrix[13],item[i].matrix[14]);
#ifdef _PSP
	//printf("Rendering item %d\n",i);
#if 0
	if(item[i].image) {
		Image *source=item[i].image;
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);
		sceGuTexScale(1.0f,1.0f);
		sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
		//sceGuEnable(GU_LIGHTING);
		sceGuDisable(GU_LIGHTING);
//		if(i==17||i==18) printf("showing image %08x\n",(int)source);
	}
	//printf("Drawing item %d\n",i);
	sceGumMatrixMode(GU_MODEL);
	sceGumPushMatrix();
	sceGumMultMatrix((ScePspFMatrix4 *)item[i].matrix);
	int j=0;
	if(item[i].image) {
		sceGuEnable(GU_TEXTURE_2D);
		sceGuColor(0xffffffff);
	}
	sceGuFrontFace(GU_CCW);
	while(j<item[i].polyCount) {
		int count=30720;
		if(j+count>item[i].polyCount) count=item[i].polyCount-j;
		//printf("Drawing %d polys\n",count);
		//sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_COLOR_8888|GU_INDEX_16BIT|GU_TRANSFORM_3D,count,item[i].poly+j,item[i].vert);
		if(item[i].vert)
			sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_COLOR_8888|GU_TRANSFORM_3D,count,0,item[i].vert+j);
		if(item[i].vertImage) {
			sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,count,0,item[i].vertImage+j);
		}
		j=j+count;
	}
	sceGuFrontFace(GU_CW);
	if(item[i].image) {
		sceGuDisable(GU_LIGHTING);
		sceGuTexScale(1,1);
	}
	sceGumPopMatrix();
#else
	sceGumMatrixMode(GU_MODEL);
	sceGumPushMatrix();
	sceGumMultMatrix((ScePspFMatrix4 *)item[i].matrix);
	int j=0;
	int g;
	int jCount;
	//printf("item image: %08x\n",(int)item[i].image);
	if(item[i].image) {
		sceGuEnable(GU_TEXTURE_2D);
		sceGuColor(0xffffffff);
	}
	sceGuFrontFace(GU_CCW);
	//sceGuFrontFace(GU_CW);
	if(item[i].image) {
		sceGuTexScale(1.0f,1.0f);
		sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
//		sceGuEnable(GU_LIGHTING);
		sceGuDisable(GU_LIGHTING);
	}
	for(g=0;g<item[i].groupCount;g++) {
		if(item[i].group[g].image) {
			Image *source=item[i].group[g].image;
			sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
			sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);
//			if(i==17||i==18) printf("showing image %08x\n",(int)source);
		}
		j=item[i].group[g].first;
		jCount=item[i].group[g].last;
		if(j>=jCount) printf("item %d: skipping group %d of %d, vert %d-%d\n",i,g,item[i].groupCount,j,jCount);
		while(j<jCount) {
			int count=30720;
			if(j+count>jCount) count=jCount-j;
			if(count<=0) { printf("lost my place.\n"); continue; }
//			if(i==17||i==18) printf("Drawing %d polys\n",count);
			//sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_COLOR_8888|GU_INDEX_16BIT|GU_TRANSFORM_3D,count,item[i].poly+j,item[i].vert);
//			if(i==17||i==18) printf("drawing group %d, vert %d x%d\n",g,j,count);
#if 1
			if(item[i].vert)
				sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_COLOR_8888|GU_TRANSFORM_3D,count,0,item[i].vert+j);
			if(item[i].vertImage) {
				sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,count,0,item[i].vertImage+j);
			}
#endif

			j=j+count;
		}
	}
	sceGuFrontFace(GU_CW);
	if(item[i].image) {
		sceGuDisable(GU_LIGHTING);
		sceGuTexScale(1,1);
	}
	sceGumPopMatrix();
#endif
#endif
}

void setItemPos(int i,float x,float y,float z)
{
	if(i<0 || i>=nextItem) return;
	item[i].matrix[12]=x;
	item[i].matrix[13]=y;
	item[i].matrix[14]=z;
	item[i].matrix[0]=1; //6;
	item[i].matrix[5]=1; //6;
	item[i].matrix[10]=1; //6;
}

float *getItemMin(int i)
{
	if(i<0 || i>nextItem) return 0;
	return item[i].min;
}

float *getItemMax(int i)
{
	if(i<0 || i>nextItem) return 0;
	return item[i].max;
}
