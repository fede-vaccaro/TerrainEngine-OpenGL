#version 330 core
in vec3 f_Col;
in vec3 Normal;
in vec2 texCoord;
in vec3 WorldPos;
in float distFromPos;
in float dispFactor;
in float height;

uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform vec3 fogColor;
uniform vec2 offset;
uniform bool drawFog;
uniform float gDispFactor;
uniform float freq;
uniform bool normals;

uniform sampler2D sand, grass, rock, snow;

out vec4 FragColor;

float Random2D(in vec2 st)
{
	return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float InterpolatedNoise(int ind, float x, float y) {
	int integer_X = int(floor(x));
	float fractional_X = fract(x);
	int integer_Y = int(floor(y));
	float fractional_Y = fract(y);

	vec2 randomInput = vec2(integer_X, integer_Y);
	float v1 = Random2D(randomInput);
	float v2 = Random2D(randomInput + vec2(1.0, 0.0));
	float v3 = Random2D(randomInput + vec2(0.0, 1.0));
	float v4 = Random2D(randomInput + vec2(1.0, 1.0));
	float i1 = mix(v1, v2, fractional_X);
	float i2 = mix(v3, v4, fractional_X);
	return mix(i1, i2, fractional_Y);
}

float perlin(float x, float y){
	
    int numOctaves = 10;
	float persistence = 0.5;
	float total = 0,
		frequency = pow(2, numOctaves),
		amplitude = 1;
	for (int i = 0; i < numOctaves; ++i) {
		frequency /= 2;
		amplitude *= persistence;
		
		total += InterpolatedNoise( int(mod(0 + i,10)), x / frequency, y / frequency) * amplitude;
	}
	return total / frequency;
}

vec3 computeNormals(vec3 WorldPos){
	float st = 0.1;
	float dhdu = (perlin((WorldPos.x + st)*freq, WorldPos.z*freq)*gDispFactor - perlin((WorldPos.x - st)*freq, WorldPos.z*freq)*gDispFactor)/(2.0*st);
	float dhdv = (perlin( WorldPos.x*freq, (WorldPos.z + st)*freq)*gDispFactor - perlin(WorldPos.x*freq, (WorldPos.z - st)*freq)*gDispFactor)/(2.0*st);

	vec3 X = vec3(1.0, dhdu, 1.0);
	vec3 Z = vec3(0.0, dhdv, 1.0);

	vec3 n = normalize(cross(Z,X));
	return n;
}

void main()
{
	
	vec3 n; 
	if(normals){
		n = computeNormals(WorldPos);
	}else{
		n = vec3(0,1,0);
	}
	//vec3 n = normalize(Normal+Z*dhdu+X*dhdv); // add offset to original normal (equivalent to calculate real surface normal)
	// calculate ambient illumination
	float ambientStrength = 0.15; 
    vec3 ambient = ambientStrength * u_LightColor; 

	// calculate diffuse illumination
	vec3 norm = mix(n, Normal, 0.5); 
	norm = normalize(norm);
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float diffuseFactor = max(0.0, dot(lightDir, norm));
	vec3 diffuse = diffuseFactor * u_LightColor; 

	// calculate specular illumination 
	float specularFactor = 0.15f;
	vec3 viewDir = normalize(u_ViewPosition - WorldPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	vec3 specular = spec * u_LightColor;  

	// calculate color by sampling the texture at the given coordinates

	vec4 rockColor = vec4(1.0, 0.0, 0.0, 1.0);//vec4(163, 163, 117, 255)/255.0;

	float trans = 0.4;

	vec4 texCol;
	if(height < dispFactor/2.5 - trans){
		texCol = texture(sand, texCoord*5.0);
	}else if( (height > dispFactor/2.5 - trans) && (height < dispFactor/2.5 + trans) ){
		vec4 texCol1 = texture(sand, texCoord*5.0);
		vec4 texCol2 = texture(grass, texCoord*5.0);
		texCol2 *= vec4(113, 113, 91, 255.0)/255.0;
		texCol = mix(texCol1, texCol2, (height - (dispFactor/2.5 - trans))/(2.0*trans) );

	}else if( (height > dispFactor/2.5 + trans) && (height < dispFactor/2.0 - trans)){
		texCol = texture(grass, texCoord*5.0);
		texCol *= vec4(113, 113, 91, 255.0)/255.0;
	}
	else if( (height > dispFactor/2.0 - trans) && (height < dispFactor/2.0 + trans) ){
		vec4 texCol1 = texture(grass, texCoord*5.0);
		vec4 texCol2 = texture(rock, texCoord*15.0);
		texCol1 *= vec4(113, 113, 91, 255.0)/255.0;
		texCol = mix(texCol1, texCol2, (height - (dispFactor/2.0 - trans))/(2.0*trans) );
	}else if( (height > dispFactor/2.0 + trans) && (height < dispFactor/1.3 - trans) ){
		texCol = texture(rock, texCoord*15.0);
	}	else if( (height > dispFactor/1.3 - trans) && (height < dispFactor/1.3 + trans) ){
		vec4 texCol1 = texture(rock, texCoord*15.0);
		vec4 texCol2 = vec4(1.0);
		texCol = mix(texCol1, texCol2, (height - (dispFactor/1.3 - trans))/(2.0*trans) );
	}
	else{
		texCol = vec4(1.0);
	}
	
	
	//texCol *= vec4(113, 113, 91, 255.0)/255.0;

	


	// calculate fog color 
	vec2 u_FogDist = vec2(20.0, 50.0);
	float fogFactor = clamp((u_FogDist.y - distFromPos) / (u_FogDist.y - u_FogDist.x), 0.0, 1.0);

	// putting all together
    vec4 color = texCol*vec4((ambient + specular + diffuse)*vec3(1.0) , 1.0f);
	if(drawFog){
	FragColor = mix(color, vec4(fogColor, 1.0f), 1 - fogFactor);
	}else{
	FragColor = color;
	}
};