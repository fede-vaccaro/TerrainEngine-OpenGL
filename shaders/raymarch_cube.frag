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

float Random2D(in vec3 st)
{
	return fract(sin(dot(st.xyz, vec3(12.9898, 78.233, 57.152))) * 43758.5453123);
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
	float v1 = Random2D(randomInput + vec3(0.0, 0.0, 0.0));
	float v2 = Random2D(randomInput + vec3(1.0, 0.0, 0.0));
	float v3 = Random2D(randomInput + vec3(0.0, 1.0, 0.0));
	float v4 = Random2D(randomInput + vec3(1.0, 1.0, 0.0));
    
    float v5 = Random2D(randomInput + vec3(0.0, 0.0, 1.0));
	float v6 = Random2D(randomInput + vec3(1.0, 0.0, 1.0));
	float v7 = Random2D(randomInput + vec3(0.0, 1.0, 1.0));
	float v8 = Random2D(randomInput + vec3(1.0, 1.0, 1.0));
    
    
	float i1 = Interpolate(v1, v2, fractional_X);
	float i2 = Interpolate(v3, v4,  fractional_X);
    
    float i3 = Interpolate(v5, v6, fractional_X);
    float i4 = Interpolate(v7, v8, fractional_X);
    
    float y1 = Interpolate(i1, i2, fractional_Y);
    float y2 = Interpolate(i3, i4, fractional_Y);
    
    
	return Interpolate(y1, y2, fractional_Z);
}

float weather(float x, float y, float z){

	float threshold = 0.4;
	float amp = 1.0;


    int numOctaves = 10;
	float persistence = 0.6;
	float total = 0.,
		frequency = .25,
		amplitude = amp;
	for (int i = 0; i < numOctaves; ++i) {
		frequency *= 2.;
		amplitude *= persistence;
		total += InterpolatedNoise(0, x * frequency, y * frequency, 0) * amplitude;
	}

	if(total < threshold) total = 0; else total = 1.0;

	return total;
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

 float HG(float costheta) {
	float g = 0.9;
	return 0.25 * PI_r * (1 - pow(g, 2.0)) / pow((1 + pow(g, 2.0) - 2 * g * costheta), 1.5);
}

 float phase(vec3 v1, vec3 v2) {
	float costheta = dot(v1, v2) / length(v1) / length(v2);
	return HG(costheta);
	//return Mie(costheta);
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

		sp = origin + dir * t;
		float increment = filterY(texture(cloud, sp/(2*qLenght) + 0.5).r, sp.y);
		float weather = texture(weatherTex, sp.xz/(2*qLenght) + 0.5).r;
		increment *= weather;
		if( increment > 0) inside+=increment;
	}
	float d = 3.0;
	float scatter = 2* exp(-d * inside) * (1.0 - exp(-2*d * inside));
	float value = scatter;
	return value;
}

 vec4 march_in_cloud(vec3 p, vec3 dir, float delta){
	float alpha = 0.0;
	float depth = abs(Random2D(p))/50.0;
	for(int i = 0; i < 1000; i++){
		float dist = sceneSDF(p + dir*depth);
		if(dist < EPSILON){
			vec3 sp = p + depth*dir; 
			float increment = filterY(texture(cloud, sp/(2*qLenght) + 0.5).r, sp.y);
			float weather = texture(weatherTex, sp.xz/(2*qLenght) + 0.5).r;
			increment *= weather;
			if( increment > 0) alpha+=increment;
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
			vec4 alpha = march_in_cloud(eye + depth*marchingDirection, marchingDirection, 0.02);
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