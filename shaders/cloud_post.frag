#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;


uniform sampler2D cloudTEX;
uniform sampler3D volTex;
uniform float time;

const float offset_x = 1. / 1600.0;  
const float offset_y = 1. / 900.0;

bool pp = true;


void main()
{
	if(pp){

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
		vec4 pixel = texture(cloudTEX, TexCoords.st + offsets[i]);
        sampleTex[i] = pixel;
    }
    vec4 col = vec4(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    FragColor = col;
	}else{
	}
}  