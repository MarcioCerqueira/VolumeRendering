#ifndef VRPARAMS_H
#define VRPARAMS_H

typedef struct VRParams
{
	int volumeTextureIndex;
	int transferFunctionTextureIndex;
	int minMaxOctreeTextureIndex;
	int noiseTextureIndex;
	int backQuadTextureIndex;
	int frontQuadTextureIndex;
	int positionTextureIndex;
	int normalTextureIndex;
	int curvatureTextureIndex;
	int focusTextureIndex;
	int contextTextureIndex;
	//step size for raycasting
	float stepSize;
	//early ray termination threshold
	float earlyRayTerminationThreshold;
	//parameter for illustrative context-preserving
	float kt;
	//parameter for illustrative context-preserving
	float ks;
	//scale factors
	float scaleWidth;
	float scaleHeight;
	float scaleDepth;
	//rotation axis
	int rotationX;
	int rotationY;
	int rotationZ;
	bool stochasticJithering;
	bool triCubicInterpolation;
	bool naiveVolumeRendering;
	bool transferFunction;
	bool localIllumination;
	bool contextPreservingVolumeRendering;
	bool nonPolygonalIsoSurface;
	bool FCVisualization;
	bool MIP;
	float isoSurfaceThreshold;
	char transferFunctionPath[100];	
	bool gradientByForwardDifferences;
	//Clipping Planes
	bool clippingPlane;
	bool inverseClipping;
	bool clippingOcclusion;
	float clippingPlaneLeftX;
	float clippingPlaneRightX;
	float clippingPlaneUpY;
	float clippingPlaneDownY;
	float clippingPlaneFrontZ;
	float clippingPlaneBackZ;
	//ClearView
	float focusPoint[2];
	float focusRadius;
} VRParams;

#endif