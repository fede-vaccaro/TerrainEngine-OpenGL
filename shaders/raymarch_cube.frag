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
uniform mat4 proj;
uniform vec3 sunPosition = vec3(1000,1000,1000)*20.;
uniform vec3 lightDir = normalize(vec3(1,10,1));
uniform sampler3D cloud;
uniform sampler3D worley32;
uniform sampler2D weatherTex;
uniform vec3 cameraPosition;
vec3 sphereCenter = vec3(0.0);

out vec4 fragColor;

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.001;
const float e = 2.71828182846;
const float PI_r = 0.3183098;

const vec3 cloud_bright = vec3(0.99, 0.96, 0.95);
const vec3 cloud_dark = vec3(0.671, 0.725, 0.753);
float qLenght = 5*500*50;

// Cone sampling random offsets
uniform vec3 noiseKernel[6u] = vec3[] 
(
	vec3( 0.38051305,  0.92453449, -0.02111345),
	vec3(-0.50625799, -0.03590792, -0.86163418),
	vec3(-0.32509218, -0.94557439,  0.01428793),
	vec3( 0.09026238, -0.27376545,  0.95755165),
	vec3( 0.28128598,  0.42443639, -0.86065785),
	vec3(-0.16852403,  0.14748697,  0.97460106)
);

vec3 planeMin = vec3(-1.1,0.35 + 0.0,-2.2 -0.2)*qLenght;
vec3 planeMax = vec3(1.1,0.35 + 0.6,-0.2)*qLenght;
vec2 planeDim = vec2(planeMax.xz - planeMin.xz);
vec3 planeDim_ = vec3(planeMax - planeMin);


// Cloud types height density gradients
#define STRATUS_GRADIENT vec4(0.0, 0.1, 0.2, 0.3)
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)
#define CUMULUS_GRADIENT vec4(0.00, 0.1625, 0.88, 0.98)

#define SPHERE_INNER_RADIUS 2000.0
#define SPHERE_OUTER_RADIUS 2125.0
#define SPHERE_DELTA float(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS)


float chunkLen;

bool intersectSphere(vec3 o, vec3 d, out vec3 minT, out vec3 maxT)
{
	// Intersect inner sphere
	vec3 sphereToOrigin = o - sphereCenter;
	float b = dot(d, sphereToOrigin);
	float c = dot(sphereToOrigin, sphereToOrigin);
	float sqrtOpInner = b*b - (c - SPHERE_INNER_RADIUS*SPHERE_INNER_RADIUS);

	// No solution (we are outside the sphere, looking away from it)
	float maxSInner;
	if(sqrtOpInner < 0.0)
	{
		return false;
		//maxSInner = 0.0;
	}
	else
	{
		float deInner = sqrt(sqrtOpInner);
		float solAInner = -b - deInner;
		float solBInner = -b + deInner;

		maxSInner = max(solAInner, solBInner);
		maxSInner = maxSInner < 0.0? 0.0 : maxSInner;
	}
	// Intersect outer sphere
	float sqrtOpOuter = b*b - (c - SPHERE_OUTER_RADIUS*SPHERE_OUTER_RADIUS);

	// No solution - same as inner sphere
	if(sqrtOpOuter < 0.0)
	{
		return false;
	}
	
	float deOuter = sqrt(sqrtOpOuter);
	float solAOuter = -b - deOuter;
	float solBOuter = -b + deOuter;

	float maxSOuter = max(solAOuter, solBOuter);
	maxSOuter = maxSOuter < 0.0? 0.0 : maxSOuter;

	// Compute entering and exiting ray points
	float minSol = min(maxSInner, maxSOuter);
	float maxSol = max(maxSInner, maxSOuter);

	minT = o + d * minSol;
	maxT = o + d * maxSol;

	chunkLen = length(maxT - minT);

	return minT.y > -500.0; // only above the horizon
}


bool intersectBox(vec3 o, vec3 d, out vec3 minT, out vec3 maxT)
{

	// compute intersection of ray with all six bbox planes
	vec3 invR = 1.0 / d;
	vec3 tbot = invR * (planeMin - o);
	vec3 ttop = invR * (planeMax - o);
	// re-order intersections to find smallest and largest on each axis
	vec3 tmin = min (ttop, tbot);
	vec3 tmax = max (ttop, tbot);
	// find the largest tmin and the smallest tmax
	vec2 t0 = max (tmin.xx, tmin.yz);
	float tnear = max (t0.x, t0.y);
	t0 = min (tmax.xx, tmax.yz);
	float tfar = min (t0.x, t0.y);
	
	// check for hit
	bool hit;
	if ((tnear > tfar) || tfar < 0.0)
		hit = false;
	else
		hit = true;

	minT = tnear < 0.0? o : o + d * tnear; // if we are inside the bb, start point is cam pos
	maxT = o + d * tfar;

	return hit;
}


float Random2D(in vec3 st)
{
	return fract(sin(iTime*dot(st.xyz, vec3(12.9898, 78.233, 57.152))) * 43758.5453123);
}


/**
 * Signed distance function for a cube centered at the origin
 * with width = height = length = 2.0
 */
float cubeSDF(vec3 p, vec3 w) {
    // If d.x < 0, then -1 < p.x < 1, and same logic applies to p.y, p.z
    // So if all components of d are negative, then p is inside the unit cube
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
    return length(p) - qLenght;
}

/**
 * Signed distance function describing the scene.
 * 
 * Absolute value of the return value indicates the distance to the surface.
 * Sign indicates whether the point is inside or outside the surface,
 * negative indicating inside.
 */
float sceneSDF(vec3 samplePoint) {
    return sphereSDF(samplePoint);
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

 float filterY(float value, float y){
		float a = -8;
		float b = -4;
		float m = 0.1;
		value *= (1. - exp(a*(y - m)))*exp(b*(y - m));
		return (value >= 0 ? value : 0);
}

float gauss(float x, float x0, float sx){
    
    float arg = x-x0;
    arg = -1./2.*arg*arg/sx;
    
    float a = 1./(pow(2.*3.1415*sx, 0.5));
    
    return exp(arg);
}

vec2 cloudMinMax = vec2(planeMin.y, planeMax.y);

 float getHeightFraction(vec3 inPos){
	
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

float threshold(float v, float t)
{
	return v > t ? v : 0.0;
}


float sampleCloudDensity(vec3 p){

	vec3 uvw = (p - planeMin)/planeDim_;

	vec2 uv = (p.xz - planeMin.xz) / planeDim;

	//uv *= 2.0;
	vec2 uv_scaled = uv*3.0;

	float heightFraction = getHeightFraction(p);

	vec4 low_frequency_noise = texture(cloud, vec3(uv_scaled, heightFraction));

	vec3 weather_data = texture(weatherTex, uv).rgb;

	float lowFreqFBM = low_frequency_noise.g*0.625 + low_frequency_noise.b*0.25 + low_frequency_noise.a*0.125;
	float base_cloud = remap(2.*low_frequency_noise.r, -(1.0 - lowFreqFBM), 1., 0.0 , 1.0);
	
	//heightFraction = clamp(heightFraction, -heightFraction - EPSILON, heightFraction + EPSILON);
	float density = getDensityForCloud(heightFraction, weather_data.g);
	base_cloud *= density/heightFraction;

	float cloud_coverage = weather_data.r*0.15;
	float base_cloud_with_coverage = remap(base_cloud , cloud_coverage , 1.0 , 0.0 , 1.0);
	base_cloud_with_coverage *= cloud_coverage;
	


	bool expensive = true;
	
	if(expensive)
	{
		vec3 erodeCloudNoise = texture(worley32, vec3(uv_scaled*1.5, heightFraction)*0.1 ).rgb;
		float highFreqFBM = (erodeCloudNoise.r * 0.625) + (erodeCloudNoise.g * 0.25) + (erodeCloudNoise.b * 0.125);
		float highFreqNoiseModifier = mix(highFreqFBM, 1.0 - highFreqFBM, clamp(heightFraction * 5.0, 0.0, 1.0));

		base_cloud_with_coverage = base_cloud_with_coverage - highFreqNoiseModifier * (1.0 - base_cloud_with_coverage);

		base_cloud_with_coverage = remap(base_cloud_with_coverage, highFreqNoiseModifier * 0.2, 1.0, 0.0, 1.0);
	}

	base_cloud_with_coverage = threshold(base_cloud_with_coverage, 0.05);

	return clamp(base_cloud_with_coverage*2.0, 0.0, 1.0);
}


 float HG(float costheta, float g) {
	float g2 = g*g;
	return 0.25 * PI_r * (1 - g2) / ( pow(1 + g2 - 2 * g * costheta, 1.5) );
}



float beer(float d){
	return exp(-d);
}

float powder(float d, float lightDotEye){
	float powd = (1. - exp(-2*d));
	return mix(1.0, powd, clamp( (-lightDotEye*0.5) + 0.5, 0.0,1.0));
}


 float phase(vec3 inLightVec, vec3 inViewVec, float g) {
	float costheta = dot(inLightVec, inViewVec) / length(inLightVec) / length(inViewVec);
	return HG(costheta, g);
	//return Mie(costheta);
}

    vec3 eye = cameraPosition;//vec3(50.0*cos(iTime/5.0), 30, 50.0*sin(iTime/5.0));
	

//#define CONE_STEP 0.1666666
//#define CONE_STEP 0.1

float raymarchToLight(vec3 o, float stepSize, vec3 lightDir, float originalDensity, float lightDotEye)
{

	vec3 startPos = o;
	vec3 rayStep = lightDir * stepSize * 6.0;
	const float CONE_STEP = 1.0/6.0;
	float coneRadius = 1.0; 
	float density = 0.0;
	float coneDensity = 0.0;
	float invDepth = 1.0/(stepSize*6.0);
	

	for(int i = 0; i < 6; i++)
	{
		vec3 posInCone = startPos + coneRadius*noiseKernel[i]*float(i);

		float heightFraction = getHeightFraction(posInCone);
		if(heightFraction >= 0)
		{
			float cloudDensity = sampleCloudDensity(posInCone);
			if(cloudDensity > 0.0)
			{
				density += cloudDensity;
				float trasmittance = 1.0 - (density * invDepth);
				coneDensity += (cloudDensity * trasmittance);
			}
		}
		startPos += rayStep;
		coneRadius += CONE_STEP;
	}

	//float phase = phase(lightDir, -vec3(cameraPosition - o), 0.1);
	//float lightDotEye = dot(lightDir, -normalize(cameraPosition));
	float phase1 = HG(0.985, 0.8);
	float phase2 = HG(0.985, -0.5);
	float phase = phase1*0.5 + phase2*0.5;

	//float powder1 = powder(coneDensity, -0.45);
	//float powder2 = powder(originalDensity, -0.25);
	//float powder_ = powder1*powder2*2.0;
	//foat pow = powder(originalDensity, -0.6);

	float energy = mix(beer(coneDensity), 1.0, 0.25)*powder(originalDensity*coneDensity, -0.25)*phase*2.0;
	return clamp(energy, 0.0, 1.0);
}

vec3 ambientlight = vec3(255, 240, 230)/255;

float ambientFactor = 1.0;
vec3 lc = ambientlight * ambientFactor;// * cloud_bright;

vec3 ambient_light(float heightFrac)
{
	return mix( vec3(0.5, 0.67, 0.82), vec3(1.0), heightFrac);
}

/**
vec4 march_(vec3 o, vec3 d)
{
	float density = 0.0, density_along_cone = 0.0;
	float cloud_test = 0.0;
	int zero_density_sample_count = 0;
	float dist = 0;
	int sample_count = 300;
	float depth = 0.0, delta = 0.075*qLenght/120.0;
	bool backfront = false;
	bool entered = false;

	vec4 col = vec4(0.0);

	for (int i = 0; i < sample_count; i++)
	{
	 vec3 sp = o + d*depth;
	 float density_sample = simpleSample(o + d*depth);
	 if(density_sample > 0.0){

		depth += -delta*0.5;
		backfront = true;

		density += density_sample;
		float light_density = cast_scatter_ray(sp, sunPosition - sp);
		float height = getHeightFraction(sp);
		vec4 src = vec4(lc * 0.7 * light_density + ambient_light(height) * ambientFactor * cloud_bright, density_sample); // ACCUMULATE
		//vec4 src = vec4(mix(cloud_bright, cloud_bright, light_density), density_sample); // ACCUMULATE
		src.rgb *= src.a;
		col = (1.0 - col.a) * src + col;
		if(col.a > 0.95){
			return col;
		}
	 }
	 float dist = sceneSDF(sp);
	 if(dist < EPSILON)
	 {	
		entered = true;
		if(density_sample == 0.0 && !backfront)
		{
			depth += delta*15.0;
		}else
		{
			depth += delta;
			backfront = false;
		}
	 }else if(!entered){
		depth += dist;
	 }else if(entered)
	 {
		break;
	 }
	}
	return col;
}
**/
      

#define BAYER_FACTOR 1.0/16.0
uniform float bayerFilter[16u] = float[]
(
	0.0*BAYER_FACTOR, 8.0*BAYER_FACTOR, 2.0*BAYER_FACTOR, 10.0*BAYER_FACTOR,
	12.0*BAYER_FACTOR, 4.0*BAYER_FACTOR, 14.0*BAYER_FACTOR, 6.0*BAYER_FACTOR,
	3.0*BAYER_FACTOR, 11.0*BAYER_FACTOR, 1.0*BAYER_FACTOR, 9.0*BAYER_FACTOR,
	15.0*BAYER_FACTOR, 7.0*BAYER_FACTOR, 13.0*BAYER_FACTOR, 5.0*BAYER_FACTOR
);


vec4 marchToCloud(vec3 startPos, vec3 endPos){
	vec3 path = endPos - startPos;
	float len = length(path);

	float maxLen = length(planeDim);

	float volumeHeight = planeMax.y - planeMin.y;

	int nSteps = int(mix(48.0, 96.0, clamp( (len - volumeHeight) / volumeHeight ,0.0,1.0) ));

	
	
	float stepSize = len/nSteps;
	vec3 dir = path/len;
	dir *= stepSize;
	vec4 col = vec4(0.0);
	int a = int(gl_FragCoord.x) % 4;
	int b = int(gl_FragCoord.y) % 4;
	startPos += dir * bayerFilter[a * 4 + b]*10.;
	//startPos += dir*step_*abs(Random2D(startPos))*10.;
	vec3 pos = startPos;

	float density = 0.0;

	float lightDotEye = -dot(lightDir, dir);

	for(int i = 0; i < nSteps; ++i)
	{		
		float density_sample = sampleCloudDensity(pos);//*(step_*0.01);

		if(density_sample > 0.0)
		{
			density += density_sample;
			float transmittance = 1.0f - (density / len);


			float light_density = raymarchToLight(pos, stepSize, lightDir, density_sample, lightDotEye);
			float height = getHeightFraction(pos);

			vec3 ambientBadApprox = ambient_light(height) * min(1.0, length(ambientlight)*0.0125 )*transmittance;

			//vec4 src = vec4(lc  * light_density + ambient_light(height) * ambientFactor * cloud_bright, density_sample); // ACCUMULATE
			//vec4 src = vec4(mix(cloud_dark, cloud_bright, light_density), density_sample); // ACCUMULATE
			//src.rgb *= ambient_light(height);
			vec4 src = vec4( ambientlight*light_density + ambientBadApprox, density_sample*transmittance);


			src.rgb *= src.a;
			col = (1.0 - col.a) * src + col;
			if(col.a > 0.95)
			{
				return col;
			}
		}
		pos += dir;
	}

	return col;
}

void main()
{
	//qLenght = iTime/5.0;

	vec2 fragCoord = gl_FragCoord.xy;
	vec2 fulluv = fragCoord - iResolution / 2.0;
	float z =  iResolution.y / tan(radians(20.0));
	vec3 viewDir = normalize(vec3(fulluv, -z / 2.0));
	vec3 worldDir = normalize( (inverse(view) * vec4(viewDir, 0)).xyz);


	//vec3 viewDir = rayDirection(90.0, iResolution.xy, fragCoord);
    //eye = cameraPosition;
    //mat4 viewToWorld = viewMatrix(eye, vec3(0.0, 0, 0.0), vec3(0.0, 1.0, 0.0));
    //viewToWorld = view;
    //vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
    
    //vec4 pos_alpha = shortestDistanceToSurface(eye, worldDir, MIN_DIST, MAX_DIST);
    
	//vec4 v = march_(vec3(eye), ray_wor);
	vec3 startPos, endPos;
	bool hit;
	hit = intersectBox(cameraPosition, worldDir, startPos, endPos);

	vec4 v = vec4(0.0);
	vec4 bg = vec4(0.0, 0.7, 1.0, 1.0);

	if(hit)
	{
		v = marchToCloud(startPos,endPos);
		//v = mix( bg, v, v.a);
	}else
	{
		//v = bg;
	}

    //fragColor = vec4( v.r >= 0.95 ?  1.0 : 0.0  );
	//vec3 ambient = vec3(255, 255, 230)/255;


	//vec4 cloud_color = vec4(mix(ambient*cloud_bright, mix(cloud_dark, ambient, 0.1) , 1 - v.g/1), v.r);

	//v = vec4(texture(weatherTex, TexCoords).g);
	//v = vec4(texture(worley32, vec3(TexCoords, iTime/10.)).r);

	fragColor = v;
}