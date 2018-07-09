#version 330 core

in vec2 Coords;

uniform int PrimeIndex;
uniform int resolution;
uniform vec2 offset;

out vec4 Color0;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

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

float SmoothedNoise(int ind, int x, int y) {
	float corners = (Noise(ind, x - 1, y - 1) + Noise(ind, x + 1, y - 1) +
		Noise(ind, x - 1, y + 1) + Noise(ind, x + 1, y + 1)) / 16,
		sides = (Noise(ind, x - 1, y) + Noise(ind, x + 1, y) + Noise(ind, x, y - 1) +
			Noise(ind, x, y + 1)) / 8,
		center = Noise(ind, x, y) / 4;
	return corners + sides + center;
}

float Interpolate(float a, float b, float x) {  // cosine interpolation
	float ft = x * 3.1415927,
		f = (1 - cos(ft)) * 0.5;
	return  a * (1 - f) + b * f;
}

float InterpolatedNoise(int ind, float x, float y) {
	highp int integer_X = int(floor(x));
	float fractional_X = x - integer_X;
	highp int integer_Y = int(floor(y));
	float fractional_Y = y - integer_Y;

	float v1 = SmoothedNoise(ind, integer_X, integer_Y),
		v2 = SmoothedNoise(ind, integer_X + 1, integer_Y),
		v3 = SmoothedNoise(ind, integer_X, integer_Y + 1),
		v4 = SmoothedNoise(ind, integer_X + 1, integer_Y + 1),
		i1 = Interpolate(v1, v2, fractional_X),
		i2 = Interpolate(v3, v4, fractional_X);
	return Interpolate(i1, i2, fractional_Y);
}

float perlin(float x, float y){
	
    int numOctaves = 9;
	float persistence = 0.5;
	float total = 0,
		frequency = pow(2, numOctaves),
		amplitude = 1;
	for (int i = 0; i < numOctaves; ++i) {
		frequency /= 2;
		amplitude *= persistence;
		
		total += InterpolatedNoise( int(mod(PrimeIndex + i,10)), x / frequency, y / frequency) * amplitude;
	}
	return total / frequency;
}

void main()
{   
	//int ind =int(mod(rand(Coords)*10,10));
   float red = (perlin((Coords.x + offset.x*2.0 + 1)*resolution/2 , (Coords.y + offset.y*2.0 + 1)*resolution/2));

   Color0 = vec4(red+0.5, 0.0, 0.0, 1.0);
}