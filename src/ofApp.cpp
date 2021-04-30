#include "ofApp.h"

// Modified by Michael Kang for CS134 Project 1.
// TODO: 
//		Break up classes into seperate files.
//			- Currently kept together for convenience.
//		Possible modulation of movement paths.
//			- Currently using enum bools.
//		Create some randomness in enemy movements and alter spawn rates.
//			- Currently pretty difficult if player collision was on. (lots of enemies/shots)

BaseObject::BaseObject() {
	trans = ofVec3f(0, 0, 0);
	scale = ofVec3f(1, 1, 1);
	rot = 0;
}

void BaseObject::setPosition(glm::vec3 pos) {
	trans = pos;
}

//
// Basic Sprite Object.
//
Sprite::Sprite() {
	speed = 0;
	velocity = glm::vec3(0, 0, 0);
	lifespan = -1;      // Lifespan of -1 => immortal.
	birthtime = 0;
	bSelected = false;
	haveImage = false;
	toRotate = false;
	name = "UnamedSprite";
	width = 60;
	height = 80;
}

// Return a sprite's age in milliseconds.
//
float Sprite::age() {
	return (ofGetElapsedTimeMillis() - birthtime);
}

//  Set an image for the sprite. If you don't set one, a rectangle
//  gets drawn.
//
void Sprite::setImage(ofImage img) {
	image = img;
	haveImage = true;
	width = image.getWidth();
	height = image.getHeight();
}


//  Render the sprite.
//
void Sprite::draw() {
	ofSetColor(255, 255, 255, 255);

	// Draw image centered and add in translation amount.
	//
	if (haveImage) {
		image.draw(-width / 2.0 + trans.x, -height / 2.0 + trans.y);
	}
	else {
		// In case no image is supplied, draw something.
		// 
		ofSetColor(255, 0, 0);
		ofDrawRectangle(-width / 2.0 + trans.x, -height / 2.0 + trans.y, width, height);
	}
}

//
// Sprite System:
// Add a Sprite to the Sprite System.
//
void SpriteSystem::add(Sprite s) {
	sprites.push_back(s);
}

// Remove a sprite from the sprite system. Note that this function is not currently
// used. The typical case is that sprites automatically get removed when the reach
// their lifespan.
//
void SpriteSystem::remove(int i) {
	sprites.erase(sprites.begin() + i);
}


//  Update the SpriteSystem by checking which sprites have exceeded their
//  lifespan (and deleting).  Also the sprite is moved to it's next
//  location based on velocity and direction.
//
void SpriteSystem::update() {
	if (sprites.size() == 0) return;
	vector<Sprite>::iterator s = sprites.begin();
	vector<Sprite>::iterator tmp;

	// Check which sprites have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, use an iterator.
	//
	while (s != sprites.end()) {
		if (s->lifespan != -1 && s->age() > s->lifespan) {
			tmp = sprites.erase(s);
			s = tmp;
		}
		else s++;
	}

	//  Move sprite.
	//
	if (paths[Default]) {
		for (int i = 0; i < sprites.size(); i++) {
			sprites[i].trans += sprites[i].velocity / ofGetFrameRate();
		}
	}
}

//  Render all the sprites.
//
void SpriteSystem::draw() {
	for (int i = 0; i < sprites.size(); i++) {
		sprites[i].draw();
	}
}

// Collision detection.
bool SpriteSystem::removeNear(ofVec3f point, float dist) {
	vector<Sprite>::iterator s = sprites.begin();
	vector<Sprite>::iterator tmp;

	while (s != sprites.end()) {
		ofVec3f v = s->trans - point;
		if (v.length() < dist) {
			tmp = sprites.erase(s);
			s = tmp;
			return true;
		}
		else s++;
	}
	return false;
}

//
// Emitter:
// Create a new Emitter - needs a SpriteSystem.
//
Emitter::Emitter(SpriteSystem *spriteSys) {
	sys = spriteSys;
	lifespan = 3000;    // Milliseconds.
	started = false;

	lastSpawned = 0;
	rate = 1;    // Sprites/sec.
	velocity = glm::vec3(100, 100, 0);
	haveChildImage = false;
	haveImage = false;
	haveSound = false;
	drawable = true;
	width = 50;
	height = 50;
	scale = 75;
	cycle = 2;
}

//  Draw the Emitter if it is drawable. In many cases you would want a hidden emitter
//
void Emitter::draw() {
	ofPushMatrix();
	ofMultMatrix(getMatrix());
	if (drawable) {
		if (haveImage) {
			if (hasPower) {
				power.resize(60, 60);
				power.setAnchorPoint(power.getWidth() / 2, power.getHeight() / 2);
				power.draw(0, 0);
			}
			image.setAnchorPoint(image.getWidth() / 2, image.getHeight() / 2);
			image.draw(0, 0);
		}
		else {
			ofSetColor(0, 0, 200);
			ofDrawRectangle(-width / 2 + trans.x, -height / 2 + trans.y, width, height);
		}
	}
	ofPopMatrix();

	// Draw sprite system.
	//
	sys->draw();
}

//  Update the Emitter. If it has been started, spawn new sprites with
//  initial velocity, lifespan, birthtime.
//
void Emitter::update() {
	// Check if started, if not, check if there are any sprites still on screen,
	// and run update on them until they are removed.
	if (!started) {
		if (sys->sprites.size() > 0) { sys->update(); }
		return;
	}

	float time = ofGetElapsedTimeMillis();
	if (isEnemy) { 
		if (ofRandom(1, 1000) < 5) {
			// Spawn a new sprite.
			Sprite sprite;
			if (haveChildImage) { sprite.setImage(childImage); }

			sprite.velocity = velocity;
			sprite.lifespan = lifespan;
			sprite.setPosition(trans);
			sprite.birthtime = time;
			sys->add(sprite);

			lastSpawned = time;
			initial = true;
		}
	}
	else {
		if (!initial || (time - lastSpawned) > (1000.0 / rate)) {
			// Spawn a new sprite.
			Sprite sprite;
			if (haveChildImage) { sprite.setImage(childImage); }

			sprite.velocity = velocity;
			sprite.lifespan = lifespan;
			sprite.setPosition(trans);
			sprite.birthtime = time;
			sys->add(sprite);

			lastSpawned = time;
			initial = true;
			// Play sound (placed here to ensure sound plays during spacebar hold).
			if (haveSound) { soundEffect.play(); }
		}
	}
	
	sys->update();
}

// Determine max distance based on velocity.
float Emitter::maxDistPerFrame() {
	return velocity.length() / ofGetFrameRate();
}

float Emitter::age() {
	return (ofGetElapsedTimeMillis() - birth);
}

// Start/Stop the emitter.
//
void Emitter::start() {
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
	sys->position = trans;
}

void Emitter::stop() {
	started = false;
	initial = false;
}

void Emitter::setLifespan(float life) {
	lifespan = life;
}

void Emitter::setVelocity(ofVec3f v) {
	velocity = v;
}

void Emitter::setChildImage(ofImage img) {
	childImage = img;
	haveChildImage = true;
}

void Emitter::setImage(ofImage img) {
	image = img;
	haveImage = true;
}

void Emitter::setRate(float r) {
	rate = r;
}

void Emitter::setSound(ofSoundPlayer s) {
	soundEffect = s;
	haveSound = true;
}

//
// Mama Emitter:
// Constructor for mother of emitters.
MamaEmitter::MamaEmitter(Emitter *e, Path p) {
	emitter = e;
	paths[p] = true;
}

//  Update the Emitter. If it has been started, spawn new emitters with
//  initial velocity, lifespan, birthtime.
//
void MamaEmitter::update() {
	// Check if started, if not, check if there are any emitters still on screen,
	// and run update on them until they are removed.
	if (!started) {
		if (emitters.size() > 0) { move(); }
		return;
	}

	// Check to see if any emitter emitted has been hit (its drawable set to false)
	// If so, check if it has any shots fired, once all of its projectiles are finished,
	// remove the emitter from list.
	for (int i = 0; i < emitters.size(); i++) {
		if (emitters[i].drawable == false) {
			if (emitters[i].sys->sprites.size() == 0) { emitters.erase(emitters.begin() + i); }
		}
	}

	float time = ofGetElapsedTimeMillis();
	if (!initial || (time - lastSpawned) > (1000.0 / rate)) {
		// Set birth and lifespan and initialize a new SpriteSystem.
		emitter->birth = time;
		emitter->duration = 8000;
		emitter->sys = new SpriteSystem();

		// If the emitter hasn't started, start it.
		if (!emitter->started) { emitter->start(); }

		// Temporary randomness to test somethings.
		if (fleet == 15) {
			emitter->scale = ofRandom(35, 50);
			emitter->cycle = ofRandom(10, 15);
		}
		else if (fleet == 30) {
			emitter->scale = ofRandom(60, 75);
			emitter->cycle = 2;
			fleet = 0;
		}

		// Push a copy of emitter into vector list.
		emitters.push_back(*emitter);
		lastSpawned = time;
		initial = true;
		fleet++;
	}
	// Update emitter movement and rotation.
	move();
	rotation();
}

// Move the emitters based on path selection.
void MamaEmitter::move() {
	if (emitters.size() == 0) return;
	vector<Emitter>::iterator e = emitters.begin();
	vector<Emitter>::iterator tmp;

	// Check which sprites have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, use an iterator.
	//
	while (e != emitters.end()) {
		if (e->lifespan != -1 && e->age() > e->duration) {
			tmp = emitters.erase(e);
			e = tmp;
		}
		else e++;
	}

	//  Move sprite.
	//  TODO: Move function to each object, so each object will have its own move function that can be
	//  changed and set seperately.
	if (paths[Default]) {
		for (int i = 0; i < emitters.size(); i++) {
			emitters[i].update();
			emitters[i].trans += velocity / ofGetFrameRate();
		}
	}
	else if (paths[EnemyWave]) {
		for (int i = 0; i < emitters.size(); i++) {
			emitters[i].update();
			emitters[i].trans = sinWave(trans.x, emitters[i].trans.y + (velocity.y / ofGetFrameRate()), emitters[i].scale, emitters[i].cycle, type);
		}
	}
	else if (paths[EnemyLine]) {
		for (int i = 0; i < emitters.size(); i++) {
			emitters[i].update();
			emitters[i].trans = triWave(trans.x, emitters[i].trans.y + (velocity.y / ofGetFrameRate()), emitters[i].scale, emitters[i].cycle + 10, type);
		}
	}
}

// Rotate emitters based on player position.
void MamaEmitter::rotation() {
	for (int i = 0; i < emitters.size(); i++) {
		// Adjust rotation of emitter for draw().
		emitters[i].rot = glm::degrees(glm::orientedAngle(glm::vec3(0, -1, 0), glm::normalize(target - emitters[i].trans), glm::vec3(0, 0, 1)));
		// Adjust vector for emitter's sprite velocity.
		emitters[i].setVelocity(glm::normalize(target - emitters[i].trans) * speed);
	}
}

// Draw emitters with modified rotation/translations.
void MamaEmitter::draw() {
	for (int i = 0; i < emitters.size(); i++) {
		emitters[i].draw();
	}
}

// Use provided method to produce sine wave movements.
ofVec3f MamaEmitter::sinWave(float x, float y, float scale, float cycles, bool type) {
	float u = (cycles * y * PI) / ofGetHeight();
	if (type) { return (ofVec3f(-scale * sin(u) + x, y, 0)); }
	else { return (ofVec3f(-scale * sin(-u) + x, y, 0)); }
}

// Triangle wave for path variety.
ofVec3f MamaEmitter::triWave(float x, float y, float scale, float cycles, bool type) {
	float u = cos((cycles * y) / ofGetHeight());
	if (type) { return (ofVec3f(-scale * (asin(u) / (PI / 2)) + x, y, 0)); }
	else { return (ofVec3f(-scale * (asin(-u) / (PI / 2)) + x, y, 0)); }
}

// Collision detection.
bool MamaEmitter::removeNear(ofVec3f point, float dist) {
	vector<Emitter>::iterator e = emitters.begin();
	vector<Emitter>::iterator tmp;

	while (e != emitters.end()) {
		ofVec3f v = e->trans - point;
		// Check length and whether emitter already been hit.
		if (v.length() < dist && e->drawable) {
			// Hide emitter and stop emitting projectiles.
			e->drawable = false;
			e->stop();
			return true;
		}
		else e++;
	}
	return false;
}

//
// Powerup Control:
// Uses physics based movement.
//
PowerUp::PowerUp() {
	trans = ofVec3f(ofGetWindowWidth() / 2, 1, 0);
	heading = ofVec3f(0, 1, 0);
	velocity = heading * 10;
	acceleration = ofVec3f(0, 0, 0);
	damping = 0.99;
	hidden = false;
}

void PowerUp::draw() {
	if (!hidden) { 
		ofDrawSphere(trans, 5);
		image.resize(50, 50);
		image.setAnchorPoint(image.getWidth() / 2, image.getHeight() / 2);
		image.draw(trans.x, trans.y);
	}
}

// Reset position and parameters.
void PowerUp::reset() {
	trans = ofVec3f(ofGetWindowWidth() / 2, 1, 0);
	heading = ofVec3f(0, 1, 0);
	velocity = heading * 10;
	acceleration = ofVec3f(0, 0, 0);
	hidden = false;
}

// Use physics based movement with screen limiters to produce a bounce effect.
void PowerUp::integrate() {
	if (trans.x <= 0 || trans.x >= ofGetWindowWidth()) {
		heading.x = -heading.x;
		velocity.x = -velocity.x;
	}
	if (trans.y <= 0 || trans.y >= ofGetWindowHeight()) {
		heading.y = -heading.y;
		velocity.y = -velocity.y;
	}
	if (velocity.length() <= 20) {
		heading.rotate(ofRandom(-90, 90), ofVec3f(0, 0, 1));
		acceleration = heading * 5000;
	}

	// Calculate position:
	trans += velocity * (1.0 / ofGetFrameRate());
	velocity += acceleration * (1.0 / ofGetFrameRate());
	velocity *= damping;
	acceleration = ofVec3f(0, 0, 0);
}

bool PowerUp::removeNear(ofVec3f point, float dist) {
	ofVec3f v = trans - point;
	// Check length and whether emitter already been hit.
	if (v.length() < dist && !hidden) {
		// Hide emitter and stop emitting projectiles.
		hidden = true;
		return true;
	}
	return false;
}

float PowerUp::maxDistPerFrame() {
	return velocity.length() / ofGetFrameRate();
}

//
// Movement Control:
//
void ofApp::keyMoveLimit() {
	// Check bool table for any keys pressed, and apply movement to player object.
	// If at the edge, set position to that edge.
	if (keys[MoveLeft] == true) {
		if (player->trans.x <= leftEdge) { player->trans.x = leftEdge; }
		else { player->trans += ofVec3f(-5, 0, 0); }
	}
	if (keys[MoveRight] == true) {
		if (player->trans.x >= rightEdge) { player->trans.x = rightEdge; }
		else { player->trans += ofVec3f(5, 0, 0); }
	}
	if (keys[MoveUp] == true) {
		if (player->trans.y <= topEdge) { player->trans.y = topEdge; }
		else { player->trans += ofVec3f(0, -5, 0); }
	}
	if (keys[MoveDown] == true) {
		if (player->trans.y >= bottomEdge) { player->trans.y = bottomEdge; }
		else { player->trans += ofVec3f(0, 5, 0); }
	}
}

void ofApp::mouseMoveLimit() {
	// Check if at screen edge and halt movement if it is.
	if (player->trans.x <= leftEdge) {
		player->trans.x = leftEdge;
		// Check if hit top/bottom edge while moving diagonally.
		if (player->trans.y <= topEdge) { player->trans.y = topEdge; }
		if (player->trans.y >= bottomEdge) { player->trans.y = bottomEdge; }
	}
	else if (player->trans.x >= rightEdge) {
		player->trans.x = rightEdge;
		if (player->trans.y <= topEdge) { player->trans.y = topEdge; }
		if (player->trans.y >= bottomEdge) { player->trans.y = bottomEdge; }
	}
	else if (player->trans.y <= topEdge) {
		player->trans.y = topEdge;
		if (player->trans.x <= leftEdge) { player->trans.x = leftEdge; }
		if (player->trans.x >= rightEdge) { player->trans.x = rightEdge; }
	}
	else if (player->trans.y >= bottomEdge) {
		player->trans.y = bottomEdge;
		if (player->trans.x <= leftEdge) { player->trans.x = leftEdge; }
		if (player->trans.x >= rightEdge) { player->trans.x = rightEdge; }
	}
}

//
// Collision Control:
// Check collisions between player's shots and enemy ships.
// Create explosion upon hit.
void ofApp::checkCollisions() {
	for (Sprite &s : player->sys->sprites) {
		for (MamaEmitter *e : enemy) {
			if (e->removeNear(s.trans, player->maxDistPerFrame())) {
				pop.play();
				// Create new explosion object and forces.
				hit = new Explosion(new ExplosionSystem());
				gravityForce = new GravityForce(ofVec3f(0, 0, 0));
				radialForce = new ImpulseRadialForce(2000.0);
				// Setup explosion parameters.
				hit->setPosition(s.trans);
				hit->debrisImage = explosionImg;
				hit->sys->addForce(gravityForce);
				hit->sys->addForce(radialForce);
				hit->sys->reset();
				hit->start();
				// Push on to list of explosions.
				exp.push_back(hit);
				// Set sprite lifespan to 0 and add score.
				s.lifespan = 0;
				score += 1;
			}
		}
	}
}

// Player collisions with enemy cannon shots.
void ofApp::playerCollisions() {
	for (MamaEmitter *e : enemy) {
		// Check collision between player and enemy ship.
		if (e->removeNear(player->trans, e->maxDistPerFrame() + (player->width / 2))) {
			playerHit.play();
			lives -= 1;
			// Create new explosion object and forces.
			hit = new Explosion(new ExplosionSystem());
			gravityForce = new GravityForce(ofVec3f(0, 0, 0));
			radialForce = new ImpulseRadialForce(2000.0);
			// Setup explosion parameters.
			hit->setPosition(player->trans);
			hit->debrisImage = explosionImg;
			hit->sys->addForce(gravityForce);
			hit->sys->addForce(radialForce);
			hit->sys->reset();
			hit->start();
			// Push on to list of explosions.
			exp.push_back(hit);
			// Game over condition.
			if (lives == 0) {
				bGameOver = true;
			}
		}
		for (Emitter &em : e->emitters) {
			// Check collision between player and enemy shot.
			if (em.sys->removeNear(player->trans, em.maxDistPerFrame() + (player->width / 2))) {
				playerHit.play();
				lives -= 1;
				// Create new explosion object and forces.
				hit = new Explosion(new ExplosionSystem());
				gravityForce = new GravityForce(ofVec3f(0, 0, 0));
				radialForce = new ImpulseRadialForce(2000.0);
				// Setup explosion parameters.
				hit->setPosition(player->trans);
				hit->debrisImage = explosionImg;
				hit->sys->addForce(gravityForce);
				hit->sys->addForce(radialForce);
				hit->sys->reset();
				hit->start();
				// Push on to list of explosions.
				exp.push_back(hit);
				// Game over condition.
				if (lives == 0) {
					bGameOver = true;
				}
			}
		}
	}
}

// Collision between player and powerup.
void ofApp::powerCollisions() {
	if (power.removeNear(player->trans, power.maxDistPerFrame() + (player->width / 2))) {
		powerHit.play();
		player->hasPower = true;
		player->powertime = ofGetElapsedTimeMillis();
	}
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
	ofBackground(ofColor::black);

	// GUI setup for fire rate and direction.
	gui.setup();
	gui.add(fireRate.setup("Rate", 10, 1, 20));
	gui.add(fireDir.setup("Direction", 0, 0, 360));

	// Load images and sound.
	// These are stored in project bin/data in their specified folders.
	laserShot.load("sounds/laser.mp3");
	pop.load("sounds/pop2.mp3");
	playerHit.load("sounds/playerhit.mp3");
	powerHit.load("sounds/power.mp3");

	background.load("images/space.jpg");
	title.load("images/title.png");
	ship.load("images/ship.png");
	explosionImg.load("images/explosion.png");
	explosionImg.resize(15, 15);
	projectile.load("images/projectile.png");
	projectile.resize(10, 20);
	enemyShip.load("images/enemy.png");
	enemyProj.load("images/enemy_proj.png");
	enemyProj.resize(15, 15);
	shield.load("images/shield.png");

	// Initialize player control.
	bPlayerShoot = false;
	keys[5] = false;
	bGameStart = false;
	bGameOver = false;
	bShowGui = false;
	defaultDir = ofVec3f(0, -1000, 0);

	// Create player object.
	player = new Emitter(new SpriteSystem());
	player->setPosition(ofVec3f(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2, 0));
	player->setVelocity(defaultDir);
	player->setLifespan(2 * 1000);
	player->setRate(8);
	player->setImage(ship);
	player->setChildImage(projectile);
	player->setSound(laserShot);

	// Temp enemy object.
	enemy1 = new Emitter(new SpriteSystem());
	enemy1->setPosition(ofVec3f(ofGetWindowWidth() / 4, 0, 0));
	enemy1->setVelocity(ofVec3f(0, -100, 0));
	enemy1->setLifespan(3000);
	enemy1->setRate(0.5);
	enemy1->setImage(enemyShip);
	enemy1->setChildImage(enemyProj);
	enemy1->isEnemy = true;

	// Temp mama enemy objects.
	mama1 = new MamaEmitter(enemy1, EnemyWave);
	mama1->setPosition(ofVec3f(ofGetWindowWidth() / 4, 0, 0));
	mama1->setVelocity(ofVec3f(0, 100, 0));
	mama1->setLifespan(2000);
	mama1->setRate(2);
	mama1->target = player->trans;
	mama2 = new MamaEmitter(enemy1, EnemyWave);
	mama2->setPosition(ofVec3f(ofGetWindowWidth() - ofGetWindowWidth() / 4, 0, 0));
	mama2->setVelocity(ofVec3f(0, 100, 0));
	mama2->setLifespan(2000);
	mama2->setRate(2);
	mama2->type = false;
	mama2->target = player->trans;
	mama3 = new MamaEmitter(enemy1, EnemyLine);
	mama3->setPosition(ofVec3f(ofGetWindowWidth() / 2, 0, 0));
	mama3->setVelocity(ofVec3f(0, 100, 0));
	mama3->setLifespan(2000);
	mama3->setRate(1);
	mama3->target = player->trans;

	// Push mama emitters to list.
	enemy.push_back(mama1);
	enemy.push_back(mama2);
	enemy.push_back(mama3);

	// Set powerup image.
	power.image = shield;
	player->power = shield;

	// Set screen limit parameters.
	leftEdge = (player->width / 2);
	topEdge = (player->height / 2);
	rightEdge = ofGetWindowWidth() - (player->width / 2);
	bottomEdge = ofGetWindowHeight() - (player->height / 2);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (bGameStart && !bGameOver) {
		ofSeedRandom();
		// Start and update all enemy emitters.
		// Update target vector based on player position.
		for (MamaEmitter *e : enemy) {
			if (!e->started) { e->start(); }
			e->target = player->trans;
			e->update();
		}

		// Player presses (or holds) spacebar to fire
		if (bPlayerShoot) {
			if (!player->started) { player->start(); }
		}
		else { player->stop(); }

		// Movement based on player input using arrow keys.
		// Limitations on movement based on window size.
		keyMoveLimit();

		// Move powerup using physics.
		if (!power.hidden) { power.integrate(); }

		// Update player object/emitter.
		player->update();

		// Check collisions.
		// Check if player retrieved powerup.
		powerCollisions();
		
		// If player picked up powerup, is invincible for 15s.
		if (!player->hasPower) { playerCollisions(); }
		else {
			if (ofGetElapsedTimeMillis() - player->powertime >= 10000) {
				player->hasPower = false;
				power.hidden = false;
				power.reset();
			}
		}
		checkCollisions();
		if (exp.size() != 0) {
			for (Explosion *e : exp) {
				e->update();
				// Check if the explosion is complete, remove if it is.
				if (e->sys->debris.size() == 0) {
					exp.erase(remove(exp.begin(), exp.end(), e), exp.end());
					delete(e);
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Draw background, GUI, start message, and player.
	background.draw(0, 0, 375, 667);
	if (bShowGui) { gui.draw(); }

	// Draw based on whether game is started.
	if (bGameStart && !bGameOver) { 
		player->draw(); 
		if (!power.hidden) { power.draw(); }
		for (MamaEmitter *e : enemy) { e->draw(); }
		if (exp.size() != 0) {
			for (Explosion *e : exp) { e->draw(); }
		}
		ofSetColor(ofColor::white);
		ofDrawBitmapString("SCORE: " + ofToString(score), 0, 10);
		ofDrawBitmapString("LIVES: " + ofToString(lives), 0, 20);
	}
	else if (bGameOver) {
		ofDrawBitmapString("GAME OVER", (ofGetWindowWidth() - 78) / 2, ofGetWindowHeight() / 2);
		ofDrawBitmapString("SCORE: " + ofToString(score), (ofGetWindowWidth() - 78) / 2, (ofGetWindowHeight() / 2) + 20);
	}
	else { 
		title.setAnchorPoint(title.getWidth() / 2, title.getHeight() / 2);
		title.draw(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case OF_KEY_RETURN:
		if (!bGameStart) { bGameStart = true; }
		break;
	case 'h':
		if (!bShowGui) { bShowGui = true; }
		else { bShowGui = false; }
		break;
	case ' ':
		bPlayerShoot = true;
		break;
	case OF_KEY_LEFT:
		keys[MoveLeft] = true;
		break;
	case OF_KEY_RIGHT:
		keys[MoveRight] = true;
		break;
	case OF_KEY_DOWN:
		keys[MoveDown] = true;
		break;
	case OF_KEY_UP:
		keys[MoveUp] = true;
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	switch (key) {
	case ' ':
		bPlayerShoot = false;
		break;
	case OF_KEY_LEFT:
		keys[MoveLeft] = false;
		break;
	case OF_KEY_RIGHT:
		keys[MoveRight] = false;
		break;
	case OF_KEY_DOWN:
		keys[MoveDown] = false;
		break;
	case OF_KEY_UP:
		keys[MoveUp] = false;
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	ofPoint mouse_cur = ofPoint(x, y);
	ofVec3f delta = mouse_cur - mouse_last;
	// Move player based on mouse delta and use move limit function to stop on edges.
	player->trans += delta;
	mouseMoveLimit();
	mouse_last = mouse_cur;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	mouse_last = ofPoint(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
