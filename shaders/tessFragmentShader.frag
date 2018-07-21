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
uniform int octaves;
uniform bool drawFog;
uniform float gDispFactor;
uniform float freq;
uniform bool normals;
uniform float u_grassCoverage;
uniform float waterHeight;

uniform sampler2D sand, grass, rock, snow;

out vec4 FragColor;

float Noise(int ind, int x, int y) {

const int primes[30] = int[30](995615039, 600173719, 701464987,
					831731269, 162318869, 136250887, 
					174329291, 946737083, 245679977,
				 362489573, 795918041, 350777237, 
				 457025711, 880830799, 909678923, 
				 787070341, 177340217, 593320781,	
				 405493717, 291031019, 391950901, 
				 458904767, 676625681, 424452397, 
				 531736441, 939683957, 810651871,
				997169939, 842027887, 423882827);

	int n = x + y * 57;
	n = (n << 13) ^ n;
	int a = primes[ind*3], b = primes[ind*3 + 1], c = primes[ind*3 + 2];
	//int a = 787070341, b = 177340217, c = 593320781;
	int t = (n * (n * n * a + b) + c) & 0x7fffffff;
	return float(1.0 - t/1073741824.0);
}

float Random2D(in vec2 st)
{
	return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float Interpolate(float a, float b, float x) {  // cosine interpolation
	float ft = x * 3.1415927f;
	float f = (1 - cos(ft)) * 0.5;
	return  a * (1 - f) + b * f;
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
	//float v1 = Random2D(randomInput);
	//float v2 = Random2D(randomInput + vec2(1.0, 0.0));
	//float v3 = Random2D(randomInput + vec2(0.0, 1.0));
	//float v4 = Random2D(randomInput + vec2(1.0, 1.0));

	float i1 = mix(v1, v2, smoothstep(0, 1, fractional_X));
	float i2 = mix(v3, v4, smoothstep(0, 1, fractional_X));
	return mix(i1, i2, smoothstep(0, 1, fractional_Y));
}

float perlin(float x, float y){
	
    int numOctaves = octaves;
	float persistence = 0.5;
	float total = 0,
		frequency = 0.05*freq,
		amplitude = gDispFactor;
	for (int i = 0; i < numOctaves; ++i) {
		frequency *= 2;
		amplitude *= persistence;
		
		total += InterpolatedNoise( int(mod(0 + i,10)), x * frequency, y * frequency) * amplitude;
	}
	return pow(total, 3.0);
}

vec3 computeNormals(vec3 WorldPos){
	float st = 0.35;
	float dhdu = (perlin((WorldPos.x + st), WorldPos.z) - perlin((WorldPos.x - st), WorldPos.z))/(2.0*st);
	float dhdv = (perlin( WorldPos.x, (WorldPos.z + st)) - perlin(WorldPos.x, (WorldPos.z - st)))/(2.0*st);

	vec3 X = vec3(1.0, dhdu, 1.0);
	vec3 Z = vec3(0.0, dhdv, 1.0);

	vec3 n = normalize(cross(Z,X));
	vec3 norm = mix(n, Normal, 0.5); 
	norm = normalize(norm);
	return norm;
}

vec3 ambient(){
	float ambientStrength = 0.15; 
    vec3 ambient = ambientStrength * u_LightColor; 
    return ambient;
}

vec3 diffuse(vec3 normal){
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float diffuseFactor = max(0.0, dot(lightDir, normal));
	vec3 diffuse = diffuseFactor * u_LightColor;
	return diffuse;
}

vec3 specular(vec3 normal){
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float specularFactor = 0.009f;
	vec3 viewDir = normalize(u_ViewPosition - WorldPos);
	vec3 reflectDir = reflect(-lightDir, normal);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 35.0);
	vec3 specular = spec * u_LightColor; 
	return specular;
}

vec4 getTexture(vec3 normal){
	float trans = 0.5;

	vec4 sand_t = vec4(244, 231, 127, 255)/255;//texture(sand, texCoord*5.0);
	vec4 rock_t = vec4(142, 75, 44, 255)/255;//texture(rock, texCoord*15.0);
	vec4 grass_t = vec4(92, 196, 66, 255)/255;//texture(grass, texCoord*5.0);
	sand_t = texture(sand, texCoord*10.0);
	rock_t = texture(rock, texCoord*35.0);
	rock_t.r *= 1.1;
	grass_t = texture(grass, texCoord*15.0);//*vec4(0.0, 1.5, 0.0, 1.0);
		
	vec4 heightColor;
	float cosV = abs(dot(normal, vec3(0.0, 1.0, 0.0)));
	float grassCoverage = pow(u_grassCoverage, 0.33);
	float tenPercentGrass = grassCoverage - grassCoverage*0.1;
	float blendingCoeff = pow((cosV - tenPercentGrass) / (grassCoverage * 0.1), 4.0);

	if(height <= waterHeight + trans){
		heightColor = sand_t;
    }else if(height <= waterHeight + 2*trans){
		heightColor = mix(sand_t, grass_t, (height - waterHeight - trans) / trans);
    }else if(cosV > grassCoverage){
		heightColor = grass_t;
    }else if(cosV > tenPercentGrass){
		heightColor = mix(rock_t , grass_t , blendingCoeff);
    }else{
		heightColor = rock_t;
	}

	return heightColor;
}

void main()
{
	// calculate fog color 
	vec2 u_FogDist = vec2(800.0, 1400.0);
	float fogFactor = clamp((u_FogDist.y - distFromPos) / (u_FogDist.y - u_FogDist.x), 0.0, 1.0);

	bool normals_fog = true;
	float eps = 0.1;
	if(fogFactor >= 0.0 && fogFactor < eps){
		normals_fog = false;
	}
	
	vec3 n; 
	if(normals && normals_fog){
		n = computeNormals(WorldPos);
	}else{
		n = vec3(0,1,0);
	}

	vec3 ambient = ambient();
	vec3 diffuse = diffuse(n);
	vec3 specular = specular(n);

	vec4 heightColor = getTexture(n);

	// putting all together
    vec4 color = heightColor*vec4((ambient + specular + diffuse)*vec3(1.0) , 1.0f);
	if(drawFog){
		FragColor = mix(color, vec4(fogColor, 1.0f), 1 - fogFactor);
	}else{
		FragColor = color;
	}
};