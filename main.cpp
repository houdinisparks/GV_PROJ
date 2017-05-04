#ifdef WIN32
#include <windows.h>
#endif

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <ctime>


#ifdef WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif
#include <vecmath.h>
#include "camera.h"
#include <string>
#include <BVH.h>
#include "ParticleSystem.h"

//#include "modelerapp.h"
//#include "ModelerView.h"
#define MAX_PARTICLES 1000
#define WCX		640
#define WCY		480
#define RAIN	0
#define SNOW	1

// toggle views
bool skeleOn = true;
int particleOn = 0; //toggle 0 - off, 1 - rain , 2 - snow



using namespace std;

namespace {

	//BVH IMPLEMENTATIONS
	BVH bvh;
	int cur_frame = 0;
	ParticleSystem particleSystem;

	Camera camera; // This is the camera

	bool g_mousePressed = false; // These are state variables for the UI

	// Declarations of functions whose implementations occur later.


	void keyboardFunc(unsigned char key, int x, int y);
	void specialFunc(int key, int x, int y);
	void mouseFunc(int button, int state, int x, int y);
	void motionFunc(int x, int y);
	void reshapeFunc(int w, int h);
	void drawScene(void);
	void initRendering();



	void keyboardFunc(unsigned char key, int x, int y)
	{
		switch (key)
		{
		case 27: // Escape key
			exit(0);
			break;
		case ' ':
		{
			Matrix4f eye = Matrix4f::identity();
			camera.SetRotation(eye);
			camera.SetCenter(Vector3f::ZERO);
			break;
		}

		case 's': //s key
		{
			skeleOn = !skeleOn;
			cout << "Skeleton: " << skeleOn;

			break;
		}
		case 'p':
		{
			particleOn += 1;


			if (particleOn %2 == 0) {
				particleOn = 0;
			}
			break;
		}


		default:
			cout << "Unhandled key press " << key << "." << endl;
		}

		glutPostRedisplay();
	}

	// This function is called whenever a "Special" key press is
	// received.  Right now, it's handling the arrow keys.
	void specialFunc(int key, int x, int y)
	{
		switch (key)
		{

		}
		//glutPostRedisplay();
	}

	//  Called when mouse button is pressed.
	void mouseFunc(int button, int state, int x, int y)
	{
		if (state == GLUT_DOWN)
		{
			g_mousePressed = true;

			switch (button)
			{
			case GLUT_LEFT_BUTTON:
				camera.MouseClick(Camera::LEFT, x, y);
				break;
			case GLUT_MIDDLE_BUTTON:
				camera.MouseClick(Camera::MIDDLE, x, y);
				break;
			case GLUT_RIGHT_BUTTON:
				camera.MouseClick(Camera::RIGHT, x, y);
			default:
				break;
			}
		}
		else
		{
			camera.MouseRelease(x, y);
			g_mousePressed = false;
		}
		glutPostRedisplay();
	}

	// Called when mouse is moved while button pressed.
	void motionFunc(int x, int y)
	{
		camera.MouseDrag(x, y);

		glutPostRedisplay();
	}

	// Called when the window is resized
	// w, h - width and height of the window in pixels.
	void reshapeFunc(int w, int h)
	{
		camera.SetDimensions(w, h);

		camera.SetViewport(0, 0, w, h);
		camera.ApplyViewport();

		// Set up a perspective view, with square aspect ratio
		glMatrixMode(GL_PROJECTION);

		camera.SetPerspective(50);
		glLoadMatrixf(camera.projectionMatrix());
	}

	// Initialize OpenGL's rendering modes
	void initRendering()
	{

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// This function is responsible for displaying the object.
	// it is called everytime whenever the window needs redrawing
	void drawScene(void)
	{
		// Clear the rendering window
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Light color (RGBA)
		GLfloat Lt0diff[] = { 1.0,1.0,1.0,1.0 };
		GLfloat Lt0pos[] = { 3.0,3.0,5.0,1.0 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
		glLightfv(GL_LIGHT0, GL_POSITION, Lt0pos);

		glLoadMatrixf(camera.viewMatrix());

		//// THIS IS WHERE THE DRAW CODE GOES

		particleSystem.drawPlane();


		// This draws the coordinate axes when you're rotating, to
		// keep yourself oriented.
		if (g_mousePressed)
		{
			glPushMatrix();
			Vector3f eye = camera.GetCenter();
			glTranslatef(eye[0], eye[1], eye[2]);

			// Save current state of OpenGL
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			// This is to draw the axes when the mouse button is down
			glDisable(GL_LIGHTING);
			glLineWidth(3);
			glPushMatrix();
			glScaled(50.0, 50.0, 50.0);
			glBegin(GL_LINES);
			glColor4f(1, 0.5, 0.5, 1); glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);
			glColor4f(0.5, 1, 0.5, 1); glVertex3f(0, 0, 0); glVertex3f(0, 1, 0);
			glColor4f(0.5, 0.5, 1, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 1);

			glColor4f(0.5, 0.5, 0.5, 1);
			glVertex3f(0, 0, 0); glVertex3f(-1, 0, 0);
			glVertex3f(0, 0, 0); glVertex3f(0, -1, 0);
			glVertex3f(0, 0, 0); glVertex3f(0, 0, -1);

			glEnd();
			glPopMatrix();

			glPopAttrib();
			glPopMatrix();


			if (skeleOn) {
				bvh.drawSkeleton(true, cur_frame);

			}
			else {
				bvh.drawMesh(true, cur_frame);

			}
		}
		else {

			if (skeleOn) {

;				bvh.drawSkeleton(true, cur_frame++);

			}
			else {
				bvh.drawMesh(true, cur_frame++);

			}
		}

        if (particleOn == 1) {


			particleSystem.drawSnow(bvh);
		}


		// Dump the image to the screen.
		glutSwapBuffers();
	}

	void timerFunc(int t)
	{

		glutPostRedisplay();

		glutTimerFunc(t, &timerFunc, t);
	}



}

int main( int argc, char* argv[] )
{
	if( argc < 2 )
	{
		cout << "Usage: " << argv[ 0 ] << " PREFIX" << endl;
		cout <<  "Please input a .bhv file to load" << endl;
		return -1;
	}

	glutInit(&argc, argv);

	// We're going to animate it, so double buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Initial parameters for window position and size
	glutInitWindowPosition(60, 60);
	glutInitWindowSize(600, 600);

	camera.SetDimensions(600, 600);

	camera.SetDistance(300);
	camera.SetCenter(Vector3f::ZERO);

	glutCreateWindow("GV PROJECT");

	// Initialize OpenGL parameters.
	initRendering();
    particleSystem.initParticles_sys();

	// Setup BVH
	//initSystem(argc, argv);gv
	bvh.load(argv[1], argv[2] , argv[3]);
	//bvh.loadBVH(argv[1]);
	//bvh.loadMesh(argv[1]);
	//bvh.loadAttachments(argv[3]);
	//bvh.testOutput();

	// Set up callback functions for key presses
	glutKeyboardFunc(keyboardFunc); // Handles "normal" ascii symbols
	glutSpecialFunc(specialFunc);   // Handles "special" keyboard keys

									// Set up callback functions for mouse
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);

	// Set up the callback function for resizing windows
	glutReshapeFunc(reshapeFunc);

	// Call this whenever window needs redrawing
	glutDisplayFunc(drawScene);

	// Trigger timerFunc every 8.33 msec / 1 frame every 8.33ms
	glutTimerFunc(2, timerFunc, 2);

	// Start the main loop.  glutMainLoop never returns.
	glutMainLoop();

    return 0;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
//STASH OF OLD CODES HERE!

// Initialize the controls.  You have to define a ModelerControl
// for every variable name that you define in the enumeration.

// The constructor for a ModelerControl takes the following arguments:
// - text label in user interface
// - minimum slider value
// - maximum slider value
// - step size for slider
// - initial slider value

//const int NUM_JOINTS = 18;


//REMOVED 1) MODELERCONTROL

/*ModelerControl controls[ NUM_JOINTS*3 ];
string jointNames[NUM_JOINTS]={ "Root", "Chest", "Waist", "Neck", "Right hip", "Right leg", "Right knee", "Right foot", "Left hip", "Left leg", "Left knee", "Left foot", "Right collarbone", "Right shoulder", "Right elbow", "Left collarbone", "Left shoulder", "Left elbow" };*/

//for(unsigned int i = 0; i < NUM_JOINTS; i++)
//{
//	char buf[255];
//	sprintf(buf, "%s X", jointNames[i].c_str());
//	controls[i*3] = ModelerControl(buf, -M_PI, M_PI, 0.1f, 0);
//	sprintf(buf, "%s Y", jointNames[i].c_str());
//	controls[i*3+1] = ModelerControl(buf, -M_PI, M_PI, 0.1f, 0);
//	sprintf(buf, "%s Z", jointNames[i].c_str());
//	controls[i*3+2] = ModelerControl(buf, -M_PI, M_PI, 0.1f, 0);
//}

//   ModelerApplication::Instance()->Init
//(
//	argc, argv,
//	controls,
//	NUM_JOINTS*3
//);

//   // Run the modeler application.
//   int ret = ModelerApplication::Instance()->Run();

//   // This line is reached when you close the program.
//   delete ModelerApplication::Instance();
