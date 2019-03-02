#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D fboTex;


void main()
{
	vec4 col = texture(fboTex, TexCoords);

	FragColor = col;

}  