uniform sampler3D volume;
uniform sampler2D positionBuffer;
uniform int windowWidth;
uniform int windowHeight;

vec3 computeNormal(vec3 position) 
{

	float delta = 0.01;
	vec3 sample1, sample2;
		
	sample1.x = texture3D(volume, vec3(position - vec3(delta, 0, 0))).x;
	sample1.y = texture3D(volume, vec3(position - vec3(0, delta, 0))).x;
	sample1.z = texture3D(volume, vec3(position - vec3(0, 0, delta))).x;
	
	sample2.x = texture3D(volume, vec3(position + vec3(delta, 0, 0))).x;
	sample2.y = texture3D(volume, vec3(position + vec3(0, delta, 0))).x;
	sample2.z = texture3D(volume, vec3(position + vec3(0, 0, delta))).x;
		
	return normalize(sample2 - sample1);
	
}

void main (void)  
{

	vec3 objectPosition = texture2D(positionBuffer, vec2(gl_FragCoord.x/float(windowWidth), gl_FragCoord.y/float(windowHeight)));
	gl_FragColor = vec4(computeNormal(objectPosition), 1.0);
	
}