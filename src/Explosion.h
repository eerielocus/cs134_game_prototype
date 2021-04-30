#pragma once

// Modified by Michael Kang for CS134.

#include "ofMain.h"

class DebrisForceField;

// Particle class used for explosions.
class Debris {
public:
	Debris();

	ofVec3f position;
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;
	float damping;
	float mass;
	float lifespan;
	float radius;
	float birthtime;
	float width, height;
	float alpha = 255;
	void integrate();
	void draw();
	void setImage(ofImage);
	float age();        // sec
	ofColor color;
	ofImage image;
};

//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
	bool applyOnce = false;
	bool applied = false;
	virtual void updateForce(Debris *) = 0;
};

// Particle System used to control debris generation during explosions.
class ExplosionSystem {
public:
	ExplosionSystem();
	void add(const Debris &);
	void addForce(ParticleForce *);
	void remove(int);
	void update();
	void setLifespan(float);
	void reset();
	int removeNear(const ofVec3f & point, float dist);
	void draw();
	vector<Debris> debris;
	vector<ParticleForce *> forces;
};

//  Explosion emitter class controlling systems involved with explosions.
//
class Explosion {
public:
	Explosion();
	Explosion(ExplosionSystem *s);
	~Explosion();
	void init();
	void draw();
	void start();
	void stop();
	void setLifespan(const float life) { lifespan = life; }
	void setVelocity(const ofVec3f &vel) { velocity = vel; }
	void setRate(const float r) { rate = r; }
	void setParticleRadius(const float r) { particleRadius = r; }
	void setGroupSize(int s) { groupSize = s; }
	void setOneShot(bool s) { oneShot = s; }
	void setPosition(const ofVec3f &pos) { position = pos; }
	void update();
	void spawn(float time);

	ExplosionSystem *sys;
	ofImage debrisImage;
	ofVec3f velocity;
	ofVec3f position, scale;
	float rate;         // per sec
	float lifespan;     // sec
	float rotation;
	float lastSpawned;  // ms
	float particleRadius;
	float radius;
	int groupSize = 25; // number of particles to spawn in a group
	bool oneShot;
	bool fired;
	bool firedOnce;
	bool bSelected;
	bool started;
	bool visible;
	bool createdSys;
};

// Some convenient built-in forces
//
class GravityForce : public ParticleForce {
	ofVec3f gravity;
public:
	GravityForce(const ofVec3f & gravity);
	void set(const ofVec3f & g) { gravity = g; }
	void updateForce(Debris *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude, height;
public:
	ImpulseRadialForce(float magnitude);
	void set(float mag) { magnitude = mag; }
	void setHeight(float h) { height = h; }
	void updateForce(Debris *);
};
