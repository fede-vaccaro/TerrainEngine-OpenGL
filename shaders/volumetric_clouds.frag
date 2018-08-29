#version 330 core
/**
 * Part 3 Challenges
 * - Make the camera move up and down while still pointing at the cube
 * - Make the camera roll (stay looking at the cube, and don't change the eye point)
 * - Make the camera zoom in and out
 */

in vec2 TexCoords;

uniform float FOV;
uniform vec2 iResolution;
uniform float iTime;
uniform mat4 inv_view;
uniform mat4 inv_proj;

uniform mat4 invViewProj;
uniform mat4 oldFrameVP;

uniform vec3 sunPosition = vec3(1000,1000,1000)*20.;
uniform vec3 lightDir = normalize(vec3(1,0.5,0.5));
uniform vec4 lightPos;
uniform sampler3D cloud;
uniform sampler3D worley32;
uniform sampler2D weatherTex;
uniform sampler2D depthMap;
uniform sampler2D lastFrameAlphaness;
uniform sampler2D lastFrameColor;
uniform vec3 lightPosition;
uniform float coverage_multiplier = 0.4;


uniform vec3 cameraPosition;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 bloom;
layout (location = 2) out vec4 alphaness;
layout (location = 3) out vec4 cloudColor;

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.001;
const float e = 2.71828182846;
const float PI_r = 0.3183098;

const vec4 cloud_bright = vec4(0.99, 0.96, 0.95, 1.0);
const vec4 cloud_dark = vec4(0.671, 0.725, 0.753, 1.0);

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


// Cloud types height density gradients
#define STRATUS_GRADIENT vec4(0.0, 0.1, 0.2, 0.3)
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)
#define CUMULUS_GRADIENT vec4(0.00, 0.1625, 0.88, 0.98)

#define EARTH_RADIUS (700000.)
#define SPHERE_INNER_RADIUS (EARTH_RADIUS + 15000.0)
#define SPHERE_OUTER_RADIUS (SPHERE_INNER_RADIUS + 18000.0)
#define SPHERE_DELTA float(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS)

#define CLOUDS_AMBIENT_COLOR_TOP (vec3(149., 149., 170.)*(1./255.))
#define CLOUDS_AMBIENT_COLOR_BOTTOM (vec3(65., 65., 77.)*(1.8/255.))
#define CLOUDS_MIN_TRANSMITTANCE 1e-1
#define CLOUDS_TRANSMITTANCE_THRESHOLD 1.0 - CLOUDS_MIN_TRANSMITTANCE

#define SUN_DIR normalize(vec3(-.0,.7,1.0))
//#define SUN_DIR normalize(vec3(1,10,1));
#define SUN_COLOR ambientlight
vec3 sphereCenter = vec3(0.0, -EARTH_RADIUS, 0.0);

bool intersectCubeMap(vec3 o, vec3 d, out vec3 minT, out vec3 maxT)
{		
	vec3 cubeMin = vec3(-0.5, -0.5, -0.5);
	vec3 cubeMax = vec3(0.5, 1 + cubeMin.y, 0.5);

	// compute intersection of ray with all six bbox planes
	vec3 invR = 1.0 / d;
	vec3 tbot = invR * (cubeMin - o);
	vec3 ttop = invR * (cubeMax - o);
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

void swap(in float a, in float b){
	float c = a;
	a = b;
	b = c;
}

bool raySphereintersection(vec3 ro, vec3 rd, float radius, out vec3 startPos){
	
	float t;

	sphereCenter.xz = cameraPosition.xz;

	float radius2 = radius*radius;

	vec3 L = ro - sphereCenter;
	float a = dot(rd, rd);
	float b = 2.0 * dot(rd, L);
	float c = dot(L,L) - radius2;

	float discr = b*b - 4.0 * a * c;
	if(discr < 0.0) return false;
	t = max(0.0, (-b + sqrt(discr))/2);
	if(t == 0.0){
		return false;
	}
	startPos = ro + rd*t;

	return true;
}

vec4 colorCubeMap(vec3 endPos, vec3 d)
{
    // background sky     
	float sun = clamp( dot(SUN_DIR,d), 0.0, 1.0 );
	vec3 col = vec3(0.6,0.71,0.85) - endPos.y*0.2*vec3(1.0,0.5,1.0) + 0.15*0.5;
	col += 0.8*vec3(1.0,.6,0.1)*pow( sun, 256.0 );

	return vec4(col, 1.0);
}


float Random2D(in vec3 st)
{
	return fract(sin(iTime*dot(st.xyz, vec3(12.9898, 78.233, 57.152))) * 43758.5453123);
}


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


 float getHeightFraction(vec3 inPos){
	
	//float height = (inPos.y - cloudMinMax.x)/(cloudMinMax.y - cloudMinMax.x);

	float height = (length(inPos - sphereCenter) - SPHERE_INNER_RADIUS)/(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS);

	return height;

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
	//return remap(heightFraction, baseGradient.x, baseGradient.y, 0.0, 1.0) * remap(heightFraction, baseGradient.z, baseGradient.w, 1.0, 0.0);
	return smoothstep(baseGradient.x, baseGradient.y, heightFraction) - smoothstep(baseGradient.z, baseGradient.w, heightFraction);

}

float threshold(float v, float t)
{
	return v > t ? v : 0.0;
}

const vec3 windDirection = normalize(vec3(0.5, 0.0, 0.1));

#define CLOUD_TOP_OFFSET 750.0
#define SATURATE(x) clamp(x, 0.0, 1.0)

float sampleCloudDensity(vec3 p, bool expensive){

	float heightFraction = getHeightFraction(p);


	//vec2 uv = (p.xz - planeMin.xz) / planeDim;
	const float cloudSpeed = 75.0;

	p += heightFraction * windDirection * CLOUD_TOP_OFFSET;
	p += windDirection * iTime * cloudSpeed;

	vec2 uv = p.xz/SPHERE_INNER_RADIUS + 0.5;

	//uv *= 2.0;
	vec2 uv_scaled = uv*40.0;


	if(heightFraction < 0.0 || heightFraction > 1.0){
		return 0.0;
	}

	vec4 low_frequency_noise = texture(cloud, vec3(uv_scaled, heightFraction));

	vec3 weather_data = texture(weatherTex, uv).rgb;

	float lowFreqFBM = low_frequency_noise.g*0.625 + low_frequency_noise.b*0.25 + low_frequency_noise.a*0.125;
	float base_cloud = remap(low_frequency_noise.r*1.5, -(1.0 - lowFreqFBM), 1., 0.0 , 1.0);
	
	//heightFraction = clamp(heightFraction, -heightFraction - EPSILON, heightFraction + EPSILON);
	float density = getDensityForCloud(heightFraction, weather_data.g);
	//base_cloud = remap(base_cloud, 1.0 - density, 1.0, 0.0, 1.0);
	base_cloud *= density/heightFraction;

	float cloud_coverage = weather_data.r*coverage_multiplier;
	float base_cloud_with_coverage = remap(base_cloud , cloud_coverage , 1.0 , 0.0 , 1.0);
	base_cloud_with_coverage *= cloud_coverage;


	//bool expensive = true;
	
	if(expensive)
	{
		vec3 erodeCloudNoise = texture(worley32, vec3(uv_scaled*1.5, heightFraction)*0.1 ).rgb;
		float highFreqFBM = (erodeCloudNoise.r * 0.625) + (erodeCloudNoise.g * 0.25) + (erodeCloudNoise.b * 0.125);
		float highFreqNoiseModifier = mix(highFreqFBM, 1.0 - highFreqFBM, clamp(heightFraction * 10.0, 0.0, 1.0));

		base_cloud_with_coverage = base_cloud_with_coverage - highFreqNoiseModifier * (1.0 - base_cloud_with_coverage);

		base_cloud_with_coverage = remap(base_cloud_with_coverage*2.0, highFreqNoiseModifier * 0.2, 1.0, 0.0, 1.0);
	}

	//return clamp(base_cloud_with_coverage*1.5, 0.0, 1.0);
	return clamp(base_cloud_with_coverage, 0.0, 1.0);
}


float HG( float sundotrd, float g) {
	float gg = g * g;
	return (1. - gg) / pow( 1. + gg - 2. * g * sundotrd, 1.5);
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
}

 vec3 eye = cameraPosition;


float raymarchToLight(vec3 o, float stepSize, vec3 lightDir, float originalDensity, float lightDotEye)
{

	vec3 startPos = o;
	float ds = stepSize * 6.0;
	vec3 rayStep = lightDir * ds;
	const float CONE_STEP = 1.0/6.0;
	float coneRadius = 1.0; 
	float density = 0.0;
	float coneDensity = 0.0;
	float invDepth = 1.0/ds;
	const float absorption = 0.0001;
	float sigma_ds = -ds*absorption;
	vec3 pos;

	float T = 1.0;

	for(int i = 0; i < 6; i++)
	{
		pos = startPos;// + coneRadius*noiseKernel[i]*float(i);

		float heightFraction = getHeightFraction(pos);
		if(heightFraction >= 0)
		{
			
			float cloudDensity = sampleCloudDensity(pos, false);
			if(cloudDensity > 0.0)
			{
				float Ti = exp(cloudDensity*sigma_ds);
				T *= Ti;
				density += cloudDensity;
			}
		}
		startPos += rayStep;
		coneRadius += CONE_STEP;
	}


	//pos += (rayStep * 8.0);
	//float heightFraction = getHeightFraction(pos);
	//float cloudDensity = sampleCloudDensity(pos);
	//if(cloudDensity > 0.0)
	//{
	//		float Ti = exp(cloudDensity*sigma_ds);
	//		T *= Ti;
	//}

	return T*mix(1.0 - exp(-2.0*originalDensity)*0.5, 1.0, 1.0 - (lightDotEye*0.5 + 0.5)*0.5);//*powder(originalDensity, 0.0);
}

vec3 ambientlight = vec3(255, 255, 235)/255;

float ambientFactor = 0.5;
vec3 lc = ambientlight * ambientFactor;// * cloud_bright;

vec3 ambient_light(float heightFrac)
{
	return mix( vec3(0.5, 0.67, 0.82), vec3(1.0), heightFrac);
}
      

#define BAYER_FACTOR 1.0/16.0
uniform float bayerFilter[16u] = float[]
(
	0.0*BAYER_FACTOR, 8.0*BAYER_FACTOR, 2.0*BAYER_FACTOR, 10.0*BAYER_FACTOR,
	12.0*BAYER_FACTOR, 4.0*BAYER_FACTOR, 14.0*BAYER_FACTOR, 6.0*BAYER_FACTOR,
	3.0*BAYER_FACTOR, 11.0*BAYER_FACTOR, 1.0*BAYER_FACTOR, 9.0*BAYER_FACTOR,
	15.0*BAYER_FACTOR, 7.0*BAYER_FACTOR, 13.0*BAYER_FACTOR, 5.0*BAYER_FACTOR
);

const float inv_sqrt_two = pow(2.0, -0.5);

vec4 marchToCloud(vec3 startPos, vec3 endPos, vec3 bg){
	vec3 path = endPos - startPos;
	float len = length(path);

	//float maxLen = length(planeDim);

	//float volumeHeight = planeMax.y - planeMin.y;

	int nSteps = int(mix(64.0, 128.0, clamp( len/SPHERE_DELTA - 1.0,0.0,1.0) ));
	
	float ds = len/nSteps;
	vec3 dir = path/len;
	dir *= ds;
	vec4 col = vec4(0.0);
	int a = int(fract(length(startPos) + iTime)*100.0) % 4;
	int b = int(fract(length(endPos) + iTime)*100.0) % 4;
	startPos += dir * bayerFilter[a * 4 + b]*5.0;
	//startPos += dir*abs(Random2D(vec3(a,b,a+b)))*len/SPHERE_DELTA*10.0;
	vec3 pos = startPos;

	float density = 0.0;

	float lightDotEye = dot(normalize(SUN_DIR), normalize(dir));

	float T = 1.0;
	const float absorption = 0.00750;
	float sigma_ds = -ds*absorption;
	bool expensive = true;
	bool entered = false;

	int zero_density_sample = 0;

	for(int i = 0; i < nSteps; ++i)
	{	
		if( pos.y >= cameraPosition.y - SPHERE_DELTA*1.5 ){

		float density_sample = sampleCloudDensity(pos, T > 0.0);
		if(density_sample > 0.)
		{
			float height = getHeightFraction(pos);
			vec3 ambientLight = mix( CLOUDS_AMBIENT_COLOR_BOTTOM, CLOUDS_AMBIENT_COLOR_TOP, height )*1.0;
			float light_density = raymarchToLight(pos, ds, SUN_DIR, density_sample, lightDotEye);
			float scattering = 0.8*mix(HG(lightDotEye, -0.15), HG(lightDotEye, 0.05), clamp(lightDotEye/2.0 + 0.65, 0.0, 1.0));
			//scattering = 0.6;
			vec3 S = (bg*0.4 + SUN_COLOR * (scattering * light_density)) * density_sample;
			float dTrans = exp(density_sample*sigma_ds);
			vec3 Sint = (S - S * dTrans) * (1. / density_sample);
			col.rgb += T * Sint;
			T *= dTrans;

		}

		if( T <= CLOUDS_MIN_TRANSMITTANCE ) break;

		pos += dir;
		}
	}

	col.a = 1.0 - T;
	
	//col = vec4( vec3(getHeightFraction(startPos)), 1.0);

	return col;
}

vec3 computeClipSpaceCoord(){
	vec2 ray_nds = 2.0*gl_FragCoord.xy/iResolution.xy - 1.0;
	return vec3(ray_nds, 1.0);
}

vec2 computeScreenPos(vec2 ndc){
	return (ndc*0.5 + 0.5);
}

float computeFogAmount(in vec3 startPos, in float factor = 0.00008){
	float dist = length(startPos - cameraPosition);
	float radius = (cameraPosition.y - sphereCenter.y) * 0.3;
	float alpha = (dist / radius);
	//v.rgb = mix(v.rgb, ambientColor, alpha*alpha);

	return (1.-exp( -dist*alpha*factor));
}

void main()
{
	//if(texture(depthMap, TexCoords).r < 1.0)
	if(false)
	{
		fragColor = vec4(0.0);
		bloom = vec4(0.0);
		return;
	}

	vec4 ray_clip = vec4(computeClipSpaceCoord(), 1.0);
	vec4 ray_view = inv_proj * ray_clip;
	ray_view = vec4(ray_view.xy, -1.0, 0.0);
	vec3 worldDir = (inv_view * ray_view).xyz;
	worldDir = normalize(worldDir);

	//for picking previous frame color -- temporal projection
	vec4 camToWorldPos = invViewProj*ray_clip;
	camToWorldPos /= camToWorldPos.w;
	vec4 pPrime = oldFrameVP*camToWorldPos;
	pPrime /= pPrime.w;
	vec2 prevFrameScreenPos = computeScreenPos(pPrime.xy); 
	bool isOut = any(greaterThan(abs(prevFrameScreenPos - 0.5) , vec2(0.5)));


	vec3 startPos, endPos;
	//bool hit = intersectSphere(cameraPosition, worldDir, startPos, endPos);
	//bool hit = sphereIntersection(cameraPosition, worldDir, startPos, endPos);

	vec4 v = vec4(0.0);

	vec3 stub, cubeMapEndPos;
	intersectCubeMap(vec3(0.0, 0.0, 0.0), worldDir, stub, cubeMapEndPos);

	vec4 bg = colorCubeMap(cubeMapEndPos, worldDir);

	bool hit = raySphereintersection(cameraPosition, worldDir, SPHERE_INNER_RADIUS, startPos);
	bool hit2 = raySphereintersection(cameraPosition, worldDir, SPHERE_OUTER_RADIUS, endPos);

	float fogAmount = computeFogAmount(startPos);

	if(fogAmount > 0.965){
		fragColor = bg;
		bloom = bg;
		return;
	}

	//early exit -- search for low alphaness areas
	float oldFrameAlphaness = 1.0;

	if(!isOut){
		oldFrameAlphaness = texture(lastFrameAlphaness, prevFrameScreenPos).r;
	}
	if( oldFrameAlphaness > 0.0 )
	{
		v = marchToCloud(startPos,endPos, bg.rgb);
		cloudColor = v;
	}else{
		v = texture(lastFrameColor, prevFrameScreenPos);
		cloudColor = v;
		//v = vec4(1.0, 0.0, 0.0,v.a);
	}
	float cloudAlphaness = threshold(v.a, 0.2);
	v = v*1.4 + 0.1;

	//v.rgb = mix(v.rgb, ambientlight, 1.0 - sqrt(v.a));
	//v.rgb = mix(bg.rgb, v.rgb, v.a);

	// Apply atmospheric fog to far away clouds
	vec3 ambientColor = bg.rgb;//vec3(0.6,0.71,0.75) + 0.25;

	// Use current position distance to center as action radius
    v.rgb = mix(v.rgb, bg.rgb*v.a, clamp(fogAmount,0.,1.));


	float sun = clamp( dot(SUN_DIR,normalize(endPos - startPos)), 0.0, 1.0 );
	vec3 s = 0.8*vec3(1.0,0.4,0.2)*pow( sun, 256.0 );
	//bg.rgb += s;
	v.rgb += s*v.a;


	bg.rgb = bg.rgb*(1.0 - v.a) + v.rgb;
	bg.a = 1.0;
	//bg.rgb = mix(bg.rgb, v.rgb, v.a);

	// sun glare    

	fragColor = bg;
	alphaness = vec4(cloudAlphaness, 0.0, 0.0, 1.0);



	//fragColor = vec4(1.0, 1.0, 0.0, 1.0);
	//fragColor = texture(worley32, vec3(TexCoords, 0.0));

	//float sun_emission = clamp( dot(SUN_DIR,normalize(endPos - startPos)), 0.0, 1.0 );
	//vec3 s_emission = 0.5*vec3(1.0,1.0,1.0)*SUN_COLOR*pow( sun_emission, 256.0 );
	//s_emission*=8.0;
	//bloom = vec4(s_emission*SUN_COLOR, length(s_emission));
	bloom = bg;
	if(cloudAlphaness > 0.1){
		//bloom = vec4(0.0, 0.0, 0.0, cloudAlphaness);
		//bloom *= (1.0 - cloudAlphaness);

		float fogAmount = computeFogAmount(startPos, 0.00002);

		vec3 cloud = mix(vec3(0.0), bloom.rgb*cloudAlphaness, clamp(fogAmount,0.,1.));
		bloom.rgb = bloom.rgb*(1.0 - cloudAlphaness) + cloud.rgb;
		//bloom.rgb *= mix(bloom.rgb, bloom.rgb*cloudAlphaness, clamp(fogAmount,0.,1.));
		//bloom *= (1.0 - cloudAlphaness);

		//fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	if(!isOut){
	//	fragColor = texture(lastFrameAlphaness, prevFrameScreenPos);
	}
	if(texture(depthMap, TexCoords).r < 1.0)
	//if(false)
	{
		fragColor = vec4(0.0);
		bloom = vec4(0.0);
		return;
	}

}