#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Explosion.h"

// Modified by Michael Kang for CS134 Project 1.

typedef enum { MoveStop, MoveLeft, MoveRight, MoveUp, MoveDown } MoveDir;
typedef enum { Default, EnemyWave, EnemyLine } Path;

// This is a base object that all drawable object inherit from
// It is possible this will be replaced by ofNode when we move to 3D
//
class BaseObject {
public:
	BaseObject();

	void setPosition(glm::vec3 position);

	glm::mat4 getMatrix() {
		glm::mat4 m = glm::translate(glm::mat4(1.0), glm::vec3(trans));
		glm::mat4 M = glm::rotate(m, glm::radians(rot), glm::vec3(0, 0, 1));
		return M;
	}

	glm::vec3 trans, scale;
	float rot;
	bool bSelected;
};

// General Sprite class.  (similar to a Particle)
//
class Sprite : public BaseObject {
public:
	Sprite();

	void draw();
	void setImage(ofImage);
	float age();

	ofVec3f velocity; // in pixels/sec

	float speed;		// in pixels/sec
	float birthtime;	// elapsed time in ms
	float lifespan;		// time in ms
	float width, height;
	float rotate;

	string name;
	ofImage image;
	bool haveImage;
	bool toRotate;
};

// Manages all Sprites in a system.  You can create multiple systems.
//
class SpriteSystem {
public:
	SpriteSystem(Path p) { paths[p] = true; }
	SpriteSystem() { paths[Default] = true; }

	void add(Sprite);
	void remove(int);
	void update();
	void draw();
	bool removeNear(ofVec3f point, float dist);
	ofVec3f curveEval(float x, float y, float scale, float cycles, bool type);
	ofVec3f position;

	vector<Sprite> sprites;
	bool type = true;
	bool paths[3] = { false, false, false };
};


// General purpose Emitter class for emitting sprites.
//
class Emitter : public BaseObject {
public:
	Emitter(SpriteSystem *);
	Emitter() {}

	void draw();
	void start();
	void stop();
	void setLifespan(float);
	void setVelocity(ofVec3f);
	void setChildImage(ofImage);
	void setImage(ofImage);
	void setSound(ofSoundPlayer);
	void setRate(float);
	void setPathType(bool t) { sys->type = t; }
	void update(); 
	float maxDistPerFrame();
	float age();

	SpriteSystem *sys;
	ofVec3f velocity;
	float rate;
	float lifespan;
	float lastSpawned;
	float width, height;
	float birth, duration;
	float scale, cycle;
	float powertime;

	ofImage childImage;
	ofImage image;
	ofImage power;
	ofSoundPlayer soundEffect;

	bool initial;
	bool started;
	bool drawable;
	bool haveChildImage;
	bool haveImage;
	bool haveSound;
	bool isEnemy = false;
	bool hasPower = false;
};

// Essentially an emitter emitting emitters AKA MamaEmitter.
// Derived from Emitter, with some modified functions and extra ones to control
// rotation and movement.
class MamaEmitter : public Emitter {
public:
	MamaEmitter(Emitter *, Path p);

	void update();
	void move();
	void draw();
	void rotation();
	bool removeNear(ofVec3f point, float dist);
	ofVec3f sinWave(float x, float y, float scale, float cycles, bool type);
	ofVec3f triWave(float x, float y, float scale, float cycles, bool type);

	Emitter *emitter;
	vector<Emitter> emitters;

	// Data:
	glm::vec3 target;

	float fleet = 0;	// Tracking # of spawns to adjust for some randomness (temporary).
	float speed = 100;	// Speed of emitted emitters.
	bool type = true;
	bool paths[3] = { false, false, false };
};

// Power Up class that utilizes physics based movement to traverse the map
// at random directions and will bounce off the edges of screen.
class PowerUp : public BaseObject {
public:
	PowerUp();

	void integrate();
	void draw();
	void reset();
	float maxDistPerFrame();
	bool removeNear(ofVec3f p, float d);

	// Data:
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f heading;
	ofImage image;
	float damping;
	bool hidden;
};

class ofApp : public ofBaseApp {
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void checkCollisions();
		void playerCollisions();
		void powerCollisions();

		// Movement limitations.
		void keyMoveLimit();
		void mouseMoveLimit();
		
		// Player object.
		Emitter *player;
		Emitter *enemy1;
		MamaEmitter *mama1;
		MamaEmitter *mama2;
		MamaEmitter *mama3;
		vector<MamaEmitter *> enemy;

		// Powerup.
		PowerUp power;

		// Explosions.
		Explosion *hit;
		ImpulseRadialForce *radialForce = NULL;
		GravityForce *gravityForce = NULL;
		vector<Explosion *> exp;

		// Last mouse point stored and default fire direction.
		ofVec3f mouse_last;
		ofVec3f defaultDir;

		// Load images.
		ofImage ship;
		ofImage projectile;
		ofImage background;
		ofImage title;
		ofImage enemyShip;
		ofImage enemyProj;
		ofImage explosionImg;
		ofImage shield;

		ofSoundPlayer laserShot;
		ofSoundPlayer pop;
		ofSoundPlayer powerHit;
		ofSoundPlayer playerHit;

		// GUI.
		ofxPanel gui;
		ofxFloatSlider fireRate;
		ofxFloatSlider fireDir;
		
		// Flags for storing keys pressed and other states.
		bool keys[5];
		bool bPlayerShoot;
		bool bGameStart;
		bool bShowGui;
		bool bGameOver;

		// Store screen edges.
		float leftEdge;
		float rightEdge;
		float topEdge;
		float bottomEdge;
		float score = 0;
		int lives = 10;
};
