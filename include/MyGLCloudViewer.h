#ifndef MYGLCLOUDVIEWER_H
#define MYGLCLOUDVIEWER_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include "VRParams.h"

class MyGLCloudViewer
{
public:
	MyGLCloudViewer();
	void configureAmbient(GLfloat *eye, GLfloat *at, GLfloat *up, VRParams params);
	void configureQuadAmbient(GLfloat *eye, GLfloat *at, GLfloat *up);
	void configureLight();
private:
	float diffuseIntensity;
	float globalAmbient;
};

#endif