#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include"main.h"
#include"ase.h"

#define NTEX 64
int texid[NTEX];
int texImageMap[NTEX];

void reshape (int w, int h)
{
    (glViewport(0, 0, w, h));
    (glMatrixMode(GL_PROJECTION));
    (glLoadIdentity());
//      GLCHK(glOrtho(-20, 20, -20, 20, -200, 200));
//      GLCHK(glFrustum(-20.0,20.0,-20.0,20.0,1.5,40.0));
    (gluPerspective(45.0,(float)w/h,1.5,512.0));
    (glMatrixMode(GL_MODELVIEW));
    (glLoadIdentity());
}

// from gumInternal.c -- licensed under BSD licence
void multMatrix(ScePspFMatrix4* result, const ScePspFMatrix4* a, const ScePspFMatrix4* b)
{
        ScePspFMatrix4 t;
        unsigned int i,j,k;
        const float* ma = (const float*)a;
        const float* mb = (const float*)b;
        float* mr = (float*)&t;

        for (i=0; i<4; i++) {
                for (j=0; j<4; j++) {
                        float v=0.0f;
                        for (k=0; k<4; k++)
                                v+=ma[(k<<2)+j]*mb[(i<<2)+k];
                        mr[(i<<2)+j]=v;
                }
        }

        memcpy(result,&t,sizeof(ScePspFMatrix4));
}

static int cmpInt(const void *one,const void *two)
{
	const int *a=(int *)one,*b=(int *)two;
	return *a<*b?-1:*a>*b?1:0;
}

static void drawAseObject(struct AseMaterial *material,int materialCount,struct AseObject *object,int ticks,int *keyframe,int keyframeCount)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // now do the incremental translations
    ScePspFMatrix4 rotation={{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
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
                multMatrix(&rotation,&rotAngle,&rotation);
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
			if(!keyframeNode) break;	// ASSERT: keyframeNode can't be NULL...
			int keyframePos=((int)keyframeNode-(int)keyframe)/sizeof(int);
			if(keyframePos==0) break;	// ASSERT: keyframePos can't be 0...
			if(keyframe[keyframePos-1]>ticks) break;	// nothing is happening here.
			oldTicks=keyframe[keyframePos-1];
            float t=(ticks-oldTicks)/(float)(node->ticks-oldTicks);
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
                multMatrix(&rotation,&rotAngle,&rotation);
            }
if(strcmp(object->name,"tom4\"")==0) printf("%d: t=%.4f pos=%.2f,%.2f,%.2f; rot=%.4f\n",i,t,offset.x,offset.y,offset.z,node->rot.w);
#endif
            break;
        }
    }
    rotation.w.x=0;	// translate back after rotation.
    rotation.w.y=0;
    rotation.w.z=0;
    glTranslatef(offset.x,offset.y,offset.z);	// rotate about position.
    glMultMatrixf((float *)&rotation);
    if(object->anim.nodeCount==0) {
		glMultMatrixf(object->matrix);
	}
    glMultMatrixf((float *)object->unmatrix);

    // draw the model
    if(object->image) {
        int i;
        for(i=0;i<materialCount;i++) {
            if(material[i].image==object->image) {
                Image *image=material[i].image;
                glBindTexture(GL_TEXTURE_2D,texid[i]);
                (glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->textureWidth, image->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data));
                (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                break;
            }
        }
    }
	//sceGuTexScale(1.0f,1.0f);
	//sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGB);
	glEnable(GL_LIGHTING);
	//printf("showing image %08x\n",(int)source);
    //int i;
    struct AseAnimationNode *node=0, *nextNode=0;
    for(i=0;i<object->anim.nodeCount;i++) {
	node=object->anim.node+i;
	if(node->ticks<=ticks && i+1<object->anim.nodeCount && node[1].ticks>ticks) {
		nextNode=node+1;
		break;
	}
    }
    glBegin(GL_TRIANGLES);
    for(i=0;i<object->vertCount;i++) {
        struct Vertex3DTNP *vert=object->vertImage+i;
        glColor3f(1, 1, 1);
        glTexCoord2f(vert->u,vert->v);
        glNormal3f(vert->nx,vert->ny,vert->nz);
        glVertex3f(vert->x,vert->y,vert->z);
    }
    glEnd();	// GL_TRIANGLES
    glPopMatrix();
}

/*
enum TransformStage {TS_END,TS_OBJTRANSLATE,TS_OBJUNTRANSLATE,TS_OBJROTATE,TS_OBJUNROTATE,
	TS_ANIMTRANSLATE,TS_ANIMUNTRANSLATE,TS_ANIMROTATE,TS_ANIMUNROTATE,TS_MAX } stage[16]={
	TS_ANIMTRANSLATE,TS_ANIMROTATE,TS_OBJUNROTATE,TS_OBJUNTRANSLATE,TS_END
};
const char *stageName[]={"End\n","Object translate;","Object untranslate;","Object rotate;","Object unrotate;","Anim Translate;","Anim Untranslate;","Anim Rotate;","Anim Unrotate;","Max"};
int stageCursor=0;

void preObjectTransform(struct AseObject *object,struct AseAnimationNode *node,struct AseAnimationNode *nextNode,int ticks)
{
	if(node==0) {
		printf("empty animation node.\n");
		return;
	}
	glMatrixMode(GL_MODELVIEW);
	int i,j;
	float M[16];
	for(i=0;i<16;i++) {
		switch(stage[i]) {
		case TS_END:
			return;
		case TS_OBJTRANSLATE:
			glTranslatef(object->matrix[12],object->matrix[13],object->matrix[14]);
			break;
		case TS_OBJUNTRANSLATE:
			glTranslatef(-object->matrix[12],-object->matrix[13],-object->matrix[14]);
			break;
		case TS_OBJROTATE:
			for(j=0;j<16;j++) M[j]=object->matrix[j]; M[12]=0.0f; M[13]=0.0f; M[14]=0.0f;
			glMultMatrixf(M);
			break;
		case TS_OBJUNROTATE:
			for(j=0;j<16;j++) M[j]=object->matrix[(j%4)*4+(j/4)]; M[3]=0; M[7]=0; M[11]=0;
			glMultMatrixf(M);
			break;
		case TS_ANIMTRANSLATE:
			glTranslatef(node->pos.x,node->pos.y,node->pos.z);
			break;
		case TS_ANIMUNTRANSLATE:
			glTranslatef(-node->pos.x,-node->pos.y,-node->pos.z);
			break;
		case TS_ANIMROTATE:
			for(j=0;j<16;j++)
				M[j]=((float *)&node->mat)[j];
			M[12]=0; M[13]=0; M[14]=0;
			glMultMatrixf(M);
			break;
		case TS_ANIMUNROTATE:
			for(j=0;j<16;j++)
				M[j]=((float *)&node->mat)[(j%4)*4+(j/4)];
			M[3]=0; M[7]=0; M[11]=0;
			glMultMatrixf(M);
			break;
		}
	}
}

void displayObject(struct AseMaterial *material,int materialCount,struct AseObject *object,int ticks)
{
    if(object->image) {
        int i;
        for(i=0;i<materialCount;i++) {
            if(material[i].image==object->image) {
                Image *image=material[i].image;
                glBindTexture(GL_TEXTURE_2D,texid[i]);
                (glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->textureWidth, image->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data));
                (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                break;
            }
        }
    }
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    int i;
    struct AseAnimationNode *node=0, *nextNode=0;
    for(i=0;i<object->anim.nodeCount;i++) {
	node=object->anim.node+i;
	if(node->ticks<=ticks && i+1<object->anim.nodeCount && node[1].ticks>ticks) {
		nextNode=node+1;
		break;
	}
    }
    preObjectTransform(object,node,nextNode,ticks);
    glBegin(GL_TRIANGLES);
    for(i=0;i<object->vertCount;i++) {
        struct Vertex3DTNP *vert=object->vertImage+i;
        glColor3f(1, 1, 1);
        glTexCoord2f(vert->u,vert->v);
        glNormal3f(vert->nx,vert->ny,vert->nz);
        glVertex3f(vert->x,vert->y,vert->z);
    }
    glEnd();	// GL_TRIANGLES
    glPopMatrix();
}
*/

float cameraTheta=5.5,cameraPhi=0.42;
float deltaTheta=0,deltaPhi=0;
float dist=25;
struct AseScene *scene;
void display ()
{
    int i,j;
    static GLfloat angle;

    cameraTheta+=deltaTheta;
    if(cameraTheta<0) cameraTheta+=2*M_PI;
    if(cameraTheta>M_PI*2) cameraTheta-=2*M_PI;
    cameraPhi+=deltaPhi;
    if(cameraPhi<0) cameraPhi=0;
    if(cameraPhi>=M_PI/2-0.01) cameraPhi=M_PI/2-0.01;
	if(deltaTheta!=0 || deltaPhi!=0) printf("theta %.3f; phi %.3f; dist=%.2f\n",cameraTheta,cameraPhi,dist);

    // Display the 2d background
    glClearColor(0.0f,0.4f,0.4f,0.0f);
    (glClear(GL_COLOR_BUFFER_BIT));
    (glClear(GL_DEPTH_BUFFER_BIT));

    // Now 3d time
    (glMatrixMode(GL_MODELVIEW));
    (glLoadIdentity());

    (glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE));
// Handy pre-lighting
    (glEnable(GL_BLEND));
    (glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    float ct=cosf(cameraTheta),st=sinf(cameraTheta);
    float cp=cosf(cameraPhi),sp=sinf(cameraPhi);
    float x=dist*ct*cp,y=dist*st*cp,z=dist*sp;
    gluLookAt(x,y,z,0,0,0,0,0,1);

    int ticks=scene->ticks;
    //int i;
	glFrontFace(GL_CCW);
    for(i=0;i<scene->objectCount;i++) {
        drawAseObject(scene->material,scene->materialCount,scene->object+i,ticks,scene->keyframe,scene->keyframeCount);
    }
	glFrontFace(GL_CW);
	/*
    for(i=0;i<scene->objectCount;i++) {
        displayObject(scene->material,scene->materialCount,scene->object+i,scene->ticks);
    }
    */
    glutSwapBuffers();
    usleep(16666);
    if(updateAseSceneMax(scene,16,180)) {
    	resetAseScene(scene,0);
    }
    //printf("scene ticks at: %d\n",scene->ticks);
    glutPostRedisplay();
}

float speed=0.01;
void specialdown (int key, int x, int y)
{
    //printf("Got specialdown event\n");
    switch (key) {
    case GLUT_KEY_LEFT:
	deltaTheta=-speed;
        break;
    case GLUT_KEY_UP:
	deltaPhi=speed;
        break;
    case GLUT_KEY_DOWN:
	deltaPhi=-speed;
        break;
    case GLUT_KEY_RIGHT:
	deltaTheta=speed;
        break;
    default:
        break;
    }
}

void specialup (int key, int x, int y)
{
    //printf("Got specialup event\n");
    switch (key) {
#if 0
    case GLUT_KEY_F1:
	stage[0]++;
	if(stage[0]>=TS_MAX) stage[0]=TS_END;
        break;
    case GLUT_KEY_F2:
	stage[1]++;
	if(stage[1]>=TS_MAX) stage[1]=TS_END;
        break;
    case GLUT_KEY_F3:
	stage[2]++;
	if(stage[2]>=TS_MAX) stage[2]=TS_END;
        break;
    case GLUT_KEY_F4:
	stage[3]++;
	if(stage[3]>=TS_MAX) stage[3]=TS_END;
        break;
    case GLUT_KEY_F5:
	stage[4]++;
	if(stage[4]>=TS_MAX) stage[4]=TS_END;
        break;
    case GLUT_KEY_F6:
	stage[5]+=1;
	if(stage[5]>=TS_MAX) stage[5]=TS_END;
        break;
    case GLUT_KEY_F7:
	stage[6]+=1;
	if(stage[6]>=TS_MAX) stage[6]=TS_END;
        break;
    case GLUT_KEY_F8:
	stage[7]+=1;
	if(stage[7]>=TS_MAX) stage[7]=TS_END;
        break;
    case GLUT_KEY_F9:
	stage[8]+=1;
	if(stage[8]>=TS_MAX) stage[8]=TS_END;
        break;
#endif
    case GLUT_KEY_LEFT:
	deltaTheta=0;
        break;
    case GLUT_KEY_UP:
	deltaPhi=0;
        break;
    case GLUT_KEY_DOWN:
	deltaPhi=0;
        break;
    case GLUT_KEY_RIGHT:
	deltaTheta=0;
        break;
    default:
        break;
    }

	int i;
//	printf("Pre-transform:\n");
	for(i=0;i<16;i++) {
//		printf("%s",stageName[stage[i]]);
//		if(stage[i]==TS_END) break;
	}
}

void keydown (unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);
	case '[':
		if(dist>5) {
			dist-=5;
		}
		break;
	case ']':
		if(dist<250) {
			dist+=5;
		}
		break;
	default:
		break;
	}
}

int main(int argc,char **argv)
{
    int nextTex=0;

    printf("Hello,\n");
    scene=loadAseScene("sushi");
    if(!scene) {
		printf("Scene not found.\n");
		return;
	}
    printf("Scene is 0x%08x\n",(int)scene);

    // Play the scene in a window.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(480, 272);
    glutCreateWindow( __FILE__ );
    glutKeyboardFunc(keydown);
    glutSpecialFunc(specialdown);
    glutSpecialUpFunc(specialup);
printf("Generating textures\n");
    (glGenTextures(NTEX, texid));      // Hard coded.
    int i;
    for(i=0;i<scene->materialCount;i++) {
		printf("Material %d\n",i);
        Image *image=scene->material[i].image;
        printf("Image is %08x; ",(int)image);
        if(image && nextTex<NTEX) {
            (glBindTexture(GL_TEXTURE_2D, texid[nextTex]));
            printf("uploading texture data for %d\n", texid[nextTex]);
            printf("Image: %dx%d (%dx%d)\n", image->imageWidth,image->imageHeight,image->textureWidth,image->textureHeight);
            (glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->textureWidth, image->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data));
            texImageMap[i]=texid[nextTex];
            printf("uploaded texture data for %d\n", texid[nextTex]);
            nextTex++;
        }
    }
printf("Textures done.\n");

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
printf("Started.\n");

    (glFrontFace(GL_CCW));
    (glEnable(GL_DEPTH_TEST));
    (glEnable(GL_CULL_FACE));
    (glEnable(GL_TEXTURE_2D));

    glutMainLoop();

    if(scene) freeAseScene(scene);
    printf("Goodbye\n");
    return 0;
}
