#version 330 core                                                                               

in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D buffTex;

out vec4 FragColor;

void main(){
	vec2 newTexCoords = vec2(TexCoords.x, -TexCoords.y);
	FragColor = texture(buffTex, newTexCoords);
}