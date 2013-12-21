#include "MyGLCloudViewer.h"

static float angle = 0;

MyGLCloudViewer::MyGLCloudViewer() 
{
	diffuseIntensity = 0.2;
	globalAmbient = 0.2;
}

void MyGLCloudViewer::configureAmbient(GLfloat *eye, GLfloat *at, GLfloat *up, VRParams params) 
{
	
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    gluPerspective(60.0, (GLfloat)640.f/480.f, 0.5, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0], eye[1], eye[2], at[0], at[1], at[2], up[0], up[1], up[2]);
    
	glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.1f );
	
	glEnable(GL_BLEND);
	if(!params.MIP) {
		glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	} else {
		glBlendEquationEXT(GL_MAX_EXT);
		glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
	}

	glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

	glEnable(GL_LIGHTING);
	// setup light
	float lightdir[3];
	lightdir[0] = 0;
	lightdir[1] = 0;
	lightdir[2] = -1;

	float lightPosition[3];
	lightPosition[0] = -30 * cosf(90);
	lightPosition[1] = -20;
	lightPosition[2] = -30 * sinf(90);
	//lightPosition[0] = -30 * cosf(angle);
	//lightPosition[1] = -20;
	//lightPosition[2] = -30 * sinf(angle);

	angle += 0.01;
	GLfloat light0_position[] = { lightPosition[0], lightPosition[1], lightPosition[2], 0 };
	GLfloat light1_position[] = { -lightPosition[0], -lightPosition[1], -lightPosition[2], 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	configureLight();

}

void MyGLCloudViewer::configureQuadAmbient(GLfloat *eye, GLfloat *at, GLfloat *up) 
{
	
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    gluPerspective(60.0, (GLfloat)640.f/480.f, 0.5, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0], eye[1], eye[2], at[0], at[1], at[2], up[0], up[1], up[2]);
    
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	
}

void MyGLCloudViewer::configureLight() 
{
	// set light parameters
	//isosurf
	GLfloat mat_specular[4] = { 0.18f, 0.18f, 0.18f, 1.f };
	GLfloat mat_shininess[] = { 64.f };
	//0.05f
	GLfloat global_ambient[] = { globalAmbient, globalAmbient, globalAmbient, 1.f };
	
	GLfloat light0_ambient[] = { 0.5f, 0.5f, 0.5f, 1.f };
	GLfloat light0_diffuse[] = { diffuseIntensity, diffuseIntensity, diffuseIntensity, 1.f };
	GLfloat light0_specular[] = { 0.5f, 0.5f, 0.5f, 1.f };

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, light0_diffuse);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	
	glLightfv(GL_LIGHT1, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light0_specular);
	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);// todo include this into spotlight node

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

}