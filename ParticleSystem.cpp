#include "ParticleSystem.h"
#include <vecmath.h>
#include <cstdlib>

#ifdef WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

void ParticleSystem::initParticles(int i) {
	par_sys[i].alive = true;
	par_sys[i].life = 3.2;
	par_sys[i].fade = float(rand() % 10) / 1000.0f + 0.003f;

	par_sys[i].xpos = (float)(rand() % 200) - 100;
	par_sys[i].ypos = 100.0;
	par_sys[i].zpos = (float)(rand() % 200) - 100;

	par_sys[i].red = 0.5;
	par_sys[i].green = 0.5;
	par_sys[i].blue = 1.0;

	par_sys[i].vel = velocity;
	par_sys[i].gravity = -0.8;

}


void ParticleSystem::initParticles_sys(const BVH &bvh) {
	int x, z;
	this->bvh = bvh;

	glShadeModel(GL_SMOOTH);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);

	// Ground Verticies
	// Ground Colors
	for (z = 0; z < 200; z++) {
		for (x = 0; x < 200; x++) {
			ground_points[x][z][0] = x - 100.0;
			ground_points[x][z][1] = accum;
			ground_points[x][z][2] = z - 100.0;

			ground_colors[z][x][0] = r; // red value
			ground_colors[z][x][1] = g; // green value
			ground_colors[z][x][2] = b; // blue value
			ground_colors[z][x][3] = 0.0; // acummulation factor
		}
	}

	// Initialize particles
	for (loop = 0; loop < MAX_PARTICLES; loop++) {
		initParticles(loop);
	}
}

// For Rain
void ParticleSystem::drawRain() {
	float x, y, z;
	//pglDisable(GL_LIGHTING);
	for (loop = 0; loop < MAX_PARTICLES; loop = loop + 2) {
		if (par_sys[loop].alive == true) {
			x = par_sys[loop].xpos;
			y = par_sys[loop].ypos;
			z = par_sys[loop].zpos;

			// Draw particles
			glColor3f(0.5, 0.5, 1.0);
			glBegin(GL_LINES);
			glVertex3f(x, y, z);
			glVertex3f(x, y + 0.5, z);
			glEnd();

			// Update values
			//Move
			// Adjust slowdown for speed!
			par_sys[loop].ypos += par_sys[loop].vel / (slowdown * 1000);
			par_sys[loop].vel += par_sys[loop].gravity;
			// Decay
			par_sys[loop].life -= par_sys[loop].fade;

			if (par_sys[loop].ypos <= -10) {
				par_sys[loop].life = -1.0;
			}
			//Revive
			if (par_sys[loop].life < 0.0) {
				initParticles(loop);
			}
		}
	}
}

// For Hail
void ParticleSystem::drawHail() {
	float x, y, z;
	//glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);
	for (loop = 0; loop < MAX_PARTICLES; loop = loop + 2) {
		if (par_sys[loop].alive == true) {
			x = par_sys[loop].xpos;
			y = par_sys[loop].ypos;
			z = par_sys[loop].zpos;

			// Draw particles
			glColor3f(0.8, 0.8, 0.9);
			
			// Front
			glVertex3f(x - hailsize, y - hailsize, z + hailsize); // lower left
			glVertex3f(x - hailsize, y + hailsize, z + hailsize); // upper left
			glVertex3f(x + hailsize, y + hailsize, z + hailsize); // upper right
			glVertex3f(x + hailsize, y - hailsize, z + hailsize); // lower left
																  //Left
			glVertex3f(x - hailsize, y - hailsize, z + hailsize);
			glVertex3f(x - hailsize, y - hailsize, z - hailsize);
			glVertex3f(x - hailsize, y + hailsize, z - hailsize);
			glVertex3f(x - hailsize, y + hailsize, z + hailsize);
			// Back
			glVertex3f(x - hailsize, y - hailsize, z - hailsize);
			glVertex3f(x - hailsize, y + hailsize, z - hailsize);
			glVertex3f(x + hailsize, y + hailsize, z - hailsize);
			glVertex3f(x + hailsize, y - hailsize, z - hailsize);
			//Right
			glVertex3f(x + hailsize, y + hailsize, z + hailsize);
			glVertex3f(x + hailsize, y + hailsize, z - hailsize);
			glVertex3f(x + hailsize, y - hailsize, z - hailsize);
			glVertex3f(x + hailsize, y - hailsize, z + hailsize);
			//Top
			glVertex3f(x - hailsize, y + hailsize, z + hailsize);
			glVertex3f(x - hailsize, y + hailsize, z - hailsize);
			glVertex3f(x + hailsize, y + hailsize, z - hailsize);
			glVertex3f(x + hailsize, y + hailsize, z + hailsize);
			//Bottom
			glVertex3f(x - hailsize, y - hailsize, z + hailsize);
			glVertex3f(x - hailsize, y - hailsize, z - hailsize);
			glVertex3f(x + hailsize, y - hailsize, z - hailsize);
			glVertex3f(x + hailsize, y - hailsize, z + hailsize);
			

			// Update values
			//Move
			if (par_sys[loop].ypos <= -10) {
				par_sys[loop].vel = par_sys[loop].vel * -1.0;
			}
			par_sys[loop].ypos += par_sys[loop].vel / (slowdown * 1000); // * 1000
			par_sys[loop].vel += par_sys[loop].gravity;

			// Decay
			par_sys[loop].life -= par_sys[loop].fade;

			//Revive
			if (par_sys[loop].life < 0.0) {
				initParticles(loop);
			}
		}
	}
	glEnd();
}

// For Snow
void ParticleSystem::drawSnow() {
	
	glBegin(GL_POINTS);
	float x, y, z;
	for (loop = 0; loop < MAX_PARTICLES; loop = loop + 2) {
		if (par_sys[loop].alive == true) {
			x = par_sys[loop].xpos;
			y = par_sys[loop].ypos;
			z = par_sys[loop].zpos;

			// Draw particles
			glColor3f(1.0, 1.0, 1.0);
			glPushMatrix();
			glTranslatef(x, y, z);
			glutSolidSphere(1, 16, 16);
			glPopMatrix();


			// Update values
			//Move
			par_sys[loop].ypos += par_sys[loop].vel / (slowdown * 1000);
			par_sys[loop].vel += par_sys[loop].gravity;
			// Decay
			par_sys[loop].life -= par_sys[loop].fade;

			if (par_sys[loop].ypos <= 50.0 && !bvh.check_mesh_collide()) {
				bvh.init_mesh_collide();

			}
			//collision with floor
			if (par_sys[loop].ypos <= 0.0) {
				int zi = z + 100;
				int xi = x + 100;
				int range_zi_minus;
				int range_zi_plus;
				int range_xi_minus;
				int range_xi_plus;
				if (zi < 2) {
					range_zi_minus = 0;
				}
				else {
					range_zi_minus = zi - 3;
				}
				if (199 - zi < 3) {
					range_zi_plus = 199;
				}
				else {
					range_zi_plus = zi + 3;
				}
				if (xi < 2) {
					range_xi_minus = 0;
				}
				else {
					range_xi_minus = xi - 3;
				}
				if (199 - xi < 3) {
					range_xi_plus = 199;
				}
				else {
					range_xi_plus = xi + 3;
				}
				for (int zzi = range_zi_minus; zzi <= range_zi_plus; ++zzi) {
					for (int xxi = range_xi_minus; xxi <= range_xi_plus; ++xxi) {
						ground_colors[zzi][xxi][0] = 1.0;
						ground_colors[zzi][xxi][2] = 1.0;
						ground_colors[zzi][xxi][3] += 1.0;
						if (ground_colors[zzi][xxi][3] > 1.0) {
							ground_points[xxi][zzi][1] += 0.1;
						}
					}
				}
				par_sys[loop].life = -1.0;
			}

			//Revive
			if (par_sys[loop].life < 0.0) {
				initParticles(loop);
			}
		}
	}
	glEnd();
}

void ParticleSystem::drawPlane() {
	//glDisable(GL_CULL_FACE);
	glColor3f(r, g, b);
	glCullFace(GL_FRONT);

	glBegin(GL_QUADS);
	// along z - y const
	//glEnable(GL_LIGHTING);     // Enable lighting calculations
	//glEnable(GL_LIGHT0);       // Turn on light #0.
	//glMaterialf(GL_BACK, GL_SHININESS, 10.0f);

	for (int i = -100; i + 1 < 100; i++) {
		// along x - y const
		for (int j = -100; j + 1 < 100; j++) {
			
			//glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, ground_colors[i + 100][j + 100]);
			//glMaterialfv(GL_BACK, GL_SPECULAR, ground_colors[i + 100][j + 100]);
			glColor3fv(ground_colors[i + 100][j + 100]);
			glVertex3f(ground_points[j + 100][i + 100][0],
				ground_points[j + 100][i + 100][1],
				ground_points[j + 100][i + 100][2]);

			//glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, ground_colors[i + 100][j + 1 + 100]);
			//glMaterialfv(GL_BACK, GL_SPECULAR, ground_colors[i + 100][j + 1 + 100]);
			glColor3fv(ground_colors[i + 100][j + 1 + 100]);
			glVertex3f(ground_points[j + 1 + 100][i + 100][0],
				ground_points[j + 1 + 100][i + 100][1],
				ground_points[j + 1 + 100][i + 100][2]);

			//glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, ground_colors[i + 1 + 100][j + 1 + 100]);
			//glMaterialfv(GL_BACK, GL_SPECULAR, ground_colors[i + 1 + 100][j + 1 + 100]);
			glColor3fv(ground_colors[i + 1 + 100][j + 1 + 100]);
			glVertex3f(ground_points[j + 1 + 100][i + 1 + 100][0],
				ground_points[j + 1 + 100][i + 1 + 100][1],
				ground_points[j + 1 + 100][i + 1 + 100][2]);

			//glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, ground_colors[i + 1 + 100][j + 100]);
			//glMaterialfv(GL_BACK, GL_SPECULAR, ground_colors[i + 1 + 100][j + 100]);
			glColor3fv(ground_colors[i + 1 + 100][j + 100]);
			glVertex3f(ground_points[j + 100][i + 1 + 100][0],
				ground_points[j + 100][i + 1 + 100][1],
				ground_points[j + 100][i + 1 + 100][2]);

			Vector3f v1(ground_points[j + 100][i + 100][0], ground_points[j + 100][i + 100][1],ground_points[j + 1 + 100][i + 100][2] );
			Vector3f v2(ground_points[j + 1 + 100][i + 100][0],
				ground_points[j + 1 + 100][i + 100][1],
				ground_points[j + 1 + 100][i + 100][2]);
			Vector3f v3(ground_points[j + 100][i + 1 + 100][0],
				ground_points[j + 100][i + 1 + 100][1],
				ground_points[j + 100][i + 1 + 100][2]);

			//glNormal3fv(Vector3f::cross(-1*(v2 - v1), (v3 - v1)));
		}


	}
	glEnd();
	//glDisable(GL_LIGHTING);     // Enable lighting calculations
	//glDisable(GL_LIGHT0);       // Turn on light #0.
	glCullFace(GL_BACK);

	//glEnable(GL_CULL_FACE);
}