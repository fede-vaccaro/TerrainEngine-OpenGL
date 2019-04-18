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
uniform vec3 gEyeWorldPos;
uniform vec3 u_ViewPosition;
uniform vec3 fogColor;
uniform vec3 seed;
uniform vec2 offset;
uniform int octaves;
uniform bool drawFog;
uniform float gDispFactor;
uniform float freq;
uniform bool normals;
uniform float u_grassCoverage;
uniform float waterHeight;

uniform sampler2D sand, grass1, grass, rock, snow, rockNormal;

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
	return fract(sin(dot(st.xy, vec2(12.9898, 78.233) + seed.xy)) * 43758.5453123);
}

float Interpolate(float a, float b, float x) {  // cosine interpolation
	float ft = x * 3.1415927f;
	float f = (1 - cos(ft)) * 0.5;
	return  a * (1 - f) + b * f;
}

float InterpolatedNoise(vec2 xy) {
	float x = xy.x, y = xy.y;
	int integer_X = int(floor(x));
	float fractional_X = fract(x);
	int integer_Y = int(floor(y));
	float fractional_Y = fract(y);
	vec2 randomInput = vec2(integer_X, integer_Y);
	float a = Random2D(randomInput);
	float b = Random2D(randomInput + vec2(1.0, 0.0));
	float c = Random2D(randomInput + vec2(0.0, 1.0));
	float d = Random2D(randomInput + vec2(1.0, 1.0));

	vec2 w = vec2(fractional_X, fractional_Y);
	w = w*w*w*(10.0 + w*(-15.0 + 6.0*w));

	float k0 = a, 
	k1 = b - a, 
	k2 = c - a, 
	k3 = d - c - b + a;

	return k0 + k1*w.x + k2*w.y + k3*w.x*w.y;

}

/**
float perlin(float x, float y){
	
    int numOctaves = octaves;
	float persistence = 0.5;
	float total = 0,
		frequency = 0.005*freq,
		amplitude = gDispFactor;
	for (int i = 0; i < numOctaves; ++i) {
		frequency *= 2;
		amplitude *= persistence;
		
		total += InterpolatedNoise( vec2(x,y)*frequency) * amplitude;
	}
	return total*total*total;
} **/

const mat2 m = mat2(0.8,-0.6,0.6,0.8);

/**
float perlin(float x, float y){
    
	vec2 st = vec2(x,y);
    //st *= freq;
    
    //int numOctaves = 10;
	float persistence = 0.5;
	float total = 0.0,
		frequency = 0.25,
		amplitude = gDispFactor;
	for (int i = 0; i < octaves; ++i) {
		frequency *= 2.0;
		amplitude *= persistence;
        st = frequency*m*st;
		total += InterpolatedNoise(st) * amplitude;
	}
	return total;
}
**/

float smoothstepd(float x){
 	return 6.0*x*(1.0 - x);   
}
vec2 smoothstepd( float a, float b, float x)
{
	if( x<a ) return vec2( 0.0, 0.0 );
	if( x>b ) return vec2( 1.0, 0.0 );
    float ir = 1.0/(b-a);
    x = (x-a)*ir;
    return vec2( x*x*(3.0-2.0*x), 6.0*x*(1.0-x)*ir );
}



vec2 InterpolatedNoiseD(vec2 xy) {
	float x = xy.x, y = xy.y;
 	int integer_X = int(floor(x));
	float fractional_X = fract(x);
	int integer_Y = int(floor(y));
	float fractional_Y = fract(y);
    
	vec2 randomInput = vec2(integer_X, integer_Y);
	float a = Random2D(randomInput + vec2(0.0, 0.0));
	float b = Random2D(randomInput + vec2(1.0, 0.0));
	float c = Random2D(randomInput + vec2(0.0, 1.0));
	float d = Random2D(randomInput + vec2(1.0, 1.0));
    
    //fractional_X = smoothstep(0.0, 1.0, fractional_X);
    //fractional_Y = smoothstep(0.0, 1.0, fractional_Y);
	float k0 = a, 
	k1 = b - a, 
	k2 = c - a, 
	k3 = d - c - b + a;
	
	float dndx = (k1 + k3*fractional_Y)*smoothstepd(fractional_X);
	float dndy = (k2 + k3*fractional_X)*smoothstepd(fractional_Y);

	return vec2(dndx, dndy);
}

uniform float power;

float perlin(float x, float y){
    
    //st *= freq;
    vec2 st = vec2(x,y);
    //int numOctaves = 10;
	float persistence = 0.5;
	float total = 0.0,
		frequency = 0.005*freq,
		amplitude = gDispFactor;
	for (int i = 0; i < octaves; ++i) {
		frequency *= 2.0;
		amplitude *= persistence;

        //st = frequency*m*st;

		vec2 v = frequency*m*st;

		total += InterpolatedNoise(v) * amplitude;
	}
	return pow(total, power);
}


vec3 computeNormals(vec3 WorldPos, out mat3 TBN){
	float st = 1.0;
	float dhdu = (perlin((WorldPos.x + st), WorldPos.z) - perlin((WorldPos.x - st), WorldPos.z))/(2.0*st);
	float dhdv = (perlin( WorldPos.x, (WorldPos.z + st)) - perlin(WorldPos.x, (WorldPos.z - st)))/(2.0*st);

	vec3 X = vec3(1.0, dhdu, 1.0);
	vec3 Z = vec3(0.0, dhdv, 1.0);

	vec3 n = normalize(cross(Z,X));
	TBN = mat3(normalize(X), normalize(Z), n);

	//vec3 norm = mix(n, Normal, 0.5); 
	//norm = normalize(norm);
	return n;
}

vec3 ambient(){
	float ambientStrength = 0.2; 
    vec3 ambient = ambientStrength * u_LightColor; 
    return ambient;
}

vec3 computeNormals(vec2 gradient){
	vec3 X = vec3(1.0, gradient.r, 0.0);
	vec3 Z = vec3(0.0, gradient.g, 1.0);

	vec3 n = normalize(cross(Z,X));

	//n = normalize( vec3(-gradient.r, 1.0, -gradient.g));

	return n;
}

vec3 diffuse(vec3 normal){
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float diffuseFactor = max(0.0, dot(lightDir, normal));
	const float diffuseConst = 0.75;
	vec3 diffuse = diffuseFactor * u_LightColor * diffuseConst;
	return diffuse;
}

vec3 specular(vec3 normal){
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float specularFactor = 0.01f;
	vec3 viewDir = normalize(u_ViewPosition - WorldPos);
	vec3 reflectDir = reflect(-lightDir, normal);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	vec3 specular = spec * u_LightColor*specularFactor; 
	return specular;
}

float perlin(float x, float y, int oct){
	
	vec2 distances = vec2(500.0, 2000.0);
	int distanceFactor = int(clamp( (distances.y - distFromPos)*3.0 / (distances.y - distances.x), 0.0, 3.0));
	distanceFactor = 3 - distanceFactor;

    int numOctaves = oct;
	float persistence = 0.5;
	float total = 0,
		frequency = 0.05*freq,
		amplitude = 1.0;
	for (int i = 0; i < numOctaves; ++i) {
		frequency *= 2;
		amplitude *= persistence;
		
		total += InterpolatedNoise(vec2(x,y)*frequency) * amplitude;
	}
	return total;
}


uniform vec3 rockColor;

vec4 getTexture(inout vec3 normal, const mat3 TBN){
	float trans = 20.;

	vec4 sand_t = vec4(244, 231, 127, 255)/255;//texture(sand, texCoord*5.0);
	vec4 rock_t = vec4(rockColor,1.0);//texture(rock, texCoord*15.0);
	vec4 grass_t = vec4(92, 196, 66, 255)/255;//texture(grass, texCoord*5.0);



	sand_t = texture(sand, texCoord*10.0);
	sand_t.rg *= 1.3;
	rock_t = texture(rock, texCoord*vec2(1.0, 1.256).yx);
	rock_t.rgb *= vec3(2.5, 2.0, 2.0);
	grass_t = texture(grass, texCoord*12.0);//*vec4(0.0, 1.5, 0.0, 1.0);
	vec4 grass_t1 = texture(grass1, texCoord*12.0);//*
	float perlinBlendingCoeff = clamp(perlin(WorldPos.x, WorldPos.z, 2)*2.0 - 0.2, 0.0, 1.0);
	grass_t = mix(grass_t*1.3, grass_t1*0.75, perlinBlendingCoeff);
	grass_t.rgb *= 0.5;
	//rock_t = mix(rock_t*0.7, rock_t*0.9, perlinBlendingCoeff);
	//grass_t = vec4(grassBlendingCoeff);

	float grassCoverage = u_grassCoverage;//pow(u_grassCoverage, 0.33);

	/*
	float transMultiplier = 4.0;
	float snowHeight = gDispFactor*gDispFactor*gDispFactor*0.3 +  1800.0  - perlinBlendingCoeff*600.0*3.;
	if( WorldPos.y > snowHeight - trans*transMultiplier && WorldPos.y < snowHeight + trans*transMultiplier ){
		float gradient = clamp((WorldPos.y - (snowHeight - trans*transMultiplier))/(2.0*trans*transMultiplier), 0.0, 1.0);
		grass_t.rgb = mix(grass_t.rgb, texture(snow, texCoord*5.0).rgb*1.35, gradient);
		grassCoverage = mix(grassCoverage, grassCoverage - 0.12, gradient);
	}else if(WorldPos.y > snowHeight + trans*transMultiplier){
		grass_t.rgb =texture(snow, texCoord*5.0).rgb*1.35;
		grassCoverage = grassCoverage - 0.12;
	}
	*/
	vec4 heightColor;
	float cosV = abs(dot(normal, vec3(0.0, 1.0, 0.0)));
	float tenPercentGrass = grassCoverage - grassCoverage*0.1;
	float blendingCoeff = pow((cosV - tenPercentGrass) / (grassCoverage * 0.1), 1.0);

	if(height <= waterHeight + trans){
		heightColor = sand_t;
    }else if(height <= waterHeight + 2*trans){
		heightColor = mix(sand_t, grass_t, pow( (height - waterHeight - trans) / trans, 1.0));
    }else if(cosV > grassCoverage){
		heightColor = grass_t;
		mix(normal, vec3(0.0, 1.0, 0.0), 0.25);
    }else if(cosV > tenPercentGrass){
		heightColor = mix(rock_t , grass_t , blendingCoeff);
		normal = mix(TBN*(texture(rockNormal, texCoord*vec2(2.0, 2.5).yx).rgb*2.0 - 1.0), normal, blendingCoeff);
    }else{
		heightColor = rock_t;
		normal = TBN*(texture(rockNormal, texCoord*vec2(2.0, 2.5).yx).rgb*2.0 - 1.0);
		
	}

	return heightColor;
}

uniform float fogFalloff;

const float c = 18.;
const float b = 3.e-6;

float applyFog( in vec3  rgb,      // original color of the pixel
               in float dist, // camera to point distance
               in vec3  cameraPos,   // camera position
               in vec3  rayDir )  // camera to point vector
{
    float fogAmount = c * exp(-cameraPos.y*fogFalloff) * (1.0-exp( -dist*rayDir.y*fogFalloff ))/rayDir.y;
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return clamp(fogAmount,0.0,1.0);//mix( rgb, fogColor, fogAmount );
    //return fogAmount;//mix( rgb, fogColor, fogAmount );
}


void main()
{
	// calculate fog color 
	vec2 u_FogDist = vec2(2500.0, 10000.0);
	//float fogFactor = clamp((u_FogDist.y - distFromPos) / (u_FogDist.y - u_FogDist.x), 0.0, 1.0);
	//float fogFactor = clamp(exp(-1.5*distFromPos/(u_FogDist.y - u_FogDist.x) + 0.5), 0.0, 1.0);
	bool normals_fog = true;
	float fogFactor = applyFog(vec3(0.0), distance(gEyeWorldPos, WorldPos), gEyeWorldPos, normalize(WorldPos - gEyeWorldPos));
	float eps = 0.1;
	if(fogFactor >= 0.0 && fogFactor > 1. - eps){
		//normals_fog = false;
	}
	
	vec3 n;
	mat3 TBN;
	if(normals && normals_fog){
		//n = computeNormals(fbmd_9(WorldPos.xz).gb);
		n = computeNormals(WorldPos, TBN);
		//smoothing
		/**float st = 0.1;
		vec3 n1 = computeNormals(WorldPos + vec3(-st, 0, st));
		vec3 n3 = computeNormals(WorldPos + vec3(0, 0, st));
		vec3 n2 = computeNormals(WorldPos + vec3(st, 0, st));
		vec3 n4 = computeNormals(WorldPos + vec3(-st, 0, 0));
		vec3 n5 = computeNormals(WorldPos + vec3(st, 0, 0));
		vec3 n6 = computeNormals(WorldPos + vec3(0, 0, -st));
		vec3 n7 = computeNormals(WorldPos + vec3(-st, 0, -st));
		vec3 n8 = computeNormals(WorldPos + vec3(st, 0, -st));
		**/
		//n = n + n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8;
		n = normalize(n);
	}else{
		n = vec3(0,1,0);
	}



	vec4 heightColor = getTexture(n, TBN);
	//heightColor = vec4(perlin(WorldPos.x, WorldPos.z, 4));
	
	vec3 ambient = ambient();
	vec3 diffuse = diffuse(n);
	vec3 specular = specular(n);


	// putting all together
    vec4 color = heightColor*vec4((ambient + specular*0 + diffuse)*vec3(1.0f) , 1.0f);
	if(drawFog){
		FragColor = mix(color, vec4(mix(fogColor*1.1,fogColor*0.85,clamp(WorldPos.y/(1500.*16.)*gDispFactor,0.0,1.0)), 1.0f), fogFactor);
		FragColor.a = WorldPos.y/waterHeight;
	}else{
		FragColor = color;
		FragColor.a = WorldPos.y/waterHeight;
	}

	//FragColor.rgb = n*0.5 + 0.5;
};