#version 330 core

layout (location = 0) out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D clouds;
uniform sampler2D emissions;
uniform sampler2D depthMap;

uniform float time;
uniform vec4 lightPos;
uniform vec2 resolution;// = vec2(1920.0, 1080.0);
uniform vec2 cloudRenderResolution;

uniform bool isLightInFront = true;
uniform bool enableGodRays;
uniform float lightDotCameraFront;

#define  offset_x  1. / cloudRenderResolution.x  
#define  offset_y  1. / cloudRenderResolution.y

bool pp = false;

vec4 gaussianBlur(sampler2D tex, vec2 uv){
 vec2 offsets[9] = vec2[](
        vec2(-offset_x,  offset_y), // top-left
        vec2( 0.0f,    offset_y), // top-center
        vec2( offset_x,  offset_y), // top-right
        vec2(-offset_x,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset_x,  0.0f),   // center-right
        vec2(-offset_x, -offset_y), // bottom-left
        vec2( 0.0f,   -offset_y), // bottom-center
        vec2( offset_x, -offset_y)  // bottom-right    
    );

	float kernel[9] = float[](
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16  
	);
    
    vec4 sampleTex[9];

    for(int i = 0; i < 9; i++)
    {	
		vec4 pixel = texture(tex, uv.st + offsets[i]);
        sampleTex[i] = pixel;
    }
    vec4 col = vec4(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    return col;
}

void main()
{

	FragColor = gaussianBlur(clouds, TexCoords);
	

	/////////////////////////////////////////////// RADIAL BLUR - CREPUSCOLAR RAYS
	bvec2 lowerLimit = greaterThan(lightPos.xy, vec2(0.0));
	bvec2 upperLimit = lessThan(lightPos.xy, vec2(1.));
	if(  lightDotCameraFront > 0.0 && enableGodRays)
	{
    // Screen coordinates.
    vec2 uv = gl_FragCoord.xy / resolution;

    // Radial blur factors.
    //
    float decay = 0.98; 
    float density = 0.9; 
    float weight = 0.07; 
	float exposure = 0.45;
    
    // Light offset. 
    vec3 l = vec3(lightPos.xy, 0.5);
    
	const int SAMPLES = 64;
	float illuminationDecay = 1.0;

	vec2 tc = uv;
	vec2 dTuv = tc - lightPos.xy;
	dTuv *= density/float(SAMPLES);
    
    //vec3 colRays = texture(emissions, uv.xy).rgb*0.4;
    vec3 colRays = gaussianBlur(emissions, uv).rgb*0.4;
    for(int i=0; i < SAMPLES; i++){
        uv -= dTuv;
        //colRays += texture(emissions, uv).rgb *illuminationDecay* weight;
        colRays += texture(emissions, uv).rgb *illuminationDecay* weight;
        illuminationDecay *= decay;
    }
    
    //FragColor -= 0.2;
	//FragColor.rgb += (smoothstep(0., 1., colRays)*exposure - 0.2);
	vec3 colorWithRays = FragColor.rgb +  (smoothstep(0., 1., colRays)*exposure);
	FragColor.rgb = mix(FragColor.rgb, colorWithRays*0.9, lightDotCameraFront*lightDotCameraFront);
	//FragColor.rgb = (smoothstep(0., 1., colRays)*exposure - 0.2);
	}


}  