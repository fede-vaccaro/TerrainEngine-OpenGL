#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D cloudTEX;

uniform sampler3D volTex;
uniform float time;

const float offset_x = 1. / 1280.0;  
const float offset_y = 1. / 720.0;

bool pp = false;


void main()
{
	
	//FragColor = vec4(0.5,0.1,0.8,1.0);
	vec4 cloud = texture(cloudTEX, TexCoords);
	vec4 bg = texture(screenTexture, TexCoords);

	FragColor = mix(bg, cloud, cloud.a);
	
}  