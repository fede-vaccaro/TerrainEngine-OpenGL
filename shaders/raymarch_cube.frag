#version 330 core
/**
 * Part 3 Challenges
 * - Make the camera move up and down while still pointing at the cube
 * - Make the camera roll (stay looking at the cube, and don't change the eye point)
 * - Make the camera zoom in and out
 */

in vec2 TexCoords;

uniform vec2 iResolution;
uniform float iTime;
uniform mat4 view;
uniform vec3 sunPosition = vec3(1000,1000,1000);
uniform sampler3D cloud;
uniform sampler2D weatherTex;

out vec4 fragColor;

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.001;
const float e = 2.71828182846;
const float PI_r = 0.3183098;

const vec3 cloud_bright = vec3(0.99, 0.96, 0.95);
const vec3 cloud_dark = vec3(0.671, 0.725, 0.753);
const float qLenght = 2.;

const vec3 noiseKernel[1] = vec3[1](vec3(1,-1,0));

// Cloud types height density gradients
#define STRATUS_GRADIENT vec4(0.0, 0.1, 0.2, 0.3)
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)
#define CUMULUS_GRADIENT vec4(0.00, 0.1625, 0.88, 0.98)

float Random2D(in vec3 st)
{
	return fract(sin(dot(st.xyz, vec3(12.9898, 78.233, 57.152))) * 43758.5453123);
}


/**
 * Signed distance function for a cube centered at the origin
 * with width = height = length = 2.0
 */
float cubeSDF(vec3 p) {
    // If d.x < 0, then -1 < p.x < 1, and same logic applies to p.y, p.z
    // So if all components of d are negative, then p is inside the unit cube
    vec3 w = vec3(.0, .0, .0);
	vec3 d = abs(p + w) - vec3(1.0, 1.0, 1.0)*qLenght;
    
    // Assuming p is inside the cube, how far is it from the surface?
    // Result will be negative or zero.
    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
    
    // Assuming p is outside the cube, how far is it from the surface?
    // Result will be positive or zero.
    float outsideDistance = length(max(d, 0.0));
    
    return insideDistance + outsideDistance;
}

/**
 * Signed distance function for a sphere centered at the origin with radius 1.0;
 */
float sphereSDF(vec3 p) {
    return length(p) - 2.5;
}

/**
 * Signed distance function describing the scene.
 * 
 * Absolute value of the return value indicates the distance to the surface.
 * Sign indicates whether the point is inside or outside the surface,
 * negative indicating inside.
 */
float sceneSDF(vec3 samplePoint) {
    return cubeSDF(samplePoint);
}

/**
 * Return the shortest distance from the eyepoint to the scene surface along
 * the marching direction. If no part of the surface is found between start and end,
 * return end.
 * 
 * eye: the eye point, acting as the origin of the ray
 * marchingDirection: the normalized direction to march in
 * start: the starting distance away from the eye
 * end: the max distance away from the ey to march before giving up
 */

 float getHeightFraction(vec3 inPos, vec2 cloudMinMax){
	float height = (inPos.y - cloudMinMax.x)/(cloudMinMax.y - cloudMinMax.x);

	return clamp(height, .0, 1.);

 }



 float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

float getDensityForCloud(float heightFraction, float cloudType)
{
	float stratusFactor = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
	float stratoCumulusFactor = 1.0 - abs(cloudType - 0.5) * 2.0;
	float cumulusFactor = clamp(cloudType - 0.5, 0.0, 1.0) * 2.0;

	vec4 baseGradient = stratusFactor * STRATUS_GRADIENT + stratoCumulusFactor * STRATOCUMULUS_GRADIENT + cumulusFactor * CUMULUS_GRADIENT;

	// gradicent computation (see Siggraph 2017 Nubis-Decima talk)
	return remap(heightFraction, baseGradient.x, baseGradient.y, 0.0, 1.0) * remap(heightFraction, baseGradient.z, baseGradient.w, 1.0, 0.0);
}

float sampleCloudDensity(vec3 p, vec3 weather_data){
	vec4 low_frequency_noise = texture(cloud, p);

	float lowFreqFBM = low_frequency_noise.g*0.625 + low_frequency_noise.b*0.25 + low_frequency_noise.a*0.125;
	float base_cloud = remap(low_frequency_noise.r, 1. - lowFreqFBM, 1., .0, 1.);
	
	float density = getDensityForCloud(p.y, weather_data.b);
	base_cloud *= density;

	float cloud_coverage = weather_data.r;

	float base_cloud_with_coverage = remap(base_cloud , cloud_coverage , 1.0 , 0.0 , 1.0);
	base_cloud_with_coverage *= cloud_coverage;

	return base_cloud_with_coverage;
}

float sampleCloudDensityAlongCone(vec3 p, vec3 dir){
	float density_along_cone = 0.0;

	for(int i = 0; i <= 6; i++)
	{
		vec3 lightStep = dir;
		float coneSpreadMultiplier = length(lightStep);
		p += lightStep + coneSpreadMultiplier * noiseKernel[0] * float(i);

		if(density_along_cone < 0.3)
		{
			density_along_cone += sampleCloudDensity(p, texture(weatherTex, p.xz).rgb);
		}
		else
		{
			density_along_cone += sampleCloudDensity(p, texture(weatherTex, p.xz).rgb);
		}
	}

	return density_along_cone;
}

vec2 march(vec3 o, vec3 d)
{
	float density = 0.0, density_along_cone = 0.0;
	float cloud_test = 0.0;
	int zero_density_sample_count = 0;
	int sample_count = 6;

	for (int i = 0; i < sample_count; i++)
	{
		float depth = 0.0, delta = 0.5;
		if(cloud_test > 0.0)
		{
		vec3 p = o + d*depth;
			float sampled_density = sampleCloudDensity(o + d*depth, texture(weatherTex, p.xz).rgb);
			if(sampled_density == 0.0)
			{
				zero_density_sample_count ++;
			}

			if(zero_density_sample_count != 6)
			{
				density += sampled_density;
				if(sampled_density != 0.0){

					density_along_cone = sampleCloudDensityAlongCone(p, sunPosition - p);

				}
				depth += delta;
			}else
			{
				zero_density_sample_count = 0;
				cloud_test = 0.0;
			}


		}else
		{
			vec3 p = o + d*depth;
			float sampled_density = sampleCloudDensity(o + d*depth, texture(weatherTex, p.xz).rgb);
			cloud_test = sampled_density;
			if(cloud_test == 0.0){
				depth += delta;
			}
		}
		

	}
	return vec2(density, density_along_cone);
}
 float HG(float costheta, float g) {
	float g2 = g*g;
	return 0.25 * PI_r * (1 - g2) / ( 1 + g2 - 2 * g * pow(costheta, 1.5) );
}

 float phase(vec3 inLightVec, vec3 inViewVec) {
	float costheta = dot(inLightVec, inViewVec) / length(inLightVec) / length(inViewVec);
	return HG(costheta, 0.9);
	//return Mie(costheta);
}

float beer(float p, float d){
	return exp(-d*p);
}

float powder(float d){
	return (1. - exp(-2*d));
}

float filterY(float value, float y){
		float a = -30;
		float b = -15;
		float m = 0.1;
		value *= (1. - exp(a*(y - m)))*exp(b*(y - m));
		return value;
}

float cast_scatter_ray(vec3 origin, vec3 dir) {
	float delta = .02;
	int N_STEPS = 50;
	float end = delta*N_STEPS;

	vec3 sp = vec3(0.0);
	float inside = 0.0;
	vec3 camera_pos = vec3(8.0*cos(iTime/5.0), 2.0, 8.0*sin(iTime/5.0));
	float phase_ = phase(dir, vec3(camera_pos - sp));

	for (float t = 0.0; t < end; t += delta) {

		vec3 sp = origin + t*dir; 
		sp = sp/(2*qLenght) + 0.5;
		vec3 weather = texture(weatherTex, sp.xz).rgb;
		float cloudSample = sampleCloudDensity(sp, weather);
		if( cloudSample > 0) inside+=cloudSample;
	}
	float d = 3.0;
	float scatter = 2* exp(-d * inside) * (1.0 - exp(-2*d * inside));
	float value = scatter;
	return value;
}

 vec4 march_in_cloud(vec3 p, vec3 dir, float delta){
	float alpha = 0.0;
	float depth = abs(Random2D(p))/30.0;
	for(int i = 0; i < 1000; i++){
		float dist = sceneSDF(p + dir*depth);
		if(dist < EPSILON){
			vec3 sp = p + depth*dir; 
			sp = sp/(2*qLenght) + 0.5;
			vec3 weather = texture(weatherTex, sp.xz).rgb;
			//float cloudSample = sampleCloudDensity(sp, weather)*100.0;
			float cloudSample = texture(cloud, sp).r;
			if( cloudSample > 0) alpha+=cloudSample;
			if(alpha >= 1.0){
				return vec4(sp, 1.0);
			}
		}else{
			return vec4(0,0,0, alpha);
		}
		depth += delta;
	}
	return vec4(0.0);
 }

vec4 shortestDistanceToSurface(vec3 eye, vec3 marchingDirection, float start, float end) {
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        float dist = sceneSDF(eye + depth * marchingDirection);
        if (dist < EPSILON) {
			vec4 alpha = march_in_cloud(eye + depth*marchingDirection, marchingDirection, 0.005);
			return alpha;
        }
        depth += dist;
        if (depth >= end) {
            return vec4(0.0);
        }
    }

    return vec4(0.0);
}
            

/**
 * Return the normalized direction to march in from the eye point for a single pixel.
 * 
 * fieldOfView: vertical field of view in degrees
 * size: resolution of the output image
 * fragCoord: the x,y coordinate of the pixel in the output image
 */
vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord) {
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fieldOfView) / 2.0);
    return normalize(vec3(xy, -z));
}

mat4 viewMatrix(vec3 eye, vec3 center, vec3 up) {
    // Based on gluLookAt man page
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    return mat4(
        vec4(s, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(0.0, 0.0, 0.0, 1)
    );
}

void main()
{
	vec2 fragCoord = gl_FragCoord.xy;

	vec3 viewDir = rayDirection(30.0, iResolution.xy, fragCoord);
    vec3 eye = vec3(8.0*cos(iTime/5.0), 2.0, 8.0*sin(iTime/5.0));
    
    mat4 viewToWorld = viewMatrix(eye, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    
    vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
    
    vec4 pos_alpha = shortestDistanceToSurface(eye, worldDir, MIN_DIST, MAX_DIST);
    
	if (pos_alpha.a == 1.0){
		float energy = cast_scatter_ray( pos_alpha.xyz, normalize(sunPosition - pos_alpha.xyz));
		vec3 color = mix(cloud_dark, cloud_bright, energy).xyz;
		float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.0) energy = energy/(brightness + energy);

		fragColor =  vec4(mix(cloud_dark, cloud_bright, energy),1.0);
		return;
	}

    fragColor = vec4( cloud_dark*vec3(pow(pos_alpha.a, 2.0)), 1.0);
}