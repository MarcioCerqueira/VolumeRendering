#include "MyGLImageViewer.h"

MyGLImageViewer::MyGLImageViewer() {
	auxDepthBuffer = (float*) malloc(640 * 480 * sizeof(float));
	depthBuffer = (unsigned char*) malloc(640 * 480 * 3 * sizeof(unsigned char));
}

MyGLImageViewer::~MyGLImageViewer() {

	delete [] auxDepthBuffer;
	delete [] depthBuffer;

}

void MyGLImageViewer::load1DTexture(unsigned char* data, GLuint *texVBO, int index, int width) {

	glBindTexture(GL_TEXTURE_1D, texVBO[index]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

}

void MyGLImageViewer::load2DTexture(unsigned char *data, GLuint *texVBO, int index, int width, int height) {
	
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

}

void MyGLImageViewer::load2DNoiseTexture(GLuint *texVBO, int index, int width, int height) {
	
	unsigned char *noise = (unsigned char*)malloc(width * height * sizeof(unsigned char));

	srand((unsigned)time(NULL));
	for(int pixel = 0; pixel < (width * height); pixel++) {
		noise[pixel] = 255.f * rand()/(float)RAND_MAX;
		//noise[pixel] = rand() % 128;
		//printf("%f\n", noise[pixel]);
		//system("pause");
	}

	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, noise);

	delete [] noise;
}

void MyGLImageViewer::load2DTextureDepthComponent(unsigned char *data, GLuint *texVBO, int index, int width, int height) {
	
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);

}

void MyGLImageViewer::load2DDepthBufferTexture(GLuint *texVBO, int index, int x, int y, int width, int height)
{

	//glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, depthBuffer);
	glReadPixels(x, y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, auxDepthBuffer);
	
	/*
	int xp, yp, inversePixel;
	for(int pixel = 0; pixel < (640 * 480); pixel++) {
		xp = pixel % 640;
		yp = pixel / 640;
		inversePixel = (480 - yp) * 640 + xp;
		auxDepthBuffer[inversePixel] = (auxDepthBuffer[inversePixel] - 0.999) * 1000;
		if(auxDepthBuffer[inversePixel] != 0 && auxDepthBuffer[inversePixel] != 1)
			printf("%f\n", auxDepthBuffer[inversePixel]);
		//depthBuffer[pixel * 3 + 0] = 255 - auxDepthBuffer[inversePixel] * 255;
		//depthBuffer[pixel * 3 + 1] = 255 - auxDepthBuffer[inversePixel] * 255;
		//depthBuffer[pixel * 3 + 2] = 255 - auxDepthBuffer[inversePixel] * 255;
	}
	*/
	/*
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,  
		0, GL_RGB, GL_UNSIGNED_BYTE, depthBuffer);
	*/
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, auxDepthBuffer);
}

void MyGLImageViewer::load3DTextureFromTIFFile(unsigned char *data, GLuint *texVBO, int index, int imageWidth, int imageHeight, int numberOfSlices, GLint filter) {

	glBindTexture(GL_TEXTURE_3D, texVBO[index]);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, imageWidth, imageHeight, numberOfSlices, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
}

	
void MyGLImageViewer::loadDepthComponentTexture(float *data, GLuint *texVBO, int index, int windowWidth, int windowHeight)
{

	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);

}

void MyGLImageViewer::loadRGBTexture(const unsigned char *data, GLuint *texVBO, int index, int windowWidth, int windowHeight)
{

	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

}

void MyGLImageViewer::draw1DTexture(GLuint *texVBO, int index, GLuint shaderProg, int windowWidth, int windowHeight, int textureID) {

	//glUseProgram(shaderProg);

	glMatrixMode(GL_PROJECTION);          
	glLoadIdentity(); 
		
	gluOrtho2D( 0, windowWidth/2, windowHeight/2, 0 ); 
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	GLuint texLoc = glGetUniformLocation(shaderProg, "transferFunction");
	glUniform1i(texLoc, textureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, texVBO[index]);
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex2f(windowWidth/2, 0.0f);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex2f(windowWidth/2, windowHeight/2);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex2f(0.0f, windowHeight/2);
	glEnd();

	//glUseProgram(0);

}

void MyGLImageViewer::draw2DTexture(GLuint *texVBO, int index, GLuint shaderProg, int windowWidth, int windowHeight, int textureID) {

	glUseProgram(shaderProg);

	glMatrixMode(GL_PROJECTION);          
	glLoadIdentity(); 
		
	gluOrtho2D( 0, windowWidth/2, windowHeight/2, 0 ); 
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	GLuint texLoc = glGetUniformLocation(shaderProg, "image");
	glUniform1i(texLoc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex2f(windowWidth/2, 0.0f);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex2f(windowWidth/2, windowHeight/2);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex2f(0.0f, windowHeight/2);
	glEnd();
	
	glUseProgram(0);

}


void MyGLImageViewer::draw3DTexture(GLuint *texVBO, float *rot, float *trans, GLuint shaderProg, int windowWidth, int windowHeight, VRParams params) {

	glUseProgram(shaderProg);

	GLuint texLoc = glGetUniformLocation(shaderProg, "volume");
	glUniform1i(texLoc, 7);

	glActiveTexture(GL_TEXTURE7);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, texVBO[params.volumeTextureIndex]);

	glActiveTexture(GL_TEXTURE7);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	texLoc = glGetUniformLocation(shaderProg, "minMaxOctree");
	glUniform1i(texLoc, 2);

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, texVBO[params.minMaxOctreeTextureIndex]);
	
	if(params.stepSize >= 0) {

		GLuint texLoc = glGetUniformLocation(shaderProg, "stepSize");
		glUniform1f(texLoc, params.stepSize);

		texLoc = glGetUniformLocation(shaderProg, "earlyRayTerminationThreshold");
		glUniform1f(texLoc, params.earlyRayTerminationThreshold);

		texLoc = glGetUniformLocation(shaderProg, "camera");
		glUniform3f(texLoc, 0, 0, 3); 

		texLoc = glGetUniformLocation(shaderProg, "kt");
		glUniform1f(texLoc, params.kt);

		texLoc = glGetUniformLocation(shaderProg, "ks");
		glUniform1f(texLoc, params.ks);

		if(params.stochasticJithering) {
			texLoc = glGetUniformLocation(shaderProg, "stochasticJithering");
			glUniform1i(texLoc, 1);
		} else {
			texLoc = glGetUniformLocation(shaderProg, "stochasticJithering");
			glUniform1i(texLoc, 0);
		}

		if(params.triCubicInterpolation) {
			texLoc = glGetUniformLocation(shaderProg, "triCubicInterpolation");
			glUniform1i(texLoc, 1);
		} else {
			texLoc = glGetUniformLocation(shaderProg, "triCubicInterpolation");
			glUniform1i(texLoc, 0);
		}

		if(params.MIP) {
			texLoc = glGetUniformLocation(shaderProg, "MIP");
			glUniform1i(texLoc, 1);
		} else {
			texLoc = glGetUniformLocation(shaderProg, "MIP");
			glUniform1i(texLoc, 0);
		}

		if(params.gradientByForwardDifferences) {
			texLoc = glGetUniformLocation(shaderProg, "forwardDifference");
			glUniform1i(texLoc, 1);
		} else { 
			texLoc = glGetUniformLocation(shaderProg, "forwardDifference");
			glUniform1i(texLoc, 0);
		}

		if(params.FCVisualization) {
			texLoc = glGetUniformLocation(shaderProg, "FCVisualization");
			glUniform1i(texLoc, 1);
		} else {
			texLoc = glGetUniformLocation(shaderProg, "FCVisualization");
			glUniform1i(texLoc, 0);
		}

		texLoc = glGetUniformLocation(shaderProg, "clippingPlane");
		glUniform1i(texLoc, (int)params.clippingPlane);

		texLoc = glGetUniformLocation(shaderProg, "inverseClipping");
		glUniform1i(texLoc, (int)params.inverseClipping);

		texLoc = glGetUniformLocation(shaderProg, "clippingOcclusion");
		glUniform1i(texLoc, (int)params.clippingOcclusion);

		texLoc = glGetUniformLocation(shaderProg, "clippingPlaneLeftX");
		glUniform1f(texLoc, params.clippingPlaneLeftX);
		
		texLoc = glGetUniformLocation(shaderProg, "clippingPlaneRightX");
		glUniform1f(texLoc, params.clippingPlaneRightX);

		texLoc = glGetUniformLocation(shaderProg, "clippingPlaneUpY");
		glUniform1f(texLoc, params.clippingPlaneUpY);
		
		texLoc = glGetUniformLocation(shaderProg, "clippingPlaneDownY");
		glUniform1f(texLoc, params.clippingPlaneDownY);
		
		texLoc = glGetUniformLocation(shaderProg, "clippingPlaneFrontZ");
		glUniform1f(texLoc, params.clippingPlaneFrontZ);
		
		texLoc = glGetUniformLocation(shaderProg, "clippingPlaneBackZ");
		glUniform1f(texLoc, params.clippingPlaneBackZ);

		texLoc = glGetUniformLocation(shaderProg, "isosurfaceThreshold");
		glUniform1f(texLoc, params.isoSurfaceThreshold);

		texLoc = glGetUniformLocation(shaderProg, "windowWidth");
		glUniform1i(texLoc, windowWidth);

		texLoc = glGetUniformLocation(shaderProg, "windowHeight");
		glUniform1i(texLoc, windowHeight);

	}
	
	drawQuads(1.0f/params.scaleWidth, 1.0f/params.scaleHeight, 1.0f/params.scaleDepth);
	
	texLoc = glGetUniformLocation(shaderProg, "transferFunction");
	glUniform1i(texLoc, 1);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.transferFunctionTextureIndex]);
	
	texLoc = glGetUniformLocation(shaderProg, "noise");
	glUniform1i(texLoc, 3);

	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.noiseTextureIndex]);
	
	
	texLoc = glGetUniformLocation(shaderProg, "backFrameBuffer");
	glUniform1i(texLoc, 5);

	glActiveTexture(GL_TEXTURE5);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.backQuadTextureIndex]);
	
	texLoc = glGetUniformLocation(shaderProg, "frontFrameBuffer");
	glUniform1i(texLoc, 6);

	glActiveTexture(GL_TEXTURE6);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.frontQuadTextureIndex]);
	
	if(params.FCVisualization) {
		
		texLoc = glGetUniformLocation(shaderProg, "positionBuffer");
		glUniform1i(texLoc, 8);

		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texVBO[params.positionTextureIndex]);
		
		texLoc = glGetUniformLocation(shaderProg, "normalBuffer");
		glUniform1i(texLoc, 9);

		glActiveTexture(GL_TEXTURE9);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texVBO[params.normalTextureIndex]);
	
	}

	glUseProgram(0);
	
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_3D);
	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE5);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE6);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE7);
	glDisable(GL_TEXTURE_3D);
	if(params.FCVisualization) {
		glActiveTexture(GL_TEXTURE8);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE9);
		glDisable(GL_TEXTURE_2D);
	}

}

void MyGLImageViewer::drawFCVisualization(GLuint *texVBO, GLuint shaderProg, int windowWidth, int windowHeight, VRParams params) {

	glUseProgram(shaderProg);

	drawQuads(1.0f/params.scaleWidth, 1.0f/params.scaleHeight, 1.0f/params.scaleDepth);
	
	GLuint texLoc = glGetUniformLocation(shaderProg, "position");
	glUniform1i(texLoc, 8);

	glActiveTexture(GL_TEXTURE8);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.positionTextureIndex]);
		
	texLoc = glGetUniformLocation(shaderProg, "normal");
	glUniform1i(texLoc, 9);

	glActiveTexture(GL_TEXTURE9);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.normalTextureIndex]);
	
	texLoc = glGetUniformLocation(shaderProg, "curvature");
	glUniform1i(texLoc, 10);

	glActiveTexture(GL_TEXTURE10);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.curvatureTextureIndex]);
	
	texLoc = glGetUniformLocation(shaderProg, "focusLayer");
	glUniform1i(texLoc, 11);

	glActiveTexture(GL_TEXTURE11);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.focusTextureIndex]);

	texLoc = glGetUniformLocation(shaderProg, "contextLayer");
	glUniform1i(texLoc, 12);

	glActiveTexture(GL_TEXTURE12);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texVBO[params.contextTextureIndex]);

	texLoc = glGetUniformLocation(shaderProg, "focusPoint");
	glUniform2f(texLoc, params.focusPoint[0], params.focusPoint[1]);
	texLoc = glGetUniformLocation(shaderProg, "focusRadius");
	glUniform1f(texLoc, params.focusRadius);
	texLoc = glGetUniformLocation(shaderProg, "windowWidth");
	glUniform1i(texLoc, windowWidth);
	texLoc = glGetUniformLocation(shaderProg, "windowHeight");
	glUniform1i(texLoc, windowHeight);

	glUseProgram(0);
	
	glActiveTexture(GL_TEXTURE8);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE9);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE10);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE11);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE12);
	glDisable(GL_TEXTURE_2D);

}

void MyGLImageViewer::setShadowTextureMatrix() {
	
	static double modelView[16];
	static double projection[16];
	
	// This is matrix transform every coordinate x,y,z
	// x = x* 0.5 + 0.5 
	// y = y* 0.5 + 0.5 
	// z = z* 0.5 + 0.5 
	// Moving from unit cube [-1,1] to [0,1]  
	const GLdouble bias[16] = {	
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0};
	
	// Grab modelview and transformation matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE6);
	
	glLoadIdentity();	
	glLoadMatrixd(bias);
	
	// concatating all matrice into one.
	glMultMatrixd (projection);
	glMultMatrixd (modelView);
	
	// Go back to normal matrix mode
	glMatrixMode(GL_MODELVIEW);

}

void MyGLImageViewer::readDepthBufferTexture(GLuint *texVBO, int index, int x, int y, int width, int height)
{
	//Read the depth buffer into the shadow map texture
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, width, height);
}

void MyGLImageViewer::loadFrameBufferTexture(GLuint *texVBO, int index, int x, int y, int width, int height)
{

	glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, depthBuffer);
	
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, depthBuffer);


}
void MyGLImageViewer::drawShadowMapTexture(GLuint shaderProg, GLuint *texVBO, int index)
{
	glUseProgram(shaderProg);
	
	GLuint texLoc = glGetUniformLocation(shaderProg, "ShadowMap");
	glUniform1i(texLoc, 0);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texVBO[index]);
	
}

void MyGLImageViewer::drawQuads(float x, float y, float z, GLenum target) {
	
	bool color = true;
	glBegin(GL_QUADS);
	//front
	if(color) glColor3f(0, 1, 1);
	glMultiTexCoord3f(target, 0.0f, 1.0f, 1.0f);
	glVertex3f(-x, y, z);	//0
	if(color) glColor3f(0, 0, 1);
	glMultiTexCoord3f(target, 0.0f, 0.0f, 1.0f);
	glVertex3f(-x, -y, z);	//1
	if(color) glColor3f(1, 0, 1);
	glMultiTexCoord3f(target, 1.0f, 0.0f, 1.0f);
	glVertex3f(x, -y, z);	//2
	if(color) glColor3f(1, 1, 1);
	glMultiTexCoord3f(target, 1.0f, 1.0f, 1.0f);
	glVertex3f(x, y, z);	//3
	
	//left
	if(color) glColor3f(0, 1, 0);
	glMultiTexCoord3f(target, 0.0f, 1.0f, 0.0f);
	glVertex3f(-x, y, -z);	//4
	if(color) glColor3f(0, 0, 0);
	glMultiTexCoord3f(target, 0.0f, 0.0f, 0.0f);
	glVertex3f(-x, -y, -z);	//5
	if(color) glColor3f(0, 0, 1);
	glMultiTexCoord3f(target, 0.0f, 0.0f, 1.0f);
	glVertex3f(-x, -y, z);	//1
	if(color) glColor3f(0, 1, 1);
	glMultiTexCoord3f(target, 0.0f, 1.0f, 1.0f);
	glVertex3f(-x, y, z);	//0

	//back
	if(color) glColor3f(1, 1, 0);
	glMultiTexCoord3f(target, 1.0f, 1.0f, 0.0f);
	glVertex3f(x, y, -z);	//7
	if(color) glColor3f(1, 0, 0);
	glMultiTexCoord3f(target, 1.0f, 0.0f, 0.0f);
	glVertex3f(x, -y, -z);	//6
	if(color) glColor3f(0, 0, 0);
	glMultiTexCoord3f(target, 0.0f, 0.0f, 0.0f);
	glVertex3f(-x, -y, -z);	//5
	if(color) glColor3f(0, 1, 0);
	glMultiTexCoord3f(target, 0.0f, 1.0f, 0.0f);
	glVertex3f(-x, y, -z);	//4

	//right
	if(color) glColor3f(1, 1, 1);
	glMultiTexCoord3f(target, 1.0f, 1.0f, 1.0f);
	glVertex3f(x, y, z);	//3
	if(color) glColor3f(1, 0, 1);
	glMultiTexCoord3f(target, 1.0f, 0.0f, 1.0f);
	glVertex3f(x, -y, z);	//2
	if(color) glColor3f(1, 0, 0);
	glMultiTexCoord3f(target, 1.0f, 0.0f, 0.0f);
	glVertex3f(x, -y, -z);	//6
	if(color) glColor3f(1, 1, 0);
	glMultiTexCoord3f(target, 1.0f, 1.0f, 0.0f);
	glVertex3f(x, y, -z);	//7

	//top
	if(color) glColor3f(0, 1, 0);
	glMultiTexCoord3f(target, 0.0f, 1.0f, 0.0f);
	glVertex3f(-x, y, -z);	//4
	if(color) glColor3f(0, 1, 1);
	glMultiTexCoord3f(target, 0.0f, 1.0f, 1.0f);
	glVertex3f(-x, y, z);	//0
	if(color) glColor3f(1, 1, 1);
	glMultiTexCoord3f(target, 1.0f, 1.0f, 1.0f);
	glVertex3f(x, y, z);	//3
	if(color) glColor3f(1, 1, 0);
	glMultiTexCoord3f(target, 1.0f, 1.0f, 0.0f);
	glVertex3f(x, y, -z);	//7

	//bottom
	if(color) glColor3f(1, 0, 0);
	glMultiTexCoord3f(target, 1.0f, 0.0f, 0.0f);
	glVertex3f(x, -y, -z);	//6
	if(color) glColor3f(1, 0, 1);
	glMultiTexCoord3f(target, 1.0f, 0.0f, 1.0f);
	glVertex3f(x, -y, z);	//2
	if(color) glColor3f(0, 0, 1);
	glMultiTexCoord3f(target, 0.0f, 0.0f, 1.0f);
	glVertex3f(-x, -y, z);	//1
	if(color) glColor3f(0, 0, 0);
	glMultiTexCoord3f(target, 0.0f, 0.0f, 0.0f);
	glVertex3f(-x, -y, -z);	//5
	
	glEnd();

}
