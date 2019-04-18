#version 330 core                                                                               

// Thanks to ThinMatrix for his water tutorial! https://www.youtube.com/watch?v=HusvGeEDU_U&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh


in vec2 TexCoords;
in vec3 Normal;
in vec4 clipSpaceCoords;
in vec4 position;

uniform float moveFactor;


uniform vec3 cameraPosition;
uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D waterDUDV;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

out vec4 FragColor;

const float distFactor = 0.05;

float Random3D(in vec3 st)
{
	return fract(sin(dot(st.xyz, vec3(12.9898, 78.233, 141.1525))) * 43758.5453123);
}

float Interpolate(float a, float b, float x) {  // cosine interpolation
	float ft = x * 3.1415927f;
	float f = (1. - cos(ft)) * 0.5;
	return  a * (1. - f) + b * f;
}

float InterpolatedNoise(int ind, float x, float y, float z) {
	int integer_X = int(floor(x));
	float fractional_X = fract(x);
	int integer_Y = int(floor(y));
	float fractional_Y = fract(y);
    int integer_Z = int(floor(z));
    float fractional_Z = fract(z);
    
	vec3 randomInput = vec3(integer_X, integer_Y, integer_Z);
	float v1 = Random3D(randomInput + vec3(0.0, 0.0, 0.0));
	float v2 = Random3D(randomInput + vec3(1.0, 0.0, 0.0));
	float v3 = Random3D(randomInput + vec3(0.0, 1.0, 0.0));
	float v4 = Random3D(randomInput + vec3(1.0, 1.0, 0.0));
    
    float v5 = Random3D(randomInput + vec3(0.0, 0.0, 1.0));
	float v6 = Random3D(randomInput + vec3(1.0, 0.0, 1.0));
	float v7 = Random3D(randomInput + vec3(0.0, 1.0, 1.0));
	float v8 = Random3D(randomInput + vec3(1.0, 1.0, 1.0));
    
    
	float i1 = Interpolate(v1, v2, fractional_X);
	float i2 = Interpolate(v3, v4,  fractional_X);
    
    float i3 = Interpolate(v5, v6, fractional_X);
    float i4 = Interpolate(v7, v8, fractional_X);
    
    float y1 = Interpolate(i1, i2, fractional_Y);
    float y2 = Interpolate(i3, i4, fractional_Y);
    
    
	return Interpolate(y1, y2, fractional_Z);
}

float perlin(float x, float y, float z){
	
    int numOctaves = 3;
	float persistence = 0.5;
	float total = 0.,
		frequency = 0.025,
		amplitude = 6.0;
	for (int i = 0; i < numOctaves; ++i) {
		frequency *= 2.;
		amplitude *= persistence;
		
		x+=moveFactor*13.0;
		y+=moveFactor*7.0;

		total += InterpolatedNoise(0, x * frequency, y * frequency, z * frequency) * amplitude;
		//total += InterpolatedNoise(0, y * frequency, x * frequency, (z+0.5) * frequency) * amplitude;
	}
	return pow(total, 1.0);
}

vec3 computeNormals(vec3 WorldPos){
	float st = 0.35;
	float dhdu = (perlin((WorldPos.x + st), WorldPos.z, moveFactor*10.0) - perlin((WorldPos.x - st), WorldPos.z, moveFactor*10.0))/(2.0*st);
	float dhdv = (perlin( WorldPos.x, (WorldPos.z + st), moveFactor*10.0) - perlin(WorldPos.x, (WorldPos.z - st), moveFactor*10.0))/(2.0*st);

	vec3 X = vec3(1.0, dhdu, 1.0);
	vec3 Z = vec3(0.0, dhdv, 1.0);

	vec3 n = normalize(cross(Z,X));
	vec3 norm = mix(n, Normal, 0.5); 
	norm = normalize(norm);
	return norm;
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1,311.7, 194.1729)),dot(p,vec3(269.5,183.3, 72.0192)), dot(p,vec3(183.3,72.0192,311.7))))*43758.5453);
}

float worley(vec3 st) {
	float color = 0.0;

    // Scale
    st *= 0.5;

    // Tile the space
    vec3 i_st = floor(st);
    vec3 f_st = fract(st);

    float m_dist = 1.;  // minimun distance

    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
		for (int z = -1; z<=1 ; z++) {

            // Neighbor place in the grid
            vec3 neighbor = vec3(float(x),float(y), float(z));

            // Random position from current + neighbor place in the grid
            vec3 point = random3(i_st + neighbor);


			// Vector between the pixel and the point
            vec3 diff = neighbor + point - f_st;

            // Distance to the point
            float dist = pow(length(diff), 1.0);

            // Keep the closer distance
            m_dist = min(m_dist, dist);
			}
        }
    }

    // Draw the min distance (distance field)
    color += m_dist;

    return color;
}

#define SATURATE(X) clamp(X, 0.0, 1.0)


void main(){
	float distFromPos = distance(position.xyz, cameraPosition); 
	vec2 u_FogDist = vec2(4000.0, 12000.0);
	float fogFactor = SATURATE((u_FogDist.y - distFromPos) / (u_FogDist.y - u_FogDist.x));

	float grain = 50.0;
	vec2 ndc = (clipSpaceCoords.xy/clipSpaceCoords.w)/2.0 + 0.5;
	vec2 distortion1 = texture(waterDUDV, vec2(TexCoords.x + moveFactor, TexCoords.y)*grain).rg*2.0 - 1.0;
	vec2 distortion2 = texture(waterDUDV, vec2(TexCoords.x + moveFactor, TexCoords.y - moveFactor)*grain).rg*2.0 - 1.0;
	vec2 totalDistortion = distortion1 + distortion2;
	float st = 0.1;
	float dhdu = (perlin((position.x + st), position.z, moveFactor*10.0) - perlin((position.x - st), position.z, moveFactor*10.0))/(2.0*st);
	float dhdv = (perlin( position.x, (position.z + st), moveFactor*10.0) - perlin(position.x, (position.z - st), moveFactor*10.0))/(2.0*st);

	float floorY = texture(refractionTex, ndc).a;
	float waterDepth = 1.0 - floorY;
	float waterDepthClamped = SATURATE(waterDepth*5.0);

	totalDistortion = vec2(dhdu, dhdv)*distFactor*waterDepth;

	vec2 reflectionTexCoords = vec2(ndc.x, -ndc.y);
	reflectionTexCoords += totalDistortion;
	reflectionTexCoords.x = clamp(reflectionTexCoords.x, 0.001, 0.999);
	reflectionTexCoords.y = clamp(reflectionTexCoords.y, -0.999, -0.001);
	vec4 reflectionColor = texture(reflectionTex, reflectionTexCoords);

	vec2 refractionTexCoords = ndc;
	
	//float near = 10.0;
	//float far = 10000.0;
	//float depth = texture(depthMap, refractionTexCoords).r;
	//float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	//float waterDistance = 2.0 * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));
	//float waterDepth = floorDistance - waterDistance;


	refractionTexCoords += totalDistortion;
	refractionTexCoords = clamp(refractionTexCoords, 0.001, 0.999);
	vec4 refractionColor = texture(refractionTex, refractionTexCoords);


	vec3 toCameraVector =  position.xyz - cameraPosition;
	float fresnelFactor = max(dot(normalize(-toCameraVector), vec3(0.0, 1.0, 0.0)), 0.0);
	fresnelFactor = pow(fresnelFactor, 3.0);
	vec4 refr_reflCol = mix(reflectionColor, refractionColor, fresnelFactor);


	// calculate diffuse illumination
	totalDistortion = normalize(totalDistortion);
	vec3 X = vec3(1.0, totalDistortion.r, 1.0);
	vec3 Z = vec3(0.0, totalDistortion.g, 1.0);
	vec3 norm = texture(normalMap, totalDistortion).rgb;
	norm = vec3(norm.r*2 - 1, norm.b*1.5, norm.g*2 - 1);
	//norm = normalize(cross(X, Z));
	norm = computeNormals(position.xyz);
	norm = mix(norm, vec3(0.0, 1.0, 0.0), 0.25);
	vec3 lightDir = normalize(u_LightPosition - position.xyz);
	float diffuseFactor = max(0.0, dot(lightDir, norm.rgb));
	float diffuseConst = 0.5;
	vec3 diffuse = diffuseFactor*diffuseConst*u_LightColor;

	// calculate specular illumination 
	float specularFactor = 1.5f;
	vec3 viewDir = normalize(cameraPosition - position.xyz);
	vec3 reflectDir = reflect(-lightDir,  normalize(mix(norm, normalize(viewDir*0.8 + vec3(0.0, 1.0, 0.0)), 0.2)) );  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 512.);
	vec3 specular = spec * u_LightColor * specularFactor;  


	vec4 color = vec4(0.2,0.71,0.85, 1.0);

	vec4 fogColor = vec4(0.4,0.6,0.75, 1.0);
	//refr_reflCol *= fogColor;
	FragColor =  mix(mix(refr_reflCol, color*0.8, 0.1)*0.8 + vec4(diffuse + specular, 1.0) , fogColor,(1 - fogFactor));
	//float worley_ = worley( vec3(position.xz, moveFactor*10.0))*0.5 + worley( vec3(position.xz*2.0, moveFactor*5.0))*0.25;
	float foam = perlin(position.x*4.0, position.z*4.0, moveFactor*10.0  )*0.25;
	foam = mix(   foam*pow((1.0 - waterDepth), 8.0), foam*0.01, 0.0);
	FragColor.rgb *= 0.95;
	//FragColor.rgb += foam;
	FragColor.a = waterDepthClamped;
	}