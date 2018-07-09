#version 330 core                                                                               

in vec2 TexCoords;
in vec3 Normal;
in vec4 clipSpaceCoords;
in vec4 position;

uniform float moveFactor;

uniform vec3 cameraPosition;
uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D waterDUDV;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

out vec4 FragColor;

const float distFactor = 0.008;

void main(){
	vec2 ndc = (clipSpaceCoords.xy/clipSpaceCoords.w)/2.0 + 0.5;
	vec2 distortion1 = texture(waterDUDV, vec2(TexCoords.x + moveFactor, TexCoords.y)*10.0).rg*2.0 - 1.0;
	vec2 distortion2 = texture(waterDUDV, vec2(TexCoords.x + moveFactor, TexCoords.y - moveFactor)*10.0).rg*2.0 - 1.0;
	vec2 totalDistortion = distortion1 + distortion2;

	totalDistortion *= distFactor;

	vec2 reflectionTexCoords = vec2(ndc.x, -ndc.y);
	reflectionTexCoords += totalDistortion;
	reflectionTexCoords.x = clamp(reflectionTexCoords.x, 0.001, 0.999);
	reflectionTexCoords.y = clamp(reflectionTexCoords.y, -0.999, -0.001);
	vec4 reflectionColor = texture(reflectionTex, reflectionTexCoords);

	vec2 refractionTexCoords = ndc;
	
	float near = 0.1;
	float far = 100.0;
	float depth = texture(depthMap, refractionTexCoords).r;
	float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	float waterDistance = 2.0 * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));
	float waterDepth = floorDistance - waterDistance;
	waterDepth = clamp(waterDepth, 0.0, 1.0);

	refractionTexCoords += totalDistortion;
	refractionTexCoords = clamp(refractionTexCoords, 0.001, 0.999);
	vec4 refractionColor = texture(refractionTex, refractionTexCoords);


	vec3 toCameraVector =  position.xyz - cameraPosition;
	float fresnelFactor = max(dot(normalize(toCameraVector), vec3(0.0, 1.0, 0.0)), 0.0);
	fresnelFactor = pow(fresnelFactor, 0.75);
	vec4 refr_reflCol = mix(reflectionColor, refractionColor, fresnelFactor);

	// calculate diffuse illumination
	totalDistortion = normalize(totalDistortion);
	vec3 X = vec3(1.0, totalDistortion.r, 1.0);
	vec3 Z = vec3(0.0, totalDistortion.g, 1.0);
	vec3 norm = texture(normalMap, totalDistortion).rgb;
	norm = vec3(norm.r*2 - 1, norm.b*1.5, norm.g*2 - 1);
	norm = normalize(norm);
	vec3 lightDir = normalize(u_LightPosition - position.xyz);
	float diffuseFactor = max(0.0, dot(lightDir, norm.rgb));
	vec3 diffuse = diffuseFactor * vec3(1.0);

	// calculate specular illumination 
	float specularFactor = 0.05f;
	vec3 viewDir = normalize(cameraPosition - position.xyz);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(-viewDir, reflectDir), 0.0), 64.0);
	vec3 specular = spec * u_LightColor * specularFactor * waterDepth;  


	vec4 color = vec4(0, 154, 255, 255)/255.0;
	FragColor = mix(mix( refr_reflCol, color, 0.15), vec4(diffuse,1.0), 0.03) + vec4(specular, 1.0);
	FragColor.a = waterDepth;
	}