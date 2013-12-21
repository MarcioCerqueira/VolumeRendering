#ifndef MYGLIMAGEVIEWER_H
#define MYGLIMAGEVIEWER_H

#include <GL/glew.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <math.h>
#include "VRParams.h"

class MyGLImageViewer
{
public:
	MyGLImageViewer();
	~MyGLImageViewer();
	void load1DTexture(unsigned char *data, GLuint *texVBO, int index, int width);
	void load2DTexture(unsigned char *data, GLuint *texVBO, int index, int width, int height);
	void load2DTextureDepthComponent(unsigned char *data, GLuint *texVBO, int index, int width, int height);
	void load2DDepthBufferTexture(GLuint *texVBO, int index, int x, int y, int width, int height);
	void load2DNoiseTexture(GLuint *texVBO, int index, int width, int height);
	void load3DTextureFromTIFFile(unsigned char *data, GLuint *texVBO, int index, int imageWidth, int imageHeight, int numberOfSlices, GLint filter);
	void loadDepthComponentTexture(unsigned short *data, GLuint *texVBO, int index, int windowWidth, int windowHeight);
	void loadRGBTexture(const unsigned char *data, GLuint *texVBO, int index, int windowWidth, int windowHeight);
	void loadFrameBufferTexture(GLuint *texVBO, int index, int x, int y, int width, int height);
	void draw1DTexture(GLuint *texVBO, int index, GLuint shaderProg, int windowWidth, int windowHeight, int textureID);
	void draw2DTexture(GLuint *texVBO, int index, GLuint shaderProg, int windowWidth, int windowHeight, int textureID);
	void draw3DTexture(GLuint *texVBO, int index, int octreeIndex, float *rot, float *trans, GLuint shaderProg, int windowWidth, int windowHeight, VRParams params, 
		int TFIndex = -1);
	void drawQuads(float x, float y, float z, GLenum target = GL_TEXTURE0);
	void drawShadowMapTexture(GLuint shaderProg, GLuint *texVBO, int index);
	void setShadowTextureMatrix();
	void readDepthBufferTexture(GLuint *texVBO, int index, int x, int y, int width, int height);
	void setStochasticJithering(bool sj) { this->stochasticJithering = sj; }
	void setTriCubicInterpolation(bool tci) { this->triCubicInterpolation = tci; }
private:
	bool stochasticJithering;
	bool triCubicInterpolation;
	float *auxDepthBuffer;
	unsigned char *depthBuffer;
};

#endif