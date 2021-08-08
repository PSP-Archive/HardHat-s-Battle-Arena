#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/unistd.h>
#ifdef _PSP
#include<pspkernel.h>
#include<pspgu.h>
#include<pspgum.h>
#else
#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>
#include<math.h>
#define GU_PI ((float)(M_PI))
#endif
#include "main.h"
#include "particle.h"

#ifdef UNIT_TEST
#define NTEX 5
unsigned int texid[NTEX];
int texImageMap[NTEX];
ScePspFMatrix4 view;
#endif
extern ScePspFMatrix4 view;

Image *particleImage;

void eulerToMatrix(ScePspFVector4 *rot,float *matrix);

void updateParticle(struct Particle *p,struct ParticleEmitter *pe,int elapsed)
{
	struct ParticleDescription *desc=pe->desc;
	float eInSec=elapsed*(1/1000.0f);
	p->x+=p->dx*eInSec;
	p->y+=p->dy*eInSec;
	p->z+=p->dz*eInSec;
	// Now rotate as needed with scaling
	float angle=desc->rotationPerSecond*(1/1000.0f);
	ScePspFVector4 euler={desc->directionPerSecond[0],desc->directionPerSecond[1],desc->directionPerSecond[2],angle};
#ifdef _PSP
	gumNormalize((ScePspFVector3 *)&euler);
#else
	{
		float mag=euler.x*euler.x+euler.y*euler.y+euler.z*euler.z;
		if(mag==0) mag=1; else mag=sqrtf(mag);
		euler.x/=mag;
		euler.y/=mag;
		euler.z/=mag;
	}
#endif
	ScePspFMatrix4 mat;
	eulerToMatrix(&euler,(float *)&mat);
	float x=p->x,y=p->y,z=p->z;
	float scale=1.0f+desc->scalePerSecond*elapsed*(1/1000.0f);
	// final matrix multiple of the rotation vector, then scale it.
	p->x=(x*mat.x.x+y*mat.y.x+z*mat.z.x)*scale;
	p->y=(x*mat.x.y+y*mat.y.y+z*mat.z.y)*scale;
	p->z=(x*mat.x.z+y*mat.y.z+z*mat.z.z)*scale;
	p->windX=0.99f*p->windX+0.01f*desc->windPerSecond[0];
	p->windY=0.99f*p->windY+0.01f*desc->windPerSecond[1];
	p->windZ=0.99f*p->windZ+0.01f*desc->windPerSecond[2];
	if(p->timeToLive<desc->duration/3) {
		unsigned long alpha=desc->startColor>>24;
		//printf("from alpha %x ",alpha);
		alpha=(alpha*p->timeToLive/(desc->duration/3))<<24;
		//printf("alpha fades to %x\n",alpha);
		p->endColor=(desc->endColor&0x00ffffff)|alpha;
		p->startColor=(desc->startColor&0x00ffffff)|alpha;
	}
	p->timeToLive-=elapsed;
	
	//printf("Particle at %.4f,%.4f,%.4f; delta: %.4f,%.4f,%.4f; ",p->x,p->y,p->z,p->dx,p->dy,p->dz);
	//printf("ttl=%d\n",p->timeToLive);
}

int updateParticles(struct ParticleEmitter *pe,int elapsed)
{
	struct ParticleDescription *desc=pe->desc;
	// update existing ones, and delete if needed.
	if(!desc) return 1;
//printf("Updating particle emitter at time %d by %d\n",pe->elapsed,elapsed);
	struct Particle **src;
	struct Particle *p;
	src=&pe->particles;
	p=*src;
	int count=0;
	while(p) {
		count++;
		//printf("%d: ",count);
		updateParticle(p,pe,elapsed);

		if(p->timeToLive<=0) {
//printf("Removed particle\n");
			*src=p->next;
			free(p);
			p=*src;
			pe->particleCount--;
			continue;
		}
		// next
		src=&p->next;
		p=p->next;	
	}
	//if(count>0) printf("particle count: %d ",count);

	// spawn any as needed
	pe->emitElapsed+=elapsed;
	if(desc->spawnRate==0) desc->spawnRate=1;	// assertion.
	while(pe->elapsed<desc->overallDuration && pe->emitElapsed>desc->spawnRate) {
		// add a new one in.
		p=(struct Particle *)malloc(sizeof(struct Particle));
//printf("Added particle at time %d\n",pe->emitElapsed);
		
		float angle=GU_PI*2*rand()/(float)RAND_MAX;
		float c=cosf(angle),s=sinf(angle);
		float A=desc->directionPerSecond[0],B=desc->directionPerSecond[1],C=desc->directionPerSecond[2];
		float mag=sqrtf(A*A+B*B+C*C);
		if(mag!=0) { float maginv=1/mag; A*=maginv; B*=maginv; C*=maginv; }
		p->x=desc->initialRadius*s*B+desc->initialRadius*c*C;
		p->y=desc->initialRadius*s*C+desc->initialRadius*c*A;
		p->z=desc->initialRadius*s*A+desc->initialRadius*c*B;
		float variance=mag*((float)rand()/RAND_MAX*desc->directionVariance-desc->directionVariance/2);
		p->dx=desc->directionPerSecond[0]+variance;
		variance=mag*((float)rand()/RAND_MAX*desc->directionVariance-desc->directionVariance/2);
		p->dy=desc->directionPerSecond[1]+variance;
		variance=mag*((float)rand()/RAND_MAX*desc->directionVariance-desc->directionVariance/2);
		p->dz=desc->directionPerSecond[2]+variance;
		p->yaw=GU_PI*2*rand()/(float)RAND_MAX;
		p->timeToLive=desc->duration+(int)(desc->duration*desc->durationVariance*rand()/(float)RAND_MAX);
		p->windX=0;
		p->windY=0;
		p->windZ=0;
		unsigned long alpha=(192+((rand()>>16)&31))<<24;
		p->startColor=(desc->startColor&0x00ffffff)|alpha;
		p->endColor=(desc->endColor&0x00ffffff)|alpha;
		//p->startColor=desc->startColor;
		//p->endColor=desc->endColor;
		pe->emitElapsed-=desc->spawnRate;
		p->next=pe->particles;
		pe->particles=p;
		pe->particleCount++;
	}
		
	pe->elapsed+=elapsed;
	return pe->particleCount==0 && pe->elapsed>desc->spawnRate;
}

void newParticle(struct ParticleEmitter *emitter,struct ParticleDescription *desc,float x,float y,float z)
{
	//printf("New particle emitter\n");
	emitter->desc=desc;
	emitter->x=x;
	emitter->y=y;
	emitter->z=z;
	emitter->elapsed=0;
	emitter->emitElapsed=desc?desc->spawnRate:0;
	// should free old particles if there are any hanging around...
	struct Particle *p,*next;
	for(p=emitter->particles;p!=0;p=next) {
		next=p->next;
		free(p);
	}
	emitter->particles=0;
	emitter->particleCount=0;
	if(!particleImage) {
		particleImage=loadPng("data/particle.png");
#ifdef _PSP
		swizzleFast(particleImage);
#endif
	}
}

void drawParticles(struct ParticleEmitter *pe)
{
	if(!pe) return;
	if(!pe->desc) return;

	struct Vertex3DTCP *vert,*v,*tcpVert;
#ifdef _PSP
	v=(struct Vertex3DTCP *)sceGuGetMemory(sizeof(struct Vertex3DTCP)*pe->particleCount*6);
	//v=(struct Vertex3DTCP *)(((int)v)&~0x40000000);	// uncached to cached version.
#else
	v=(struct Vertex3DTCP *)malloc(sizeof(struct Vertex3DTCP)*pe->particleCount*6);
#endif
	tcpVert=v;
	struct Particle *p;
	struct ParticleDescription *desc=pe->desc;

	float uu=(desc->which%4)/4.0f,vv=(desc->which/4)/4.0f;
	vert=v;
	int count=0;
	const struct Corner {int x,y; float u,v;} corner[6]={ 
		{-1,-1,0.0f,0.0f}, {-1,1,0.0f,0.25f}, {1,-1,0.25f,0.0f}, 
		{1,-1,0.25f,0.0f}, {-1,1,0.0f,0.25f}, {1,1,0.25f,0.25f}, 
	};
	if(pe->particles==NULL) return;
	for(p=pe->particles;p!=NULL;p=p->next) {
		unsigned int r,g,b,a;
		unsigned int firstColor=p->startColor;
		unsigned int lastColor=p->endColor;
		r=firstColor&0xff;
		g=(firstColor>>8)&0xff;
		b=(firstColor>>16)&0xff;
		a=(firstColor>>24)&0xff;
		int offset=p->timeToLive;
		float per=(offset/(float)desc->duration);
		if(per>1.0f) per=1.0f;
		float cent=1.0f-per;
		r=(unsigned int)(r*per+(lastColor&0xff)*cent);
		g=(unsigned int)(g*per+((lastColor>>8)&0xff)*cent);
		b=(unsigned int)(b*per+((lastColor>>16)&0xff)*cent);
		a=(unsigned int)(a*per+((lastColor>>24)&0xff)*cent);
		unsigned int color=r|(g<<8)|(b<<16)|(a<<24);
		float x=pe->x,y=pe->y,z=pe->z;
		x+=p->x;
		y+=p->y;
		z+=p->z;
		float particleElapsed=(desc->duration-p->timeToLive)*(1/1000.0f);
		x+=p->windX*particleElapsed;
		y+=p->windY*particleElapsed;
		z+=p->windZ*particleElapsed;
		int i; 
		float s=sinf(p->yaw),c=cosf(p->yaw);
		for(i=0;i<6;i++) {
			v->color=color;
			v->u=uu+corner[i].u;
			v->v=vv+corner[i].v;
			float rotx=corner[i].x*c+corner[i].y*s;
			float roty=corner[i].x*(-s)+corner[i].y*c;
			v->x=x+(view.x.x*rotx+view.x.y*roty)*desc->size;
			v->y=y+(view.y.x*rotx+view.y.y*roty)*desc->size;
			v->z=z+(view.z.x*rotx+view.z.y*roty)*desc->size;
//if(count==0) printf("init vert0x%08x %d at %.4f,%.4f,%.4f (u=%.2f,v=%.2f) 0x%08x\n",(int)v,count,v->x,v->y,v->z,v->u,v->v,(int)v->color);
			v++;
			count++;
		}
	}
	//printf("Allocated %d verts, and filled in %d verts.\n",pe->particleCount*6,count);
#ifdef _PSP
	if(particleImage) {
		Image *source=particleImage;
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);
		sceGuTexScale(1.0f,1.0f);
		sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
		sceGuDisable(GU_LIGHTING);
		//printf("showing image %08x\n",(int)source);
	}

	sceKernelDcacheWritebackAll();
	
	sceGumMatrixMode(GU_MODEL);
	sceGumPushMatrix();
	sceGumTranslate((ScePspFVector3 *)&pe->x);
	//sceGuAlphaFunc(GU_GREATER,3,0xff);
	//sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthMask(GU_TRUE);
	//sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_TRUE);
	sceGuDisable(GU_CULL_FACE);
	sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TEXTURE_32BITF|GU_COLOR_8888|GU_TRANSFORM_3D,pe->particleCount*6,0,tcpVert);
	sceGuEnable(GU_CULL_FACE);
	//sceGuDisable(GU_ALPHA_TEST);
	//sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_FALSE);
	sceGumPopMatrix();
#else
	int i=0;
    if(particleImage) {
        Image *image=particleImage;
        glBindTexture(GL_TEXTURE_2D,texid[i]);
        //printf("Binding texture %d\n",texid[i]);
        (glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->textureWidth, image->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data));
        (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }
	int vertCount=2*pe->particleCount;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    //int i;
    v=vert;
    //printf("particleCount=%d\n",pe->particleCount);
    for(i=0;i<pe->particleCount*6;i++) {
        struct Vertex3DTCP *vert=v+i;
	//printf("Vert0x%08x %d at %.4f,%.4f,%.4f (u=%.2f,v=%.2f) 0x%08x\n",(int)vert,i,vert->x,vert->y,vert->z,vert->u,vert->v,vert->color);
        //glColor4ub((vert->color>>16)&255, (vert->color>>8)&255, (vert->color)&255,(vert->color>>24)&255);
        //glColor3ub((vert->color>>16)&255, (vert->color>>8)&255, (vert->color)&255);
        glColor4f(((vert->color>>16)&255)/255.0f, ((vert->color>>8)&255)/255.0f, ((vert->color)&255)/255.0f,((vert->color>>24)&255)/255.0f);
        glTexCoord2f(vert->u,vert->v);
        glVertex3f(vert->x,vert->y,vert->z);
    }
    glEnd();	// GL_TRIANGLES
	glPopMatrix();
	free(vert);
#endif
}

#ifdef UNIT_TEST
float cameraTheta=0,cameraPhi=M_PI/3;
float deltaTheta=0,deltaPhi=0;
float dist=350;
struct ParticleEmitter emitter;
struct ParticleDescription desc={5000,20,600,0.25f,{0,80,0},1.1f,{2,0,2},6.28f,20,5,5,0.05,0xffff00ff,0xff00ffff};

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
    gluLookAt(x,y,z,0,0,0,0,1,0);
    glGetFloatv(GL_MODELVIEW_MATRIX, (float *)&view); 

    drawParticles(&emitter);
    glutSwapBuffers();
    usleep(16666);
    if(updateParticles(&emitter,1000/60)) {
		printf("new particle triggered.\n");
    	newParticle(&emitter,&desc,0,0,0);
    }
    //printf("scene ticks at: %d\n",scene->ticks);
    glutPostRedisplay();
}

int attribute=0;
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
    case GLUT_KEY_F1:
		attribute=1;
        break;
    case GLUT_KEY_F2:
		attribute=2;
        break;
    case GLUT_KEY_F3:
		attribute=3;
        break;
    case GLUT_KEY_F4:
		attribute=4;
        break;
    case GLUT_KEY_F5:
		attribute=5;
        break;
    case GLUT_KEY_F6:
		attribute=6;
        break;
    case GLUT_KEY_F7:
		attribute=7;
        break;
    case GLUT_KEY_F8:
		attribute=8;
        break;
    case GLUT_KEY_F9:
		attribute=9;
        break;
    case GLUT_KEY_F10:
		attribute=10;
        break;
    case GLUT_KEY_F11:
		attribute=11;
        break;
    case GLUT_KEY_F12:
		attribute=12;
        break;

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
		if(dist<550) {
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

    printf("Hello\n");	

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

	particleImage=loadPng("data/particle.png");
    Image *image=particleImage;
    i=0;
    if(image && nextTex<NTEX) {
       (glBindTexture(GL_TEXTURE_2D, texid[nextTex]));
       printf("uploading texture data for %d\n", texid[nextTex]);
       (glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->textureWidth, image->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data));
       texImageMap[i]=texid[nextTex];
       nextTex++;
    }

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
printf("Started.\n");
   	newParticle(&emitter,&desc,0,0,0);

    (glFrontFace(GL_CCW));
    (glDisable(GL_DEPTH_TEST));
    (glDisable(GL_CULL_FACE));
    (glEnable(GL_TEXTURE_2D));

    glutMainLoop();

    printf("Goodbye\n");
    return 0;
}
#endif
