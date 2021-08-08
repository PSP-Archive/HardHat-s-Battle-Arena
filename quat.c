#ifndef quat
struct quat {
    float w,x,y,z;
};
#endif

// with thanks to wikipedia.
void rotateQuaternion( struct quat *result,struct quat *vert,struct quat *rot1)
{
	float t2,t3,t4,t5,t6,t7,t8,t9,t10;
	float a=rot1->w,b=rot1->x,c=rot1->y,d=rot1->z;
	float v1=vert->x,v2=vert->y,v3=vert->z;
	t2 =   a*b;
	t3 =   a*c;
	t4 =   a*d;
	t5 =  -b*b;
	t6 =   b*c;
	t7 =   b*d;
	t8 =  -c*c;
	t9 =   c*d;
	t10 = -d*d;
	result->x = 2*( (t8 + t10)*v1 + (t6 -  t4)*v2 + (t3 + t7)*v3 ) + v1;
	result->y = 2*( (t4 +  t6)*v1 + (t5 + t10)*v2 + (t9 - t2)*v3 ) + v2;
	result->z = 2*( (t7 -  t3)*v1 + (t2 +  t9)*v2 + (t5 + t8)*v3 ) + v3;
}

void multiplyQuaternion( struct quat *result,struct quat *qa,struct quat *qb)
{
	float w,x,y,z;

	w=qa->w*qb->w-qa->x*qb->x-qa->y*qb->y-qa->z*qb->z;
	x=qa->x*qb->w+qa->w*qb->x+qa->y*qb->z-qa->z*qb->y;
	y=qa->y*qb->w+qa->w*qb->y+qa->z*qb->x-qa->x*qb->z;
	z=qa->z*qb->w+qa->w*qb->z+qa->x*qb->y-qa->y*qb->x;

	result->w=w;
	result->x=x;
	result->y=y;
	result->z=z;
}

void normalizeQuaternion( struct quat *q)
{
	float len=q->w*q->w+q->x*q->x+q->y*q->y+q->z*q->z;
	if(len==0) return;	// I can't help you.
	//printf("Normalize: %f magnitude\n",len);
	len=1.0f/sqrtf(len);
	q->w*=len;
	q->x*=len;
	q->y*=len;
	q->z*=len;
}

float dotQuaternion(struct quat *one,struct quat *two)
{
    return one->w*two->w+one->x*two->x+one->y*two->y+one->z*two->z;
}

void slerpQuaternion(struct quat *a,struct quat *b,float t,struct quat *output)
{
    float w1,w2;
    float cosTheta=dotQuaternion(a,b);
    float theta=acosf(cosTheta);
    float sinTheta=sin(theta);

    if( sinTheta>0.001f) {
        w1=sinf( (1.0f-t)*theta ) / sinTheta;
        w2=sinf( t*theta )/sinTheta;
    } else {
        w1=1.0f-t;
        w2=t;
    }
    // now do output=a*w1 + b*w2;
}

void vectorAngleToQuat(ScePspFVector4 *a,ScePspFVector4 *b,float t,struct quat *quat)
{
    float t1=1-t;

    float x=a->x*t+b->x*t1,y=a->y*t+b->y*t1,z=a->z*t+b->z*t1,w=a->w*t+b->w*t1;

    float s=sinf(w/2);
	quat->w=cosf(w/2);    // convert to quaternions
	quat->x=s*x;
	quat->y=s*y;
	quat->z=s*z;
}

