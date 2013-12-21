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
bool automatic = false;

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

//
// Global handles for the currently active program object, with its two shader objects
//
GLuint ProgramObject = 0;
GLuint VertexShaderObject = 0;
GLuint FragmentShaderObject = 0;

GLuint shaderVS, shaderFS, shaderProg[6];   // handles to objects
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
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0, trans[1], trans[2]);
	glRotatef(rot[vrparams.rotationX], 1, 0, 0);
	glRotatef(rot[vrparams.rotationY], 0, 1, 0);
	glRotatef(rot[vrparams.rotationZ], 0, 0, 1);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	myGLImageViewer->drawQuads(1.0f/vrparams.scaleWidth, 1.0f/vrparams.scaleHeight, 1.0f/vrparams.scaleDepth);
	myGLImageViewer->loadFrameBufferTexture(texVBO, 5, 0, windowHeight/2, windowWidth/2, windowHeight/2);
	glDisable(GL_CULL_FACE);

	myGLImageViewer->drawQuads(1.0f/vrparams.scaleWidth, 1.0f/vrparams.scaleHeight, 1.0f/vrparams.scaleDepth);
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

void initGL()
{

	
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0 );
	glShadeModel(GL_SMOOTH);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);  

	if(texVBO[0] == 0)
		glGenTextures(10, texVBO);

	myGLImageViewer = new MyGLImageViewer();
	myGLImageViewer->load3DTextureFromTIFFile(volume->getData(), texVBO, 0, volume->getWidth(), volume->getHeight(), volume->getDepth(), GL_LINEAR);
	myGLImageViewer->load3DTextureFromTIFFile(minMaxOctree->getData(), texVBO, 2, minMaxOctree->getWidth(), minMaxOctree->getHeight(), minMaxOctree->getDepth(), GL_LINEAR);

	myGLCloudViewer = new MyGLCloudViewer();

	transferFunction = new TransferFunction();
	transferFunction->load();
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
	
	myGLImageViewer->loadRGBTexture(NULL, texVBO, 5, windowWidth, windowHeight);
	myGLImageViewer->loadRGBTexture(NULL, texVBO, 6, windowWidth, windowHeight);

}

void keyboard(unsigned char key, int x, int y) 
{
	
	switch(key) {
	case 27:
		exit(0);
		break;
	case 'r':
		rotation = true;
		translation = false;
		earlyRayTermination = false;
		stepSizeModification = false;
		isosurfaceThresholdModification = false;
		break;
	case 't':
		translation = true;
		rotation = false;
		earlyRayTermination = false;
		stepSizeModification = false;
		isosurfaceThresholdModification = false;
		break;
	case 'e':
		translation = false;
		rotation = false;
		earlyRayTermination = true;
		stepSizeModification = false;
		isosurfaceThresholdModification = false;
		break;
	case 's':
		translation = false;
		rotation = false;
		earlyRayTermination = false;
		stepSizeModification = true;
		isosurfaceThresholdModification = false;
		break;
	case 'o':
		translation = false;
		rotation = false;
		earlyRayTermination = false;
		stepSizeModification = false;
		isosurfaceThresholdModification = true;
		break;
	case 'j':
		vrparams.stochasticJithering = !vrparams.stochasticJithering;
		break;
	case 'i':
		vrparams.triCubicInterpolation = !vrparams.triCubicInterpolation;
		break;
	case 'a':
		automatic = !automatic;
		break;
	case 'k':
		vrparams.kt += 1;
		printf("Kt: %f\n", vrparams.kt);
		break;
	case 'K':
		vrparams.kt -= 1;
		printf("Kt: %f\n", vrparams.kt);
		break;
	case 'l':
		vrparams.ks += 0.05;
		printf("Ks: %f\n", vrparams.ks);
		break;
	case 'L':
		vrparams.ks -= 0.05;
		printf("Ks: %f\n", vrparams.ks);
		break;
	case 'm':
		vrparams.MIP = !vrparams.MIP;
		break;
	case 'g':
		vrparams.gradientByForwardDifferences = !vrparams.gradientByForwardDifferences;
		break;
	case 'n':
		vrparams.NonPolygonalIsoSurface = !vrparams.NonPolygonalIsoSurface;
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
		break;
	case GLUT_KEY_LEFT:
		if(translation)
			trans[0] -= vel;
		if(rotation)
			rot[0] -= 5 * vel;
		break;
	case GLUT_KEY_RIGHT:
		if(translation)
			trans[0] += vel;
		if(rotation)
			rot[0] += 5 * vel;
		break;
	case GLUT_KEY_PAGE_UP:
		if(translation)
			trans[2] += vel;
		if(rotation)
			rot[2] += 5 * vel;
		break;
	case GLUT_KEY_PAGE_DOWN:
		if(translation)
			trans[2] -= vel;
		if(rotation)
			rot[2] -= 5 * vel;
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