#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>

#include "ase.h"
#ifndef _PSP
#define GU_COLOR(r,g,b,a) (((int)((r)*255))<<24|((int)((g)*255))<<16|((int)((b)*255))<<8|255)
#endif

struct AseFace {
    int vert[3];
};

struct AseObjectParse {
	ScePspFVector3 *vert;
	int vertCount;
	struct AseFace *face;
	int faceCount;
	ScePspFVector3 *tvert;
	int tvertCount;
	struct AseFace *tface;
	int tfaceCount;
	ScePspFVector3 *faceNormal;	// faceCount of them
	ScePspFVector3 *vertNormal;	// vertCount of them
};

/** Takes a euler normal + angle in radians, and makes a nice rotation matrix
 * \param rot the normal in x,y,z with the angle in radians in w.
 * \param matrix fills in the 16 elementx of the 4x4 matrix
 */
void eulerToMatrix(ScePspFVector4 *rot,float *matrix)
{
    float c=cosf(rot->w);
    float s=sinf(rot->w);
    float t=1-c;
    float x=rot->x,y=rot->y,z=rot->z;
#if 1
    matrix[0]=t*x*x+c;      // left hand rotation
    matrix[1]=t*x*y-s*z;
    matrix[2]=t*x*z+s*y;
    matrix[3]=0;

    matrix[4]=t*x*y+s*z;
    matrix[5]=t*y*y+c;
    matrix[6]=t*y*z-s*x;
    matrix[7]=0;

    matrix[8]=t*x*z-s*y;
    matrix[9]=t*y*z+s*x;
    matrix[10]=t*z*z+c;
    matrix[11]=0;

    matrix[12]=0;
    matrix[13]=0;
    matrix[14]=0;
    matrix[15]=1;
#else
    matrix[0]=t*x*x+c;      // right hand rotation
    matrix[1]=t*x*y+s*z;
    matrix[2]=t*x*z-s*y;
    matrix[3]=0;

    matrix[4]=t*x*y-s*z;
    matrix[5]=t*y*y+c;
    matrix[6]=t*y*z+s*x;
    matrix[7]=0;

    matrix[8]=t*x*z+s*y;
    matrix[9]=t*y*z-s*x;
    matrix[10]=t*z*z+c;
    matrix[11]=0;

    matrix[12]=0;
    matrix[13]=0;
    matrix[14]=0;
    matrix[15]=1;
#endif
}

void parseAseScene(struct AseScene *scene,char *line,char *command,int depth)
{
    char cmd[256];
	if(strcmp(command,"*SCENE_FIRSTFRAME")==0) sscanf(line,"%s %d",cmd,&scene->firstFrame);
	else if(strcmp(command,"*SCENE_LASTFRAME")==0) sscanf(line,"%s %d",cmd,&scene->lastFrame);
	else if(strcmp(command,"*SCENE_FRAMESPEED")==0) sscanf(line,"%s %d",cmd,&scene->fps);
	else if(strcmp(command,"*SCENE_TICKSPERFRAME")==0) sscanf(line,"%s %d",cmd,&scene->ticksPerFrame);
}

void parseAseMaterial(struct AseScene *scene,struct AseMaterial *material,char *line,char *command,int depth)
{
    char cmd[256];
	if(strcmp(command,"*MATERIAL_DIFFUSE")==0) {
		float r=0,g=0,b=0;
		sscanf(line,"%s %f%f%f",cmd,&r,&g,&b);
		material->color=GU_COLOR(r,g,b,1.0f);
	} else if(strcmp(command,"*MATERIAL_NAME")==0) {
		sscanf(line,"%s \"%s\"",cmd,material->name);
	} else if(strcmp(command,"*BITMAP")==0) {
		char buf[256]="";
		//sscanf(line,"%s \"%s\"",cmd,buf);
		char *base=strchr(line,'"');    // leading "
		if(base) {
		    strcpy(buf,base+1);
		}
		base=buf;
		if(strrchr(buf,'"')) strrchr(buf,'"')[0]=0; // trailing "
		if(strrchr(base,'\\')) { base=strrchr(base,'\\'); base++; }
		char path[256];
		sprintf(path,"models/%s/%s",scene->name,base);
		material->image=loadPng(path);
		if(!material->image) {
			printf("Missing image '%s' from file models/%s.ase\n",path,scene->name);
		} else {
#ifdef _PSP
			swizzleFast(material->image);
#endif
		}
	}
}

void parseAseGeomObject(struct AseScene *scene,struct AseObject *object,struct AseObjectParse *parse,char *line,char *command,int depth)
{
    char cmd[256];
	float x=0,y=0,z=0;
	if(strcmp(command,"*NODE_NAME")==0) {
		sscanf(line,"%s \"%s\"",cmd,object->name);
	} else if(strcmp(command,"*TM_ROW0")==0) {
		int count=sscanf(line,"%s %f%f%f",cmd,&x,&y,&z);
		if(count==4) {
			object->matrix[0]=x;
			object->matrix[1]=y;
			object->matrix[2]=z;
		}
	} else if(strcmp(command,"*TM_ROW1")==0) {
		int count=sscanf(line,"%s %f%f%f",cmd,&x,&y,&z);
		if(count==4) {
			object->matrix[4]=x;
			object->matrix[5]=y;
			object->matrix[6]=z;
		}
	} else if(strcmp(command,"*TM_ROW2")==0) {
		int count=sscanf(line,"%s %f%f%f",cmd,&x,&y,&z);
		if(count==4) {
			object->matrix[8]=x;
			object->matrix[9]=y;
			object->matrix[10]=z;
		}
	} else if(strcmp(command,"*TM_ROW3")==0) {
		int count=sscanf(line,"%s %f%f%f",cmd,&x,&y,&z);
		if(count==4) {
			object->matrix[12]=x;
			object->matrix[13]=y;
			object->matrix[14]=z;
			object->matrix[15]=1;
		}
/*	} else if(strcmp(command,"*TM_SCALE")==0) {
		int count=sscanf(line,"%s %f%f%f",cmd,&x,&y,&z);
		if(count==4) {
			object->matrix[0]*=x;
			object->matrix[1]*=y;
			object->matrix[2]*=z;
			object->matrix[4]*=x;
			object->matrix[5]*=y;
			object->matrix[6]*=z;
			object->matrix[8]*=x;
			object->matrix[9]*=y;
			object->matrix[10]*=z;
			object->matrix[12]*=x;
			object->matrix[13]*=y;
			object->matrix[14]*=z;
		}
*/
	} else if(strcmp(command,"*MESH_NUMVERTEX")==0) {
		int count=0;
		sscanf(line,"%s %d",cmd,&count);
		//printf("Got %d verts\n",count);
		parse->vertCount=count;
		parse->vert=(ScePspFVector3 *)calloc(sizeof(ScePspFVector3),count);
		parse->vertNormal=(ScePspFVector3 *)calloc(sizeof(ScePspFVector3),count);
	} else if(strcmp(command,"*MESH_NUMFACES")==0) {
		int count=0;
		sscanf(line,"%s %d",cmd,&count);
		//printf("Got %d faces\n",count);
		parse->faceCount=count;
		parse->face=(struct AseFace *)calloc(sizeof(struct AseFace),count);
		parse->faceNormal=(ScePspFVector3 *)calloc(sizeof(ScePspFVector3),count);
	} else if(strcmp(command,"*MESH_NUMTVERTEX")==0) {
		int count=0;
		sscanf(line,"%s %d",cmd,&count);
		//printf("Got %d tverts\n",count);
		parse->tvertCount=count;
		parse->tvert=(ScePspFVector3 *)calloc(sizeof(ScePspFVector3),count);
	} else if(strcmp(command,"*MESH_NUMTVFACES")==0) {
		int count=0;
		sscanf(line,"%s %d",cmd,&count);
		//printf("Got %d tfaces\n",count);
		parse->tfaceCount=count;
		parse->tface=(struct AseFace *)calloc(sizeof(struct AseFace),count);
	} else if(strcmp(command,"*MESH_VERTEX")==0) {
		int i=0;
		int count=sscanf(line,"%s %d %f%f%f",cmd,&i,&x,&y,&z);
		if(count==5 && i>=0 && i<parse->vertCount) {
			parse->vert[i].x=x;
			parse->vert[i].y=y;
			parse->vert[i].z=z;
		}
	} else if(strcmp(command,"*MESH_FACE")==0) {
		int i=0;
		int face[3];
		int count=sscanf(line,"%s %d: A: %d B: %d C: %d",cmd,&i,&face[0],&face[1],&face[2]);
		if(count==5 && i>=0 && i<parse->faceCount) {
			parse->face[i].vert[0]=face[0];
			parse->face[i].vert[1]=face[1];
			parse->face[i].vert[2]=face[2];
		} else {
            printf("Couldn't parse line '%s'\n",line);
		}
	} else if(strcmp(command,"*MESH_TVERT")==0) {
		int i=0;
		int count=sscanf(line,"%s %d %f%f%f",cmd,&i,&x,&y,&z);
		if(count==5 && i>=0 && i<parse->tvertCount) {
			parse->tvert[i].x=x;
			parse->tvert[i].y=y;
			parse->tvert[i].z=z;
		}
	} else if(strcmp(command,"*MESH_TFACE")==0) {
		int i=0;
		int face[3];
		int count=sscanf(line,"%s %d %d%d%d",cmd,&i,&face[0],&face[1],&face[2]);
		if(count==5 && i>=0 && i<parse->tfaceCount) {
			parse->tface[i].vert[0]=face[0];
			parse->tface[i].vert[1]=face[1];
			parse->tface[i].vert[2]=face[2];
		}
	} else if(strcmp(command,"*MESH_FACENORMAL")==0) {
		int i=0;
		int count=sscanf(line,"%s %d %f%f%f",cmd,&i,&x,&y,&z);
		if(count==5 && i>=0 && i<parse->faceCount) {
			parse->faceNormal[i].x=x;
			parse->faceNormal[i].y=y;
			parse->faceNormal[i].z=z;
		}
	} else if(strcmp(command,"*MESH_VERTEXNORMAL")==0) {
		int i=0;
		int count=sscanf(line,"%s %d %f%f%f",cmd,&i,&x,&y,&z);
		if(count==5 && i>=0 && i<parse->vertCount) {
			parse->vertNormal[i].x=x;
			parse->vertNormal[i].y=y;
			parse->vertNormal[i].z=z;
		}
	} else if(strcmp(command,"*CONTROL_POS_SAMPLE")==0) {
		int i=0;
		int ticks=0;
		int count=sscanf(line,"%s %d %f%f%f",cmd,&ticks,&x,&y,&z);
		if(count==5) {
		    int item=object->anim.nodeCount;    // assume that the next one will be added to the end.
		    if(object->anim.node==0) {
		        object->anim.node=(struct AseAnimationNode *)malloc(sizeof(struct AseAnimationNode));
		        object->anim.nodeCount=1;
                object->anim.node[item].flags=0;
                object->anim.node[item].rot.x=0;
                object->anim.node[item].rot.y=0;
                object->anim.node[item].rot.z=0;
                object->anim.node[item].rot.w=0;
		    } else {
		        // search for a match.
		        for(i=0;i<object->anim.nodeCount;i++) {
		            if(object->anim.node[i].ticks==ticks) {
		                item=i;
		                break;
		            }
		        }
		        if(item==object->anim.nodeCount) {  // Make it bigger if no match.
                    object->anim.nodeCount++;
                    object->anim.node=(struct AseAnimationNode *)realloc(object->anim.node,sizeof(struct AseAnimationNode)*object->anim.nodeCount);
                    object->anim.node[item].flags=0;
                    object->anim.node[item].rot.x=0;
                    object->anim.node[item].rot.y=0;
                    object->anim.node[item].rot.z=0;
                    object->anim.node[item].rot.w=0;
		        }
		    }
		    //printf("node %d is @%d; pos %.2f,%.2f,%.2f\n",item,ticks,x,y,z);
			object->anim.node[item].flags|=AAN_POS;
			object->anim.node[item].ticks=ticks;
			object->anim.node[item].pos.x=x;
			object->anim.node[item].pos.y=y;
			object->anim.node[item].pos.z=z;
		}
	} else if(strcmp(command,"*CONTROL_ROT_SAMPLE")==0) {
		int i=0;
		int ticks=0;
		float w=0;
		int count=sscanf(line,"%s %d %f%f%f%f",cmd,&ticks,&x,&y,&z,&w);
		if(count==6) {
            float mag=sqrtf(x*x+y*y+z*z);
            if(mag!=0) {
                x/=mag; // eliminate any rounding errors.
                y/=mag;
                z/=mag;
            }
		    int item=object->anim.nodeCount;    // assume that the next one will be added to the end.
		    if(object->anim.node==0) {
		        object->anim.node=(struct AseAnimationNode *)malloc(sizeof(struct AseAnimationNode));
		        object->anim.nodeCount=1;
                object->anim.node[item].flags=0;
                object->anim.node[item].pos.x=0;
                object->anim.node[item].pos.y=0;
                object->anim.node[item].pos.z=0;
		    } else {
		        // search for a match.
		        for(i=0;i<object->anim.nodeCount;i++) {
		            if(object->anim.node[i].ticks==ticks) {
		                item=i;
		                break;
		            }
		        }
		        if(item==object->anim.nodeCount) {  // Make it bigger if no match.
                    object->anim.nodeCount++;
                    object->anim.node=(struct AseAnimationNode *)realloc(object->anim.node,sizeof(struct AseAnimationNode)*object->anim.nodeCount);
                    object->anim.node[item].flags=0;
                    object->anim.node[item].pos.x=0;
                    object->anim.node[item].pos.y=0;
                    object->anim.node[item].pos.z=0;
		        }
		    }
		    //printf("node %d is @%d; rot %.2f,%.2f,%.2f,%.2f\n",item,ticks,w,x,y,z);
			object->anim.node[item].flags|=AAN_ROT;
			object->anim.node[item].ticks=ticks;
			object->anim.node[item].rot.w=w;
			object->anim.node[item].rot.x=x;
			object->anim.node[item].rot.y=y;
			object->anim.node[item].rot.z=z;
		}
    } else if( strcmp(command,"*MATERIAL_REF")==0) {
        int ref=0;
   		int count=sscanf(line,"%s %d",cmd,&ref);
        if(count>=1) object->image=scene->material[ref].image;
	}
}

int cmpAseAnimationNode(const void *one,const void *two)
{
    const struct AseAnimationNode *a=(struct AseAnimationNode *)one,*b=(struct AseAnimationNode *)two;

    return a->ticks<b->ticks?-1:a->ticks>b->ticks?1:0;
}

void readXmlTag(FILE *file,char *tag,int length)
{
	int i=0,ch=0;		// grab a tag.
	tag[0]=0;
	while( (ch=fgetc(file))!=EOF && i<length-2) {
		tag[i]=ch;
		tag[i+1]=0;
		if(ch=='>') break;
		i++;
	}
	//printf("Found %d long tag: '%s'\n",i,tag);
}

void loadAxfAnimation(const char *fname,struct AseObject *object)
{
	char pattern[256];	// what we are looking for
	char tag[256];		// last tag we've read in.
	int id=0;
	FILE *file=fopen(fname,"r");
	if(!fname) return;
	sprintf(pattern,"<Node name=\"%s",object->name);
	// found the node that interests us.
	while(!feof(file)) {
		readXmlTag(file,tag,256);
		if(strstr(tag,pattern)) break;
	}
	printf("pattern: %s; tag: %s\n",pattern,tag);
	while(!feof(file)) {
		readXmlTag(file,tag,256);
		if(strstr(tag,"</Samples>")) break;
		if(strstr(tag,"<Samples")!=NULL) {
			char *attr=strchr(tag,'"');
			if(attr) {
				object->anim.nodeCount=atoi(attr+1);
				printf("sample count is %d; \n",object->anim.nodeCount);
				object->anim.node=(struct AseAnimationNode *)calloc(object->anim.nodeCount,sizeof(struct AseAnimationNode));
			}
		}
		if(strstr(tag,"<S ")==NULL) continue;
		if(id<object->anim.nodeCount) {
			// Handle a sample, since we have space set aside for it.
			struct AseAnimationNode *node=object->anim.node+id;
			node->flags=0;
			char *attr=tag;
			while((attr=strchr(attr,'"'))!=0) {
				attr++;
				if(attr[-3]=='t') {
					node->ticks=atoi(attr);
					//printf("ticks=%d; ",node->ticks);
					attr=strchr(attr,'"');
					if(!attr) break;
					attr++;
				} else if(attr[-3]=='v') {
					float m[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
					int count=sscanf(attr,"%f%f%f %f%f%f %f%f%f %f%f%f",
						m+0,m+1,m+2,
						m+4,m+5,m+6,
						m+8,m+9,m+10,
						m+12,m+13,m+14);
					//printf("scanning sample matrix attr; count=%d\n",count);
					if(count==12) {
						// save for rendering.
						node->flags=AAN_MAT;
						node->mat.x.x=m[0];
						node->mat.x.y=m[1];
						node->mat.x.z=m[2];
						node->mat.x.w=m[3];
						node->mat.y.x=m[4];
						node->mat.y.y=m[5];
						node->mat.y.z=m[6];
						node->mat.y.w=m[7];
						node->mat.z.x=m[8];
						node->mat.z.y=m[9];
						node->mat.z.z=m[10];
						node->mat.z.w=m[11];
						node->mat.w.x=0;//m[12];
						node->mat.w.y=0;//m[13];
						node->mat.w.z=0;//m[14];
						node->mat.w.w=m[15];
						node->flags|=AAN_POS;
						node->pos.x=m[12];
						node->pos.y=m[13];
						node->pos.z=m[14];
					}
					attr=strchr(attr,'"');
					if(!attr) break;
					attr++;
				}
			}
			id++;
		}
	}

	fclose(file);
}

void finalizeObject(struct AseScene *scene,struct AseObject *object,struct AseObjectParse *parse)
{
	object->vertCount=parse->faceCount*3;
	object->vertImage=(struct Vertex3DTNP *)malloc(parse->faceCount*3*sizeof(struct Vertex3DTNP));
	int i;
	//printf("Generating %d faces in object\n",parse->faceCount);
	for(i=0;i<parse->faceCount;i++) {
		int j;
		for(j=0;j<3;j++) {
			struct Vertex3DTNP *v=object->vertImage+i*3+j;
			v->u=parse->tvert[parse->tface[i].vert[j]].x;
			v->v=1.0f-parse->tvert[parse->tface[i].vert[j]].y;
			v->nx=parse->vertNormal[parse->face[i].vert[j]].x;
			v->ny=parse->vertNormal[parse->face[i].vert[j]].y;
			v->nz=parse->vertNormal[parse->face[i].vert[j]].z;
			v->x=parse->vert[parse->face[i].vert[j]].x;
			v->y=parse->vert[parse->face[i].vert[j]].y;
			v->z=parse->vert[parse->face[i].vert[j]].z;
			//printf("uv:%.2f,%.2f; n:%.2f,%.2f,%.2f; p:%.2f,%.2f,%.2f\n",v->u,v->v,v->nx,v->ny,v->nz,v->x,v->y,v->z);
		}
	}
	if(parse->vert) free(parse->vert);
	parse->vert=0;
	parse->vertCount=0;
	if(parse->face) free(parse->face);
	parse->face=0;
	if(parse->tvert) free(parse->tvert);
	parse->tvert=0;
	parse->tvertCount=0;
	if(parse->tface) free(parse->tface);
	parse->tface=0;
	parse->tfaceCount=0;
	if(parse->vertNormal) free(parse->vertNormal);
	parse->vertNormal=0;
	if(parse->faceNormal) free(parse->faceNormal);
	parse->faceNormal=0;
	memset(parse,0,sizeof(struct AseObjectParse));
	qsort(object->anim.node,object->anim.nodeCount,sizeof(struct AseAnimationNode),cmpAseAnimationNode);
	for(i=0;i<object->anim.nodeCount;i++) {
	    //printf("node %d: ticks=%d, flags=%d\n",i,object->anim.node[i].ticks,object->anim.node[i].flags);
	}
#ifdef _PSP
	gumFullInverse((ScePspFMatrix4 *)object->unmatrix,(ScePspFMatrix4 *)object->matrix);
#else
	// Almost an inversion of the matrix...
	int j;
	for(j=0;j<16;j++) object->unmatrix[j]=object->matrix[(j%4)*4+(j/4)];
	object->unmatrix[12]=-object->unmatrix[3];
	object->unmatrix[3]=0;
	object->unmatrix[13]=-object->unmatrix[7];
	object->unmatrix[7]=0;
	object->unmatrix[14]=-object->unmatrix[11];
	object->unmatrix[11]=0;
#endif
#if USE_XAFANIMATION
	// Now try the XAF animation information instead.
	if(object->anim.node) free(object->anim.node);
	object->anim.node=0;
	object->anim.nodeCount=0;
#endif
}

static int cmpInt(const void *one,const void *two)
{
	const int *a=(int *)one,*b=(int *)two;
	return *a<*b?-1:*a>*b?1:0;
}

/** Loads an AseScene, and all image assets that go with it.
 * \param fname the filename to load
 * \returns the scene with all images loaded or NULL on failure
 */
struct AseScene *loadAseScene(const char *fname)
{
	char path[256];
	sprintf(path,"models/%s.ase",fname);
	FILE *file=fopen(path,"r");
	printf("Loading '%s': 0x%08x\n",path,(int)file);
	if(!file) return NULL;
	struct AseScene *scene=(struct AseScene *)calloc(sizeof(struct AseScene),1);
	strcpy(scene->name,fname);

	enum ParseContainer { PC_TOP, PC_SCENE, PC_MATERIAL_LIST, PC_GEOMOBJECT} state=PC_TOP;
	char line[256];
	char command[256];
	int depth=0;
	struct AseMaterial *material=0;
	struct AseObject *object=0;
	struct AseObjectParse parseObject;
	memset(&parseObject,0,sizeof(struct AseObjectParse));
	while(fgets(line,255,file)) {
		line[255]=0;
		if(strrchr(line,'\r')) strrchr(line,'\r')[0]=0;
		if(strrchr(line,'\n')) strrchr(line,'\n')[0]=0;
		sscanf(line,"%s",command);
		if(strchr(line,'{')) {
			depth++;
			if(depth==1) {
				if(strcmp(command,"*SCENE")==0) {
					state=PC_SCENE;
					continue;
				}
				if(strcmp(command,"*MATERIAL_LIST")==0) {
					state=PC_MATERIAL_LIST;
					continue;
				}
				if(strcmp(command,"*GEOMOBJECT")==0) {
					state=PC_GEOMOBJECT;
					if(!scene->object) {
						object=(struct AseObject *)calloc(sizeof(struct AseObject),1);
						scene->object=object;
						scene->objectCount=1;
					} else {
						object=(struct AseObject *)realloc(scene->object,sizeof(struct AseObject)*(scene->objectCount+1));
						scene->object=object;
						object+=scene->objectCount;
						scene->objectCount++;
						memset(object,0,sizeof(struct AseObject));
					}
					continue;
				}
				// unknown container.
			}
		}
		if(strchr(line,'}')) {
			depth--;
			if(depth==0 && state==PC_GEOMOBJECT) {
				finalizeObject(scene,object,&parseObject);
#ifdef USE_XAFANIMATION
				char xafname[256];
				sprintf(xafname,"models/%s.xaf",fname);
				loadAxfAnimation(xafname,object);
#endif
				// now we look for the animation keyframes to keep a global list so the we can interpolate correctly.
				int i;
				for(i=0;i<object->anim.nodeCount;i++) {
					struct AseAnimationNode *node=object->anim.node+i;
					//printf("Searching for keyframe at %d ticks\n",node->ticks);
					void *found=0;
					if(scene->keyframe) bsearch(&node->ticks,scene->keyframe,scene->keyframeCount,sizeof(int),cmpInt);
					if(found) {
						//printf("Found keyframe at %d ticks\n",node->ticks);
						continue;
					}
					//printf("Not found. Adding keyframe at %d ticks\n",node->ticks);
					// need to add it to the list.
					if(!scene->keyframe) {
						scene->keyframe=(int *)malloc(sizeof(int));
						scene->keyframeCount=1;
					} else {
						scene->keyframe=(int *)realloc(scene->keyframe,sizeof(int)*(scene->keyframeCount+1));
						scene->keyframeCount++;
					}
					scene->keyframe[scene->keyframeCount-1]=node->ticks;
					qsort(scene->keyframe,scene->keyframeCount,sizeof(int),cmpInt);	// would be better to do an insertion sort...
				}
			}
			if(depth==0) state=PC_TOP;
		}
		if(depth>=1) {
		    char cmd[256];
			switch(state) {
			case PC_SCENE:
				parseAseScene(scene,line,command,depth);
				break;
			case PC_MATERIAL_LIST:
				if(depth==1) material=0;
				if(depth==1 && strcmp(command,"*MATERIAL_COUNT")==0) {
					sscanf(line,"%s %d",cmd,&scene->materialCount);
					if(scene->materialCount<=0) {
						printf("Error: material_count=%d too small.\n",scene->materialCount);
						scene->materialCount=1;
					}
					if(scene->materialCount>255) {
						printf("Error: material_count %d too big.\n",scene->materialCount);
						scene->materialCount=256;
					}
					scene->material=(struct AseMaterial *)calloc(sizeof(struct AseMaterial),scene->materialCount);
				}
				if(depth==2 && material==0 && strcmp(command,"*MATERIAL")==0) {
					int i;
					sscanf(line,"%s %d",cmd,&i);
					if(i<0 || i>=scene->materialCount) {
						material=scene->material;
						printf("Out of range material (%d)!\n",i);
					} else {
						material=scene->material+i;
					}
					continue;
				}
				if(depth>=2 && material) {
					parseAseMaterial(scene,material,line,command,depth);
				}
				break;
			case PC_GEOMOBJECT:
				parseAseGeomObject(scene,object,&parseObject,line,command,depth);
				break;
            default:
                break;
			}
		}
		// ignore all other content.
	}
	fclose(file);

	return scene;
}

static void freeAseObject(struct AseObject *object)
{
    if(!object) return;
    object->image=0;    // master copy owned by the MaterialList in the scene
    if(object->vert) free(object->vert);
    object->vert=0;
    if(object->vertImage) free(object->vertImage);
    object->vertImage=0;
    if(object->anim.node) free(object->anim.node);
    object->anim.node=0;
}

/** Frees a previously loaded scene and all associated images
 * \param scene the scene to deallocate
 */
void freeAseScene(struct AseScene *scene)
{
    int i;
    for(i=0;i<scene->objectCount;i++) {
        freeAseObject(&scene->object[i]);
    }
    free(scene->object);
    scene->object=0;
    for(i=0;i<scene->materialCount;i++) {
#ifdef _PSP
        if(scene->material[i].image) freeImage(scene->material[i].image);
#endif
        scene->material[i].image=0;
    }
    free(scene->material);
}

/** updates the animation for the scene, to be ready to draw
 * \param scene the scene to update
 * \param elapsed the number of 1/1000ths of a second that elapsed since the last update/reset
 * \param maxFrame the terminus for the animation. once that frame is reached the animation stops
 * \returns 1 if maxFrame is reached, or 0 otherwise.
 */
int updateAseSceneMax(struct AseScene *scene,int elapsed,int maxFrame)
{
    scene->ticks+=elapsed*scene->ticksPerFrame*scene->fps/1000;
    int ticks=scene->ticksPerFrame*maxFrame;
    if(scene->ticks>ticks) {
        scene->ticks=ticks;
        return 1;
    }
    //printf("At frame %.04f (%d less than %d) maxframe: %d\n",scene->ticks/(float)scene->ticksPerFrame,scene->ticks,ticks,maxFrame);
    return 0;
}

/** updates the animation for the scene, to be ready to draw from a specific frame of the animation
 * \param scene the scene to update
 * \param frame the new frame to reset to
 */
void resetAseScene(struct AseScene *scene,int frame)
{
    scene->ticks=frame*scene->ticksPerFrame;
}

#ifdef _PSP
static void drawAseObject(struct AseObject *object,int ticks,int *keyframe,int keyframeCount)
{
    sceGumMatrixMode(GU_MODEL);
    sceGumPushMatrix();

    // now do the incremental translations
    ScePspFMatrix4 rotation;
    gumLoadIdentity(&rotation);
    ScePspFVector3 offset={0,0,0};
    int i;
    int oldTicks=0;
    for(i=0;i<object->anim.nodeCount;i++) {
        struct AseAnimationNode *node=object->anim.node+i;
        if(node->ticks<=ticks) {
            if(node->flags&AAN_POS) {
                offset.x=node->pos.x;
                offset.y=node->pos.y;
                offset.z=node->pos.z;
            }
            if(node->flags&AAN_ROT) {
                ScePspFMatrix4 rotAngle;
                eulerToMatrix(&node->rot,(float *)&rotAngle);
                gumMultMatrix(&rotation,&rotAngle,&rotation);
/* if(strcmp(object->name,"tom4\"")==0) {
                printf("<%.2f,%.2f,%.2f> by %.4f radians;\n",node->rot.x,node->rot.y,node->rot.z,node->rot.w);
                printf("M=[%.4f,%.4f,%.4f,%.4f]",rotAngle.x.x,rotAngle.x.y,rotAngle.x.z,rotAngle.x.w);
                printf("[%.4f,%.4f,%.4f,%.4f]",rotAngle.y.x,rotAngle.y.y,rotAngle.y.z,rotAngle.y.w);
                printf("[%.4f,%.4f,%.4f,%.4f]",rotAngle.z.x,rotAngle.z.y,rotAngle.z.z,rotAngle.z.w);
                printf("[%.4f,%.4f,%.4f,%.4f]\n",rotAngle.w.x,rotAngle.w.y,rotAngle.w.z,rotAngle.w.w);
} */
            }
            if(node->flags&AAN_MAT) {
				rotation=node->mat;
				offset.x=node->mat.w.x;
				offset.y=node->mat.w.y;
				offset.z=node->mat.w.z;
			}
            oldTicks=node->ticks;
//if(strcmp(object->name,"tom4\"")==0) printf("%d: pos=%.2f,%.2f,%.2f; <%.2f,%.2f,%.2f> th=%.4f\n",i,offset.x,offset.y,offset.z,node->rot.x,node->rot.y,node->rot.z,node->rot.w);
        } else {
#define ENABLE_INTERPOLATION
#ifdef ENABLE_INTERPOLATION
			if(node->flags&AAN_MAT) break;
			int *keyframeNode=(int *)bsearch(&node->ticks,keyframe,keyframeCount,sizeof(int),cmpInt);
//if(strcmp(object->name,"tom4\"")==0) printf("ticks @ %d; found keyframe 0x%08x\n",ticks,(int)keyframeNode);
			if(!keyframeNode) break;	// ASSERT: keyframeNode can't be NULL...
//if(strcmp(object->name,"tom4\"")==0) printf("keyframeNode at %d\n",keyframeNode[0]);
			int keyframePos=((int)keyframeNode-(int)keyframe)/sizeof(int);
//if(strcmp(object->name,"tom4\"")==0) printf("keyframe position %d of %d\n",keyframePos,keyframeCount);
			if(keyframePos==0) break;	// ASSERT: keyframePos can't be 0...
			if(keyframe[keyframePos-1]>ticks) break;	// nothing is happening here.
//if(strcmp(object->name,"tom4\"")==0) printf("shifting old ticks %d to new ticks %d.\n",oldTicks,keyframe[keyframePos-1]);
			oldTicks=keyframe[keyframePos-1];
            float t=(ticks-oldTicks)/(float)(node->ticks-oldTicks);
//if(strcmp(object->name,"tom4\"")==0) printf("interpolation t=%.6f\n",t);
            if(node->flags&AAN_POS) {
                offset.x=node->pos.x*t+offset.x*(1-t);
                offset.y=node->pos.y*t+offset.y*(1-t);
                offset.z=node->pos.z*t+offset.z*(1-t);
            }
            if(node->flags&AAN_ROT) {
                ScePspFVector4 rotPartial=node->rot;
                rotPartial.w*=t;
                ScePspFMatrix4 rotAngle;
                eulerToMatrix(&rotPartial,(float *)&rotAngle);
                gumMultMatrix(&rotation,&rotAngle,&rotation);
/* if(strcmp(object->name,"tom4\"")==0) {
				printf("<%.2f,%.2f,%.2f> by %.4f*t=%.4f radians;\n",node->rot.x,node->rot.y,node->rot.z,node->rot.w,rotPartial.w);
				printf("M=[%.4f,%.4f,%.4f,%.4f]\n",rotAngle.x.x,rotAngle.x.y,rotAngle.x.z,rotAngle.x.w);
				printf("  [%.4f,%.4f,%.4f,%.4f]\n",rotAngle.y.x,rotAngle.y.y,rotAngle.y.z,rotAngle.y.w);
				printf("  [%.4f,%.4f,%.4f,%.4f]\n",rotAngle.z.x,rotAngle.z.y,rotAngle.z.z,rotAngle.z.w);
				printf("  [%.4f,%.4f,%.4f,%.4f]\n",rotAngle.w.x,rotAngle.w.y,rotAngle.w.z,rotAngle.w.w);
} */
            }
if(strcmp(object->name,"tom4\"")==0) printf("%d: t=%.4f pos=%.2f,%.2f,%.2f; rot=%.4f\n",i,t,offset.x,offset.y,offset.z,node->rot.w);
#endif
            break;
        }
    }
    rotation.w.x=0;	// translate back after rotation.
    rotation.w.y=0;
    rotation.w.z=0;
    sceGumTranslate(&offset);	// rotate about position.
    sceGumMultMatrix(&rotation);
/*    if(ticks<=object->anim.node[0].ticks) {
        printf("object '%s' @%d ticks\n",object->name,ticks);
        printf("M=[%.4f,%.4f,%.4f,%.4f]\n",rotation.x.x,rotation.x.y,rotation.x.z,rotation.x.w);
        printf("  [%.4f,%.4f,%.4f,%.4f]\n",rotation.y.x,rotation.y.y,rotation.y.z,rotation.y.w);
        printf("  [%.4f,%.4f,%.4f,%.4f]\n",rotation.z.x,rotation.z.y,rotation.z.z,rotation.z.w);
        printf("  [%.4f,%.4f,%.4f,%.4f]\n",rotation.w.x,rotation.w.y,rotation.w.z,rotation.w.w);
        printf("Should have been:\n");
        printf("M=[%.4f,%.4f,%.4f,%.4f]\n",object->matrix[0],object->matrix[1],object->matrix[2],object->matrix[3]);
        printf("  [%.4f,%.4f,%.4f,%.4f]\n",object->matrix[4],object->matrix[5],object->matrix[6],object->matrix[7]);
        printf("  [%.4f,%.4f,%.4f,%.4f]\n",object->matrix[8],object->matrix[9],object->matrix[10],object->matrix[11]);
        printf("  [%.4f,%.4f,%.4f,%.4f]\n",object->matrix[12],object->matrix[13],object->matrix[14],object->matrix[15]);
    } */

    if(object->anim.nodeCount==0) {
		sceGumMultMatrix((ScePspFMatrix4 *)object->matrix);
	}
    sceGumMultMatrix((ScePspFMatrix4 *)object->unmatrix);

    // draw the model
    if(object->image) {
		Image *source=object->image;
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->imageWidth, source->imageHeight, source->textureWidth, source->data);
		sceGuTexScale(1.0f,1.0f);
		sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGB);
		sceGuEnable(GU_LIGHTING);
		//printf("showing image %08x\n",(int)source);
    }
    sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_NORMAL_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,object->vertCount,0,object->vertImage);
    sceGumPopMatrix();
}

/** draws the current frame of the animation for the scene based on the current clock and interpolation
 * \param scene the scene to draw
 */
void drawAseScene(struct AseScene *scene)
{
	if(!scene) return;
    int ticks=scene->ticks;
    int i;
	sceGuFrontFace(GU_CCW);
    for(i=0;i<scene->objectCount;i++) {
        drawAseObject(scene->object+i,ticks,scene->keyframe,scene->keyframeCount);
    }
	sceGuFrontFace(GU_CW);
}
#endif
