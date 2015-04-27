uniform sampler3D volume;
uniform sampler2D normalBuffer;
uniform int windowWidth;
uniform int windowHeight;

void main (void)  
{

	vec3 center = texture2D(normalBuffer, vec2(gl_FragCoord.x/float(windowWidth), gl_FragCoord.y/float(windowHeight)));
	vec3 left = texture2D(normalBuffer, vec2((gl_FragCoord.x - 1)/float(windowWidth), gl_FragCoord.y/float(windowHeight)));
	vec3 right = texture2D(normalBuffer, vec2((gl_FragCoord.x + 1)/float(windowWidth), gl_FragCoord.y/float(windowHeight)));
	vec3 up = texture2D(normalBuffer, vec2(gl_FragCoord.x/float(windowWidth), (gl_FragCoord.y + 1)/float(windowHeight)));
	vec3 down = texture2D(normalBuffer, vec2(gl_FragCoord.x/float(windowWidth), (gl_FragCoord.y - 1)/float(windowHeight)));
	
	float A = distance(up, center);	
	float B = distance(left, center);
	float C = distance(down, center);
	float D = distance(right, center);

	float curvature =  abs(A) + abs(B) + abs(C) + abs(D);
	gl_FragColor = vec4(curvature, curvature, curvature, 1.0);

}