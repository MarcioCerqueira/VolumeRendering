#include "MyGLCloudViewer.h"

static float angle = 0;

MyGLCloudViewer::MyGLCloudViewer() 
{
	diffuseIntensity = 0.35;
	globalAmbient = 0.35;
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

void MyGLCloudViewer::loadVBOQuad(GLuint *VBO, float x, float y, float z)
{

	float pointCloud[24];
	float color[24];
	int indices[36];

	pointCloud[0] = -x;	pointCloud[1] = y;	pointCloud[2] = z;
	pointCloud[3] = -x;	pointCloud[4] = -y;	pointCloud[5] = z;
	pointCloud[6] = x;	pointCloud[7] = -y;	pointCloud[8] = z;
	pointCloud[9] = x;	pointCloud[10] = y;	pointCloud[11] = z;

	pointCloud[12] = -x;	pointCloud[13] = y;	pointCloud[14] = -z;
	pointCloud[15] = -x;	pointCloud[16] = -y;	pointCloud[17] = -z;
	pointCloud[18] = x;	pointCloud[19] = -y;	pointCloud[20] = -z;
	pointCloud[21] = x;	pointCloud[22] = y;	pointCloud[23] = -z;

	color[0] = 0;	color[1] = 1;	color[2] = 1;
	color[3] = 0;	color[4] = 0;	color[5] = 1;
	color[6] = 1;	color[7] = 0;	color[8] = 1;
	color[9] = 1;	color[10] = 1;	color[11] = 1;

	color[12] = 0;	color[13] = 1;	color[14] = 0;
	color[15] = 0;	color[16] = 0;	color[17] = 0;
	color[18] = 1;	color[19] = 0;	color[20] = 0;
	color[21] = 1;	color[22] = 1;	color[23] = 0;
	
	indices[0] = 0;	indices[1] = 1;	indices[2] = 2;
	indices[3] = 0;	indices[4] = 2;	indices[5] = 3;
	indices[6] = 4;	indices[7] = 5;	indices[8] = 1;
	indices[9] = 4;	indices[10] = 1;	indices[11] = 0;
	indices[12] = 7;	indices[13] = 6;	indices[14] = 5;
	indices[15] = 7;	indices[16] = 5;	indices[17] = 4;
	indices[18] = 3;	indices[19] = 2;	indices[20] = 6;
	indices[21] = 3;	indices[22] = 6;	indices[23] = 7;
	indices[24] = 4;	indices[25] = 0;	indices[26] = 3;
	indices[27] = 4;	indices[28] = 3;	indices[29] = 7;
	indices[30] = 6;	indices[31] = 2;	indices[32] = 1;
	indices[33] = 6;	indices[34] = 1;	indices[35] = 5;
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glEnableClientState(GL_VERTEX_ARRAY);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), pointCloud, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glEnableClientState(GL_COLOR_ARRAY);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), color, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

	glDisableClientState(GL_VERTEX_ARRAY); 
	glDisableClientState(GL_COLOR_ARRAY); 

}

void MyGLCloudViewer::drawQuad(GLuint *VBO)
{

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]); 
	glEnableClientState(GL_VERTEX_ARRAY); 		
	glVertexPointer(3, GL_FLOAT, 0, 0); 

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]); 		
	glEnableClientState(GL_COLOR_ARRAY); 		
	glColorPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[2]); 
	
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY); 
	glDisableClientState(GL_COLOR_ARRAY); 

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 

}