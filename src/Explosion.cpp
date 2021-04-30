#include "Explosion.h"

// Modified by Michael Kang for CS134.

// Debris class definitions.
Debris::Debris() {

	// Initialize particle with some reasonable values first.
	//
	velocity.set(0, 0, 0);
	acceleration.set(0, 0, 0);
	position.set(0, 0, 0);
	forces.set(0, 0, 0);
	lifespan = 5;
	birthtime = 0;
	radius = .1;
	damping = .99;
	mass = 1;
	color = ofColor::white;
}

void Debris::draw() {
	alpha -= 5;
	ofEnableAlphaBlending();
	ofSetColor(255, 255, 255, alpha);
	image.draw(-width / 2.0 + position.x, -height / 2.0 + position.y);
	image.rotate90(1);
	ofDisableAlphaBlending();
}

// Physics based movement for explosion particles.
void Debris::integrate() {
	// Interval for this step.
	float dt = 1.0 / ofGetFrameRate();

	// Update position based on velocity.
	position += (velocity * dt);

	// Update acceleration with accumulated particles forces.
	ofVec3f accel = acceleration; 
	accel += (forces * (1.0 / mass));
	velocity += accel * dt;

	// Add a little damping for good measure.
	velocity *= damping;

	// Clear forces on particle (they get re-added each step).
	forces.set(0, 0, 0);
}

void Debris::setImage(ofImage img) {
	image = img;
	width = image.getWidth();
	height = image.getHeight();
}

//  Return age in seconds.
float Debris::age() {
	return (ofGetElapsedTimeMillis() - birthtime) / 1000.0;
}

// Explosion system class definitions.
ExplosionSystem::ExplosionSystem() { }

void ExplosionSystem::add(const Debris &p) {
	debris.push_back(p);
}

void ExplosionSystem::addForce(ParticleForce *f) {
	forces.push_back(f);
}

void ExplosionSystem::remove(int i) {
	debris.erase(debris.begin() + i);
}

void ExplosionSystem::setLifespan(float l) {
	for (int i = 0; i < debris.size(); i++) {
		debris[i].lifespan = l;
	}
}

void ExplosionSystem::reset() {
	for (int i = 0; i < forces.size(); i++) {
		forces[i]->applied = false;
	}
}

void ExplosionSystem::update() {
	// Check if empty and just return.
	if (debris.size() == 0) return;

	vector<Debris>::iterator p = debris.begin();
	vector<Debris>::iterator tmp;

	// Check which particles have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, we need to use an iterator.
	while (p != debris.end()) {
		if (p->lifespan != -1 && p->age() > p->lifespan) {
			tmp = debris.erase(p);
			p = tmp;
		}
		else p++;
	}

	// Update forces on all particles first.
	for (int i = 0; i < debris.size(); i++) {
		for (int k = 0; k < forces.size(); k++) {
			if (!forces[k]->applied)
				forces[k]->updateForce(&debris[i]);
		}
	}

	// Update all forces only applied once to "applied"
	// so they are not applied again.
	for (int i = 0; i < forces.size(); i++) {
		if (forces[i]->applyOnce)
			forces[i]->applied = true;
	}

	// Integrate all the particles in the store.
	for (int i = 0; i < debris.size(); i++) {
		debris[i].integrate();
	}
}

//  Draw the particle cloud.
//
void ExplosionSystem::draw() {
	for (int i = 0; i < debris.size(); i++) {
		debris[i].draw();
	}
}

// Explosion emitter class definitions.
Explosion::Explosion() {
	sys = new ExplosionSystem();
	createdSys = true;
	init();
}

Explosion::Explosion(ExplosionSystem *s) {
	sys = s;
	createdSys = true;
	init();
}

Explosion::~Explosion() {
	// Deallocate particle system if emitter created one internally.
	//
	if (createdSys) delete sys;
}

void Explosion::init() {
	rate = 1;
	velocity = ofVec3f(0, 0, 0);
	lifespan = 1;
	started = false;
	oneShot = true;
	fired = false;
	firedOnce = false;
	lastSpawned = 0;
	radius = 1;
	particleRadius = 1;
	visible = true;
	groupSize = 10;
}

void Explosion::draw() {
	sys->draw();
}

void Explosion::start() {
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
}

void Explosion::stop() {
	started = false;
	fired = false;
}

void Explosion::update() {

	float time = ofGetElapsedTimeMillis();

	if (oneShot && started) {
		if (!fired) {
			// Spawn a new particle(s).
			for (int i = 0; i < groupSize; i++) { spawn(time); }
			lastSpawned = time;
		}
		fired = true;
		firedOnce = true;
		stop();
	}

	else if (((time - lastSpawned) > (1000.0 / rate)) && started) {
		// Spawn a new particle(s).
		for (int i = 0; i < groupSize; i++) { spawn(time); }
		lastSpawned = time;
	}

	sys->update();
}

// Spawn a single particle.  time is current time of birth.
//
void Explosion::spawn(float time) {

	Debris particle;

	ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), 0);
	float speed = velocity.length();
	particle.velocity = dir.getNormalized() * speed;
	particle.position.set(position);
	particle.lifespan = lifespan;
	particle.birthtime = time;
	particle.radius = particleRadius;
	particle.setImage(debrisImage);

	// Add to system.
	sys->add(particle);
}

// Gravity Force Field 
//
GravityForce::GravityForce(const ofVec3f &g) {
	gravity = g;
}

void GravityForce::updateForce(Debris * particle) {
	//
	// f = mg
	//
	particle->forces += gravity * particle->mass;
}

// Impulse Radial Force - this is a "one shot" force that
// eminates radially outward in random directions.
ImpulseRadialForce::ImpulseRadialForce(float magnitude) {
	this->magnitude = magnitude;
	this->height = 1;
	applyOnce = true;
}

void ImpulseRadialForce::updateForce(Debris *particle) {
	ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), 0);
	particle->forces += dir.getNormalized() * magnitude;
}