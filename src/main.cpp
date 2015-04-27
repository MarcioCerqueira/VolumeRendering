#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/glut.h>
#include "Volume.h"
#include "MinMaxOctree.h"
#include "MyGLImageViewer.h"
#include "MyGLCloudViewer.h"
#include "shader.h"
#include "TransferFunction.h"
#include "TriCubicInterpolationPreFilter.h"
#include "VRParams.h"

int windowWidth = 640;
int windowHeight = 480;

char path[1000];
int firstSlice, lastSlice;
char pathExtension[3];

bool rotation = false;
bool translation = false;
bool focusRadiusOn = false;
bool earlyRayTermination = false;
bool stepSizeModification = false;
bool isosurfaceThresholdModification = false;
bool ks = false;
bool kt = false;
bool automatic = false;

bool clippingPlaneLeftXOn = false;
bool clippingPlaneRightXOn = false;
bool clippingPlaneUpYOn = false;
bool clippingPlaneDownYOn = false;
bool clippingPlaneFrontZOn = false;
bool clippingPlaneBackZOn = false;

//180 -90
float rot[3] = {0, 0, 0};
float trans[3] = {0, 0, -5};

int scale = 1;
int vel = 1;

GLfloat eye[3] = { 0.0, 0.0, 3.0 };
//GLfloat eye[3] = {-30 * cosf(90), -20, -30 * cosf(90)};
GLfloat at[3]  = { 0.0, 0.0, 0.0 };
GLfloat up[3]  = { 0.0, -1.0, 1.0 };

Volume *volume;
MinMaxOctree *minMaxOctree;
MyGLImageViewer *myGLImageViewer;
MyGLCloudViewer *myGLCloudViewer;
TransferFunction *transferFunction;
TriCubicInterpolationPreFilter *triCubicInterpolationPreFilter;
VRParams vrparams;

GLuint texVBO[20];
GLuint quadVBO[4];

GLuint frontQuadFrameBuffer;
GLuint backQuadFrameBuffer;
GLuint positionFrameBuffer;
GLuint normalFrameBuffer;
GLuint curvatureFrameBuffer;
GLuint focusFrameBuffer;
GLuint contextFrameBuffer;

enum
{
	VOLUME_TEXTURE = 0,
	TRANSFER_FUNCTION_TEXTURE = 1,
	MIN_MAX_OCTREE_TEXTURE = 2,
	NOISE_TEXTURE = 3, 
	BACK_QUAD_FRAMEBUFFER_COLOR_TEXTURE = 4,
	BACK_QUAD_FRAMEBUFFER_DEPTH_TEXTURE = 5,
	FRONT_QUAD_FRAMEBUFFER_COLOR_TEXTURE = 6,
	FRONT_QUAD_FRAMEBUFFER_DEPTH_TEXTURE = 7,
	POSITION_FRAMEBUFFER_COLOR_TEXTURE = 8,
	POSITION_FRAMEBUFFER_DEPTH_TEXTURE = 9,
	NORMAL_FRAMEBUFFER_COLOR_TEXTURE = 10,
	NORMAL_FRAMEBUFFER_DEPTH_TEXTURE = 11,
	CURVATURE_FRAMEBUFFER_COLOR_TEXTURE = 12,
	CURVATURE_FRAMEBUFFER_DEPTH_TEXTURE = 13,
	FOCUS_FRAMEBUFFER_COLOR_TEXTURE = 14,
	FOCUS_FRAMEBUFFER_DEPTH_TEXTURE = 15,
	CONTEXT_FRAMEBUFFER_COLOR_TEXTURE = 16,
	CONTEXT_FRAMEBUFFER_DEPTH_TEXTURE = 17,
};

enum
{
	VOLUME_RENDERING_SHADER = 0,
	CONTEXT_PRESERVING_VOLUME_RENDERING_SHADER = 1,
	PRE_INTEGRATION_TRANSFER_FUNCTION_SHADER = 2,
	LOCAL_ILLUMINATION_SHADER = 3,
	NON_POLYGONAL_SHADER = 4,
	FC_NORMAL_ESTIMATION_SHADER = 5,
	FC_CURVATURE_ESTIMATION_SHADER = 6,
	FC_FINAL_RENDERING_SHADER = 7
};

//
// Global handles for the currently active program object, with its two shader objects
//
GLuint ProgramObject = 0;
GLuint VertexShaderObject = 0;
GLuint FragmentShaderObject = 0;

GLuint shaderVS, shaderFS, shaderProg[10];   // handles to objects
GLint  linked;


//  The number of frames
int frameCount = 0;
float fps = 0;
int currentTime = 0, previousTime = 0;

void calculateFPS()
{

	frameCount++;
	currentTime = glutGet(GLUT_ELAPSED_TIME);

    int timeInterval = currentTime - previousTime;

    if(timeInterval > 1000)
    {
        fps = frameCount / (timeInterval / 1000.0f);
        previousTime = currentTime;
        frameCount = 0;
		std::cout << fps << std::endl;
    }

}

void readArguments(int argc, char** argv)
{
	if(argv[1] == "-h")
	{
		std::cout << "Arguments: path" << std::endl;
		std::cout << "example config.txt" << std::endl;
	}
	else
	{
		std::fstream file(argv[1]);
		std::string line;
		if(file.is_open()) {
			std::getline(file, line);
			strcpy(path, line.c_str());
			std::getline(file, line);
			strcpy(vrparams.transferFunctionPath, line.c_str());
			std::getline(file, line);
			firstSlice = atoi(line.c_str());
			std::getline(file, line);
			lastSlice = atoi(line.c_str());
			std::getline(file, line);
			strcpy(pathExtension, line.c_str());
			std::getline(file, line);
			vrparams.scaleWidth = atof(line.c_str());
			std::getline(file, line);
			vrparams.scaleHeight = atof(line.c_str());
			std::getline(file, line);
			vrparams.scaleDepth = atof(line.c_str());
			std::getline(file, line);
			vrparams.rotationX = atoi(line.c_str());
			std::getline(file, line);
			vrparams.rotationY = atoi(line.c_str());
			std::getline(file, line);
			vrparams.rotationZ = atoi(line.c_str());
			std::cout << "Path: " << path << std::endl;
			std::cout << "Number of the first slice: " << firstSlice << std::endl;
			std::cout << "Number of the last slice: " << lastSlice << std::endl;
			std::cout << "Extension: " << pathExtension << std::endl;
			std::cout << "Scale factor: x(" << vrparams.scaleWidth << "), y(" << vrparams.scaleHeight << "), z(" << vrparams.scaleDepth << ")";
		}
	}
}

void reshape(int w, int h)
{
	
	windowWidth = w;
	windowHeight = h;

	glViewport( 0, 0, windowWidth, windowHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0, windowWidth, 0, windowHeight );
	glMatrixMode( GL_MODELVIEW );

}

void displayQuadForVolumeRendering(bool back) 
{
	
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	myGLCloudViewer->configureQuadAmbient(eye, at, up);
	myGLCloudViewer->loadVBOQuad(quadVBO, 1.0f/vrparams.scaleWidth, 1.0f/vrparams.scaleHeight, 1.0f/vrparams.scaleDepth);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0, trans[1], trans[2]);
	glRotatef(rot[vrparams.rotationX], 1, 0, 0);
	glRotatef(rot[vrparams.rotationY], 0, 1, 0);
	glRotatef(rot[vrparams.rotationZ], 0, 0, 1);
	
	if(back) 
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}

	myGLCloudViewer->drawQuad(quadVBO);
	
	if(back) 
	{
		glDisable(GL_CULL_FACE);
	}

	glPopMatrix();

}

void display()
{
	
	glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );

	if(automatic)
		rot[1] += vel * 5;
		
	glBindFramebuffer(GL_FRAMEBUFFER, backQuadFrameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	displayQuadForVolumeRendering(true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, frontQuadFrameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	displayQuadForVolumeRendering(false);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0, trans[1], trans[2]);
	glRotatef(rot[vrparams.rotationX], 1, 0, 0);
	glRotatef(rot[vrparams.rotationY], 0, 1, 0);
	glRotatef(rot[vrparams.rotationZ], 0, 0, 1);
	
	glViewport(0, 0, windowWidth, windowHeight);
	glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );
	myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
	
	if(vrparams.naiveVolumeRendering)
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[VOLUME_RENDERING_SHADER], windowWidth, windowHeight, vrparams);
	else if(vrparams.transferFunction)
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[PRE_INTEGRATION_TRANSFER_FUNCTION_SHADER], windowWidth, windowHeight, vrparams);
	else if(vrparams.localIllumination)
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[LOCAL_ILLUMINATION_SHADER], windowWidth, windowHeight, vrparams);
	else if(vrparams.contextPreservingVolumeRendering)
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[CONTEXT_PRESERVING_VOLUME_RENDERING_SHADER], windowWidth, windowHeight, vrparams);
	else if(vrparams.nonPolygonalIsoSurface)
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[NON_POLYGONAL_SHADER], windowWidth, windowHeight, vrparams);
	else if(vrparams.FCVisualization) {

		//context layer
		vrparams.nonPolygonalIsoSurface = true;
		vrparams.isoSurfaceThreshold = 0.1;
		
		glBindFramebuffer(GL_FRAMEBUFFER, positionFrameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[NON_POLYGONAL_SHADER], windowWidth, windowHeight, vrparams);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
		glBindFramebuffer(GL_FRAMEBUFFER, normalFrameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );
		myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[FC_NORMAL_ESTIMATION_SHADER], windowWidth, windowHeight, vrparams);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, curvatureFrameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );
		myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[FC_CURVATURE_ESTIMATION_SHADER], windowWidth, windowHeight, vrparams);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		vrparams.FCVisualization = false;
		vrparams.isoSurfaceThreshold = 0.35;
		glBindFramebuffer(GL_FRAMEBUFFER, focusFrameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );
		myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[NON_POLYGONAL_SHADER], windowWidth, windowHeight, vrparams);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		vrparams.FCVisualization = true;
		
		glBindFramebuffer(GL_FRAMEBUFFER, contextFrameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );
		myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
		myGLImageViewer->draw3DTexture(texVBO, rot, trans, shaderProg[PRE_INTEGRATION_TRANSFER_FUNCTION_SHADER], windowWidth, windowHeight, vrparams);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		myGLImageViewer->drawFCVisualization(texVBO, shaderProg[FC_FINAL_RENDERING_SHADER], windowWidth, windowHeight, vrparams);
		vrparams.nonPolygonalIsoSurface = false;

	}	

	glPopMatrix();
	
	glutSwapBuffers();
	glutPostRedisplay();


}	

void idle() 
{
	calculateFPS();
}

void resetVolumeRenderingMode()
{

	vrparams.naiveVolumeRendering = false;
	vrparams.transferFunction = false;
	vrparams.localIllumination = false;
	vrparams.nonPolygonalIsoSurface = false;
	vrparams.contextPreservingVolumeRendering = false;
	vrparams.FCVisualization = false;

}

void volumeRenderingMenu(int id)
{


	switch(id)
	{
	case 0:
		resetVolumeRenderingMode();
		vrparams.naiveVolumeRendering = true;
		break;
	case 1:
		resetVolumeRenderingMode();
		vrparams.transferFunction = true;
		break;
	case 2:
		resetVolumeRenderingMode();
		vrparams.localIllumination = true;
		break;
	case 3:
		resetVolumeRenderingMode();
		vrparams.nonPolygonalIsoSurface = true;
		break;
	case 4:
		resetVolumeRenderingMode();
		vrparams.contextPreservingVolumeRendering = true;
		break;
	case 5:
		resetVolumeRenderingMode();
		vrparams.FCVisualization = true;
		break;
	case 6:
		vrparams.stochasticJithering = !vrparams.stochasticJithering;
		break;
	case 7:
		vrparams.triCubicInterpolation = !vrparams.triCubicInterpolation;
		break;
	case 8:
		vrparams.MIP = !vrparams.MIP;
		break;
	case 9:
		vrparams.gradientByForwardDifferences = !vrparams.gradientByForwardDifferences;
		break;
	case 10:
		vrparams.clippingOcclusion = !vrparams.clippingOcclusion;
		break;
	case 11:
		vrparams.inverseClipping = !vrparams.inverseClipping;
		break;
	}
}

void thresholdMenu(int id)
{

	translation = false;
	rotation = false;
	focusRadiusOn = false;
	automatic = false;
	earlyRayTermination = false;
	stepSizeModification = false;
	isosurfaceThresholdModification = false;
	ks = false;
	kt = false;
	clippingPlaneLeftXOn = false;
	clippingPlaneRightXOn = false;
	clippingPlaneUpYOn = false;
	clippingPlaneDownYOn = false;
	clippingPlaneFrontZOn = false;
	clippingPlaneBackZOn = false;

	switch(id)
	{
	case 0:
		earlyRayTermination = true;
		break;
	case 1:
		stepSizeModification = true;
		break;
	case 2:
		isosurfaceThresholdModification = true; 
		break;
	case 3:
		ks = true;
		break;
	case 4:
		kt = true;
		break;
	case 5:
		clippingPlaneLeftXOn = true;
		break;
	case 6:
		clippingPlaneRightXOn = true;
		break;
	case 7:
		clippingPlaneUpYOn = true;
		break;
	case 8:
		clippingPlaneDownYOn = true;
		break;
	case 9:
		clippingPlaneFrontZOn = true;
		break;
	case 10:
		clippingPlaneBackZOn = true;
		break;	
	case 11:
		focusRadiusOn = true;
		break;
	}
}

void transformationMenu(int id) 
{
	
	translation = false;
	rotation = false;
	focusRadiusOn = false;
	automatic = false;
	earlyRayTermination = false;
	stepSizeModification = false;
	isosurfaceThresholdModification = false;
	ks = false;
	kt = false;
	clippingPlaneLeftXOn = false;
	clippingPlaneRightXOn = false;
	clippingPlaneUpYOn = false;
	clippingPlaneDownYOn = false;
	clippingPlaneFrontZOn = false;
	clippingPlaneBackZOn = false;

	switch(id)
	{
	case 0:
		translation = true;
		break;
	case 1:
		rotation = true;
		break;
	case 2:
		automatic = true;
		break;
	}

}

void mainMenu(int id)
{
}

void createMenu()
{
	GLint volumeRenderingMenuID, thresholdMenuID, transformationMenuID;

	volumeRenderingMenuID = glutCreateMenu(volumeRenderingMenu);
		glutAddMenuEntry("Naive Volume Rendering", 0);
		glutAddMenuEntry("Transfer Function", 1);
		glutAddMenuEntry("Local Illumination", 2);
		glutAddMenuEntry("Non Polygonal Iso Surface Rendering", 3);
		glutAddMenuEntry("Context-Preserving Volume Rendering", 4);
		glutAddMenuEntry("Focus + Context Visualization", 5);
		glutAddMenuEntry("Stochastic Jithering [On/Off]", 6);
		glutAddMenuEntry("Tricubic Interpolation [On/Off]", 7);
		glutAddMenuEntry("MIP [On/Off]", 8);
		glutAddMenuEntry("Gradient by Forward Differences [On/Off]", 9);
		glutAddMenuEntry("Occlusion Based on Clipping [On/Off]", 10);
		glutAddMenuEntry("Invert Clipping [On/Off]", 11);
		
	thresholdMenuID = glutCreateMenu(thresholdMenu);
		glutAddMenuEntry("Change Early Ray Termination", 0);
		glutAddMenuEntry("Change Step Size (Raycasting)", 1);
		glutAddMenuEntry("Change Iso Surface", 2);
		glutAddMenuEntry("Change Ks (Context-Preserving VR)", 3);
		glutAddMenuEntry("Change Kt (Context-Preserving VR)", 4);
		glutAddMenuEntry("Change Clipping Plane Right X", 5); //inverted (also in specialKeyboard)
		glutAddMenuEntry("Change Clipping Plane Left X", 6); //inverted (also in specialKeyboard)
		glutAddMenuEntry("Change Clipping Plane Up Y", 7);
		glutAddMenuEntry("Change Clipping Plane Down Y", 8);
		glutAddMenuEntry("Change Clipping Plane Front Z", 9);
		glutAddMenuEntry("Change Clipping Plane Back Z", 10);
		glutAddMenuEntry("Change Focus Radius", 11);
	
	transformationMenuID = glutCreateMenu(transformationMenu);
		glutAddMenuEntry("Change Translation", 0);
		glutAddMenuEntry("Change Rotation", 1);
		glutAddMenuEntry("Automatic Rotation", 2);

	glutCreateMenu(mainMenu);
		glutAddSubMenu("Transformation", transformationMenuID);
		glutAddSubMenu("Volume Rendering", volumeRenderingMenuID);
		glutAddSubMenu("Threshold", thresholdMenuID);
		glutAttachMenu(GLUT_RIGHT_BUTTON);

}

void initGL()
{

	//glClearColor( 1.0f, 1.0f, 1.0f, 0.0 );
	glClearColor(0.0f, 0.0f, 0.0f, 0.0);
	glShadeModel(GL_SMOOTH);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);  

	if(texVBO[0] == 0)
		glGenTextures(20, texVBO);

	if(quadVBO[0] == 0)
		glGenBuffers(4, quadVBO);

	if(frontQuadFrameBuffer == 0)
		glGenFramebuffers(1, &frontQuadFrameBuffer);
	if(backQuadFrameBuffer == 0)
		glGenFramebuffers(1, &backQuadFrameBuffer);
	if(positionFrameBuffer == 0)
		glGenFramebuffers(1, &positionFrameBuffer);
	if(normalFrameBuffer == 0)
		glGenFramebuffers(1, &normalFrameBuffer);
	if(curvatureFrameBuffer == 0)
		glGenFramebuffers(1, &curvatureFrameBuffer);
	if(focusFrameBuffer == 0)
		glGenFramebuffers(1, &focusFrameBuffer);
	if(contextFrameBuffer == 0)
		glGenFramebuffers(1, &contextFrameBuffer);

	myGLImageViewer = new MyGLImageViewer();
	myGLImageViewer->load3DTextureFromTIFFile(volume->getData(), texVBO, VOLUME_TEXTURE, volume->getWidth(), volume->getHeight(), volume->getDepth(), GL_LINEAR);
	myGLImageViewer->load3DTextureFromTIFFile(minMaxOctree->getData(), texVBO, MIN_MAX_OCTREE_TEXTURE, minMaxOctree->getWidth(), minMaxOctree->getHeight(), minMaxOctree->getDepth(), GL_LINEAR);

	myGLCloudViewer = new MyGLCloudViewer();

	transferFunction = new TransferFunction();
	transferFunction->load(vrparams.transferFunctionPath);
	transferFunction->computePreIntegrationTable();
	myGLImageViewer->load2DTexture(transferFunction->getPreIntegrationTable(), texVBO, TRANSFER_FUNCTION_TEXTURE, 256, 256);

	myGLImageViewer->load2DNoiseTexture(texVBO, NOISE_TEXTURE, 32, 32);

	vrparams.stepSize = 0.008;//1.0/50.0;
	vrparams.earlyRayTerminationThreshold = 0.95;
	vrparams.kt = 1;
	vrparams.ks = 0;
	vrparams.stochasticJithering = false;
	vrparams.triCubicInterpolation = false;
	vrparams.naiveVolumeRendering = true;
	vrparams.transferFunction = false;
	vrparams.localIllumination = false;
	vrparams.contextPreservingVolumeRendering = false;
	vrparams.MIP = false;
	vrparams.nonPolygonalIsoSurface = false;
	vrparams.FCVisualization = false;
	vrparams.gradientByForwardDifferences = false;
	vrparams.isoSurfaceThreshold = 0.1;
	//Inverted because of the view
	vrparams.clippingPlane = true;
	vrparams.inverseClipping = false;
	vrparams.clippingOcclusion = false;
	vrparams.clippingPlaneLeftX = 0.0;
	vrparams.clippingPlaneRightX = 1.0;
	vrparams.clippingPlaneUpY = 1.0;
	vrparams.clippingPlaneDownY = 0.0;
	vrparams.clippingPlaneFrontZ = 0.0;
	vrparams.clippingPlaneBackZ = 1.0;
	vrparams.volumeTextureIndex = VOLUME_TEXTURE;
	vrparams.transferFunctionTextureIndex = TRANSFER_FUNCTION_TEXTURE;
	vrparams.minMaxOctreeTextureIndex = MIN_MAX_OCTREE_TEXTURE;
	vrparams.noiseTextureIndex = NOISE_TEXTURE;
	vrparams.backQuadTextureIndex = BACK_QUAD_FRAMEBUFFER_COLOR_TEXTURE;
	vrparams.frontQuadTextureIndex = FRONT_QUAD_FRAMEBUFFER_COLOR_TEXTURE;
	vrparams.positionTextureIndex = POSITION_FRAMEBUFFER_COLOR_TEXTURE;
	vrparams.normalTextureIndex = NORMAL_FRAMEBUFFER_COLOR_TEXTURE;
	vrparams.curvatureTextureIndex = CURVATURE_FRAMEBUFFER_COLOR_TEXTURE;
	vrparams.focusTextureIndex = FOCUS_FRAMEBUFFER_COLOR_TEXTURE;
	vrparams.contextTextureIndex = CONTEXT_FRAMEBUFFER_COLOR_TEXTURE;
	vrparams.focusPoint[0] = 0;
	vrparams.focusPoint[1] = 0;
	vrparams.focusRadius = 1;

	createMenu();

	myGLImageViewer->loadRGBTexture(NULL, texVBO, BACK_QUAD_FRAMEBUFFER_COLOR_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, FRONT_QUAD_FRAMEBUFFER_COLOR_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, POSITION_FRAMEBUFFER_COLOR_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, NORMAL_FRAMEBUFFER_COLOR_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, CURVATURE_FRAMEBUFFER_COLOR_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, FOCUS_FRAMEBUFFER_COLOR_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, CONTEXT_FRAMEBUFFER_COLOR_TEXTURE, windowWidth, windowHeight);
	
	myGLImageViewer->loadDepthComponentTexture(NULL, texVBO, BACK_QUAD_FRAMEBUFFER_DEPTH_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadDepthComponentTexture(NULL, texVBO, FRONT_QUAD_FRAMEBUFFER_DEPTH_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadDepthComponentTexture(NULL, texVBO, POSITION_FRAMEBUFFER_DEPTH_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadDepthComponentTexture(NULL, texVBO, NORMAL_FRAMEBUFFER_DEPTH_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadDepthComponentTexture(NULL, texVBO, CURVATURE_FRAMEBUFFER_DEPTH_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadDepthComponentTexture(NULL, texVBO, FOCUS_FRAMEBUFFER_DEPTH_TEXTURE, windowWidth, windowHeight);
	myGLImageViewer->loadDepthComponentTexture(NULL, texVBO, CONTEXT_FRAMEBUFFER_DEPTH_TEXTURE, windowWidth, windowHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, backQuadFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texVBO[BACK_QUAD_FRAMEBUFFER_DEPTH_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texVBO[BACK_QUAD_FRAMEBUFFER_COLOR_TEXTURE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO OK" << std::endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, frontQuadFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texVBO[FRONT_QUAD_FRAMEBUFFER_DEPTH_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texVBO[FRONT_QUAD_FRAMEBUFFER_COLOR_TEXTURE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO OK" << std::endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, positionFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texVBO[POSITION_FRAMEBUFFER_DEPTH_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texVBO[POSITION_FRAMEBUFFER_COLOR_TEXTURE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO OK" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, normalFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texVBO[NORMAL_FRAMEBUFFER_DEPTH_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texVBO[NORMAL_FRAMEBUFFER_COLOR_TEXTURE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO OK" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, curvatureFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texVBO[CURVATURE_FRAMEBUFFER_DEPTH_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texVBO[CURVATURE_FRAMEBUFFER_COLOR_TEXTURE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO OK" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, focusFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texVBO[FOCUS_FRAMEBUFFER_DEPTH_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texVBO[FOCUS_FRAMEBUFFER_COLOR_TEXTURE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO OK" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, contextFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texVBO[CONTEXT_FRAMEBUFFER_DEPTH_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texVBO[CONTEXT_FRAMEBUFFER_COLOR_TEXTURE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO OK" << std::endl;

}

void keyboard(unsigned char key, int x, int y) 
{
	
	switch(key) {
	case 27:
		exit(0);
		break;
	case 's':
		scale++;
		break;
	default:
		break;
	}
	
}

void specialKeyboard(int key, int x, int y)
{
	
	switch(key) {
	case GLUT_KEY_UP:
		if(translation)
			trans[1] += vel;
		if(rotation)
			rot[1] += 5 * vel;
		if(focusRadiusOn)
			vrparams.focusRadius += 0.1;
		if(earlyRayTermination)
			vrparams.earlyRayTerminationThreshold += 0.01;
		if(stepSizeModification)
			vrparams.stepSize += 1.0/2048.0;
		if(isosurfaceThresholdModification)
			vrparams.isoSurfaceThreshold += 0.05;
		if(ks)
			vrparams.ks += 0.05;
		if(kt)
			vrparams.kt += 1;
		if(clippingPlaneUpYOn) {
			vrparams.clippingPlaneUpY += 0.05;
			if(vrparams.clippingPlaneUpY > 1) vrparams.clippingPlaneUpY = 1;
		}
		if(clippingPlaneDownYOn) {
			vrparams.clippingPlaneDownY += 0.05;
			if(vrparams.clippingPlaneDownY > 1) vrparams.clippingPlaneDownY = 1;
		}
		break;
	case GLUT_KEY_DOWN:
		if(translation)
			trans[1] -= vel;
		if(rotation)
			rot[1] -= 5 * vel;
		if(focusRadiusOn) {
			vrparams.focusRadius -= 0.1;
			if(vrparams.focusRadius < 0) vrparams.focusRadius = 0;
		}
		if(earlyRayTermination)
			vrparams.earlyRayTerminationThreshold -= 0.01;
		if(stepSizeModification) {
			vrparams.stepSize -= 1.0/2048.0;
			if(vrparams.stepSize <= 0) vrparams.stepSize = 0;
		}
		if(isosurfaceThresholdModification)
			vrparams.isoSurfaceThreshold -= 0.05;
		if(ks)
			vrparams.ks -= 0.05;
		if(kt)
			vrparams.kt -= 1;
		if(clippingPlaneUpYOn) {
			vrparams.clippingPlaneUpY -= 0.05;
			if(vrparams.clippingPlaneUpY < 0) vrparams.clippingPlaneUpY = 0;
		}
		if(clippingPlaneDownYOn) {
			vrparams.clippingPlaneDownY -= 0.05;
			if(vrparams.clippingPlaneDownY < 0) vrparams.clippingPlaneDownY = 0;
		}
		break;
	case GLUT_KEY_LEFT:
		if(translation)
			trans[0] -= vel;
		if(rotation)
			rot[0] -= 5 * vel;
		if(clippingPlaneLeftXOn) {
			vrparams.clippingPlaneLeftX += 0.05f;
			if(vrparams.clippingPlaneLeftX > 1) vrparams.clippingPlaneLeftX = 1;
		}
		if(clippingPlaneRightXOn) {
			vrparams.clippingPlaneRightX += 0.05f;
			if(vrparams.clippingPlaneRightX > 1) vrparams.clippingPlaneRightX = 1;
		}
		break;
	case GLUT_KEY_RIGHT:
		if(translation)
			trans[0] += vel;
		if(rotation)
			rot[0] += 5 * vel;
		if(clippingPlaneLeftXOn) {
			vrparams.clippingPlaneLeftX -= 0.05f;
			if(vrparams.clippingPlaneLeftX < 0) vrparams.clippingPlaneLeftX = 0;
		} if(clippingPlaneRightXOn) {
			vrparams.clippingPlaneRightX -= 0.05f;
			if(vrparams.clippingPlaneRightX < 0) vrparams.clippingPlaneRightX = 0;
		}
		break;
	case GLUT_KEY_PAGE_UP:
		if(translation)
			trans[2] += vel;
		if(rotation)
			rot[2] += 5 * vel;
		if(clippingPlaneFrontZOn) {
			vrparams.clippingPlaneFrontZ += 0.05f;
			if(vrparams.clippingPlaneFrontZ > 1) vrparams.clippingPlaneFrontZ = 1;
		}
		if(clippingPlaneBackZOn) {
			vrparams.clippingPlaneBackZ += 0.05f;
			if(vrparams.clippingPlaneBackZ > 1) vrparams.clippingPlaneBackZ = 1;
		}
		break;
	case GLUT_KEY_PAGE_DOWN:
		if(translation)
			trans[2] -= vel;
		if(rotation)
			rot[2] -= 5 * vel;
		if(clippingPlaneFrontZOn) {
			vrparams.clippingPlaneFrontZ -= 0.05f;
			if(vrparams.clippingPlaneFrontZ < 0) vrparams.clippingPlaneFrontZ = 0;
		}
		if(clippingPlaneBackZOn) {
			vrparams.clippingPlaneBackZ -= 0.05f;
			if(vrparams.clippingPlaneBackZ < 0) vrparams.clippingPlaneBackZ = 0;
		}
		break;
	default:
		break;
	}
	
}

void mouse(int button, int state, int x, int y) 
{

	if (button == GLUT_LEFT_BUTTON)
		if (state == GLUT_UP) {
			vrparams.focusPoint[0] = (float)x;///(float)windowWidth;
			vrparams.focusPoint[1] = windowHeight - (float)y;///(float)windowHeight;
		}

		std::cout << vrparams.focusPoint[0] << " " << vrparams.focusPoint[1] << std::endl;
	glutPostRedisplay();

}

int main(int argc, char** argv)
{
	
	readArguments(argc, argv);
	volume = new Volume();

	if(!strcmp(pathExtension, "tif"))
		volume->loadTIFData(path, firstSlice, lastSlice);
	else if(!strcmp(pathExtension, "pgm"))
		volume->loadPGMData(path, firstSlice, lastSlice);
	else
		volume->loadRAWData(path, firstSlice, firstSlice, lastSlice);

	//triCubicInterpolationPreFilter = new TriCubicInterpolationPreFilter();
	//triCubicInterpolationPreFilter->applyPreFilterForAccurateCubicBSplineInterpolation(volume->getData(), volume->getWidth(), volume->getHeight(), volume->getDepth());

	minMaxOctree = new MinMaxOctree(volume->getWidth(), volume->getHeight(), volume->getDepth());
	minMaxOctree->build(volume->getData(), volume->getWidth(), volume->getHeight(), volume->getDepth());
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_ALPHA);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Volume Rendering");

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);

	glewInit();
	initGL();
	
	initShader("Shaders/VRBlendRaycasting", VOLUME_RENDERING_SHADER);
	initShader("Shaders/VRContextPreservingPreIntegrationRaycasting", CONTEXT_PRESERVING_VOLUME_RENDERING_SHADER);
	initShader("Shaders/VRPreIntegrationRaycasting", PRE_INTEGRATION_TRANSFER_FUNCTION_SHADER);
	initShader("Shaders/VRLocalIlluminationPreIntegrationRaycasting", LOCAL_ILLUMINATION_SHADER);
	initShader("Shaders/VRNonPolygonalRaycasting", NON_POLYGONAL_SHADER);
	initShader("Shaders/FCNormalEstimation", FC_NORMAL_ESTIMATION_SHADER);
	initShader("Shaders/FCCurvatureEstimation", FC_CURVATURE_ESTIMATION_SHADER);
	initShader("Shaders/FCFinalRendering", FC_FINAL_RENDERING_SHADER);

	glUseProgram(0);

	glutMainLoop();
	
	delete volume;
	delete minMaxOctree;
	delete myGLImageViewer;
	delete myGLCloudViewer;
	delete transferFunction;
	delete triCubicInterpolationPreFilter;
	
	return 0;
	
}