#version 330 core                                                                               

in vec2 TexCoords;
in vec3 Normal;
in vec4 clipSpaceCoords;

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D waterDUDV;

out vec4 FragColor;

const float distFactor = 0.2;

void main(){
	vec2 ndc = (clipSpaceCoords.xy/clipSpaceCoords.w)/2.0 + 0.5;
	vec2 distortion1 = texture(waterDUDV, TexCoords).rg*2.0 - 1.0;
	distortion1 *= distFactor;

	vec2 reflectionTexCoords = vec2(ndc.x, -ndc.y);
	reflectionTexCoords.x = clamp(reflectionTexCoords.x, 0.001, 0.999);
	reflectionTexCoords.y = clamp(reflectionTexCoords.y, -0.999, -0.001);
	vec4 reflectionColor = texture(reflectionTex, reflectionTexCoords);

	vec2 refractionTexCoords = ndc;
	reflectionTexCoords += distortion1;
	refractionTexCoords = clamp(refractionTexCoords, 0.001, 0.999);
	vec4 refractionColor = texture(refractionTex, refractionTexCoords);

	vec4 refr_reflCol = mix(reflectionColor, refractionColor, 0.25);

	vec4 color = vec4(0, 154, 255, 255)/255.0;
	
	FragColor = mix( refr_reflCol, color, 0.25);
	//FragColor = texture(waterDUDV, TexCoords);
}