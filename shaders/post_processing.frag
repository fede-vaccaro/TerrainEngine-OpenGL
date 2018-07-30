#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform sampler3D volTex;
uniform float time;

const float offset_x = 1. / 1280.0;  
const float offset_y = 1. / 720.0;

bool pp = false;


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
    
    vec3 sampleTex[9];

    for(int i = 0; i < 9; i++)
    {	
		vec3 color = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
		float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
		float trans = 0.25;
		vec3 bg = vec3(204, 224, 255)/255;
		//if(brightness < 0.5){ color += bg*(1 - brightness);}

        sampleTex[i] = color;
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    FragColor = vec4(col, 1.0);
	}else{
		//FragColor = texture(screenTexture, TexCoords);
		FragColor = texture(volTex, vec3(TexCoords, time/10.0));
		//FragColor = vec4(FragColor);
	}
}  