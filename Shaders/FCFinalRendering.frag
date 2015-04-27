uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D curvature;
uniform sampler2D focusLayer;
uniform sampler2D contextLayer;
uniform vec2 focusPoint;
uniform float focusRadius;
uniform int windowWidth;
uniform int windowHeight;

void main()
{

	//ClearView - Distance-based Importance
	vec2 normalizedFragCoord = vec2(gl_FragCoord.x/float(windowWidth), gl_FragCoord.y/float(windowHeight));
	vec2 normalizedFocusPoint = vec2(focusPoint.x/float(windowWidth), focusPoint.y/float(windowHeight));

	vec4 focusColor = texture2D(focusLayer, normalizedFragCoord);
	vec4 contextColor = texture2D(contextLayer, normalizedFragCoord);
	float volumeCurvature = texture2D(curvature, normalizedFragCoord);
	vec3 volumePosition = texture2D(position, normalizedFragCoord);
	vec3 focusPosition = texture2D(position, normalizedFocusPoint);
	
	float trans = 1.0 - clamp(max(distance(focusPosition, volumePosition)/focusRadius, volumeCurvature), 0.0, 1.0);
	gl_FragColor = focusColor * trans + contextColor * (1.0 - trans);

}