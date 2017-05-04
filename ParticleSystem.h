#pragma once

#include "BVH.h"

#define MAX_PARTICLES 1000
#define WCX		640
#define WCY		480
#define RAIN	0
#define SNOW	1
#define	HAIL	2

class ParticleSystem{
public:

	typedef struct {
		// Life
		bool alive;	// is the particle alive?
		float life;	// particle lifespan
		float fade; // decay
					// color
		float red;
		float green;
		float blue;
		// Position/direction
		float xpos;
		float ypos;
		float zpos;
		// Velocity/Direction, only goes down in y dir
		float vel;
		// Gravity
		float gravity;
	}particles;

	particles par_sys[MAX_PARTICLES]; 
	BVH bvh;
	float slowdown = 2.0;
	float velocity = 0.0;
	// float zoom = 0.0;
	float pan = 0.0;
	float tilt = 0.0;
	float hailsize = 0.1;

	int loop;
	int fall;
	bool first = true;

	//floor colors
	float r = 0.0;
	float g = 1.0;
	float b = 0.0;
	float ground_points[200][200][3];
	float ground_colors[200][200][4];
	float accum = 0.0;

	void initParticles(int i);
	void initParticles_sys(const BVH & bvh);
	void drawRain();
	void drawHail();
	void drawSnow();
	void drawPlane();
	void arcballRotation(int endX, int endY);

private:
	

};
