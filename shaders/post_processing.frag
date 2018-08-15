#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D cloudTEX;

void main()
{
	
	//FragColor = vec4(0.5,0.1,0.8,1.0);
	vec4 cloud = texture(cloudTEX, TexCoords);
	vec4 bg = texture(screenTexture, TexCoords);

	FragColor = mix(bg, cloud, cloud.a);
	
}  