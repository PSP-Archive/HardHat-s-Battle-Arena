#ifndef PARTICLE_H
#define PARTICLE_H

struct ParticleDescription {
	int overallDuration;	// in 1/1000th of a second for whole effect
	int spawnRate;	// in particles per second
	int duration;	// in 1/1000th of a second for one particle
	float durationVariance;	// in percent/100
	float directionPerSecond[3];	// emission direction and speed
	float scalePerSecond;	// how much the grows/shrinks per second
	float windPerSecond[3];	// wind direction and speed
	float rotationPerSecond;	// rotation in radians
	float initialRadius;	// perpendicular to the direction vector
	float size;		// for rendering purposes
	int which;	// the particular texture to use
	float directionVariance;	// in percent/100
	unsigned long startColor;
	unsigned long endColor;
};

struct Particle {
	struct Particle *next;
	float x,y,z;	// position
	float dx,dy,dz;	// direction vector
	float yaw;	// spin in plane parallel to viewing plane.
	float windX,windY,windZ;	// current speed of the wind.
	int timeToLive;	// in 1/1000ths of a second
	unsigned long startColor;
	unsigned long endColor;
};

struct ParticleEmitter {
	float x,y,z;	// position
	struct ParticleDescription *desc;
	int elapsed;
	int emitElapsed;
	struct Particle *particles;
	int particleCount;
};

void updateParticle(struct Particle *p,struct ParticleEmitter *pe,int elapsed);
int updateParticles(struct ParticleEmitter *pe,int elapsed);
void newParticle(struct ParticleEmitter *emitter,struct ParticleDescription *desc,float x,float y,float z);
void drawParticles(struct ParticleEmitter *pe);

#endif
