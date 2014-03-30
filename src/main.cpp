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

int windowWidth = 1280;
int windowHeight = 960;

char path[1000];
int firstSlice, lastSlice;
char pathExtension[3];

bool rotation = false;
bool translation = false;
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

GLuint texVBO[10];
GLuint quadVBO[4];

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
			std::cout << "Scale factor: x (" << vrparams.scaleWidth << "), y(" << vrparams.scaleHeight << "), z(" << vrparams.scaleDepth << ")";
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

void display()
{
	
	glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );

	if(automatic)
		rot[1] += vel * 5;

	glViewport( 0, windowHeight/2, windowWidth/2, windowHeight/2 );
	
	myGLCloudViewer->configureQuadAmbient(eye, at, up);
	myGLCloudViewer->loadVBOQuad(quadVBO, 1.0f/vrparams.scaleWidth, 1.0f/vrparams.scaleHeight, 1.0f/vrparams.scaleDepth);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0, trans[1], trans[2]);
	glRotatef(rot[vrparams.rotationX], 1, 0, 0);
	glRotatef(rot[vrparams.rotationY], 0, 1, 0);
	glRotatef(rot[vrparams.rotationZ], 0, 0, 1);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	myGLCloudViewer->drawQuad(quadVBO);
	myGLImageViewer->loadFrameBufferTexture(texVBO, 5, 0, windowHeight/2, windowWidth/2, windowHeight/2);
	glDisable(GL_CULL_FACE);

	myGLCloudViewer->drawQuad(quadVBO);
	myGLImageViewer->loadFrameBufferTexture(texVBO, 6, 0, windowHeight/2, windowWidth/2, windowHeight/2);
	glPopMatrix();
	
	glViewport( 0, windowHeight/2, windowWidth/2, windowHeight/2 );
	glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );
	
	myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
	myGLImageViewer->draw3DTexture(texVBO, 0, 2, rot, trans, shaderProg[0], windowWidth, windowHeight, vrparams, 1);
	
	glViewport( windowWidth/2, windowHeight/2, windowWidth/2, windowHeight/2 );
	
	myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
	myGLImageViewer->draw3DTexture(texVBO, 0, 2, rot, trans, shaderProg[1], windowWidth, windowHeight, vrparams, 1);
	
	glViewport(0, 0, windowWidth/2, windowHeight/2);
	
	myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
	myGLImageViewer->draw3DTexture(texVBO, 0, 2, rot, trans, shaderProg[2], windowWidth, windowHeight, vrparams, 1);
	
	glViewport(windowWidth/2, 0, windowWidth/2, windowHeight/2);
	
	myGLCloudViewer->configureAmbient(eye, at, up, vrparams);
	
	if(vrparams.NonPolygonalIsoSurface)
		myGLImageViewer->draw3DTexture(texVBO, 0, 2, rot, trans, shaderProg[4], windowWidth, windowHeight, vrparams, 1);
	else
		myGLImageViewer->draw3DTexture(texVBO, 0, 2, rot, trans, shaderProg[3], windowWidth, windowHeight, vrparams, 1);
	
	glutSwapBuffers();
	glutPostRedisplay();


}	

void idle() 
{
	calculateFPS();
}

void volumeRenderingMenu(int id)
{
	switch(id)
	{
	case 0:
		vrparams.stochasticJithering = !vrparams.stochasticJithering;
		break;
	case 1:
		vrparams.triCubicInterpolation = !vrparams.triCubicInterpolation;
		break;
	case 2:
		vrparams.MIP = !vrparams.MIP;
		break;
	case 3:
		vrparams.NonPolygonalIsoSurface = !vrparams.NonPolygonalIsoSurface;
		break;
	case 4:
		vrparams.gradientByForwardDifferences = !vrparams.gradientByForwardDifferences;
		break;
	case 5:
		vrparams.clippingOcclusion = !vrparams.clippingOcclusion;
		break;
	case 6:
		vrparams.inverseClipping = !vrparams.inverseClipping;
		break;
	}
}

void thresholdMenu(int id)
{

	translation = false;
	rotation = false;
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
	}
}

void transformationMenu(int id) 
{
	
	translation = false;
	rotation = false;
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
		glutAddMenuEntry("Stochastic Jithering [On/Off]", 0);
		glutAddMenuEntry("Tricubic Interpolation [On/Off]", 1);
		glutAddMenuEntry("MIP [On/Off]", 2);
		glutAddMenuEntry("Non Polygonal Iso Surface Rendering", 3);
		glutAddMenuEntry("Gradient by Forward Differences [On/Off]", 4);
		glutAddMenuEntry("Occlusion Based on Clipping [On/Off]", 5);
		glutAddMenuEntry("Invert Clipping [On/Off]", 6);

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
		glGenTextures(10, texVBO);

	if(quadVBO[0] == 0)
		glGenBuffers(4, quadVBO);

	myGLImageViewer = new MyGLImageViewer();
	myGLImageViewer->load3DTextureFromTIFFile(volume->getData(), texVBO, 0, volume->getWidth(), volume->getHeight(), volume->getDepth(), GL_LINEAR);
	myGLImageViewer->load3DTextureFromTIFFile(minMaxOctree->getData(), texVBO, 2, minMaxOctree->getWidth(), minMaxOctree->getHeight(), minMaxOctree->getDepth(), GL_LINEAR);

	myGLCloudViewer = new MyGLCloudViewer();

	transferFunction = new TransferFunction();
	transferFunction->load(vrparams.transferFunctionPath);
	transferFunction->computePreIntegrationTable();
	myGLImageViewer->load2DTexture(transferFunction->getPreIntegrationTable(), texVBO, 1, 256, 256);

	myGLImageViewer->load2DNoiseTexture(texVBO, 3, 32, 32);

	vrparams.stepSize = 1.0/50.0;
	vrparams.earlyRayTerminationThreshold = 0.95;
	vrparams.kt = 1;
	vrparams.ks = 0;
	vrparams.stochasticJithering = false;
	vrparams.triCubicInterpolation = false;
	vrparams.MIP = false;
	vrparams.gradientByForwardDifferences = false;
	vrparams.NonPolygonalIsoSurface = false;
	vrparams.isoSurfaceThreshold = 0.1;
	vrparams.importanceAwareComposition = false;
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
	
	createMenu();

	myGLImageViewer->loadRGBTexture(NULL, texVBO, 5, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, 6, windowWidth, windowHeight);

}

void keyboard(unsigned char key, int x, int y) 
{
	
	switch(key) {
	case 27:
		exit(0);
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

	glewInit();
	initGL();
	
	initShader("Shaders/VRBlendRaycasting", 0);
	initShader("Shaders/VRContextPreservingPreIntegrationRaycasting", 1);
	initShader("Shaders/VRPreIntegrationRaycasting", 2);
	initShader("Shaders/VRLocalIlluminationPreIntegrationRaycasting", 3);
	initShader("Shaders/VRNonPolygonalRaycasting", 4);
	//initShader("Shaders/VRImportanceAwareCompositionRaycasting", 5);

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