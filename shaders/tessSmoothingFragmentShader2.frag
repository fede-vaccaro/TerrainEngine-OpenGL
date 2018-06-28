#version 410 core
in vec3 f_Col;
in vec3 Normal;
in vec2 texCoord;
in vec3 WorldPos;
in float distFromPos;

in vec3 vColor;

uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform vec3 fogColor;
uniform vec3 gWorldEyePos;

uniform sampler2D tex;
uniform sampler2D height_map;

out vec4 FragColor;

void main()
{
	// calculate ambient illumination
	float ambientStrength = 0.15; 
    vec3 ambient = ambientStrength * u_LightColor; 

	// calculate diffuse illumination
	vec3 norm = Normal;
	norm = normalize(norm);
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float diffuseFactor = max(0.0, dot(lightDir, norm));
	vec3 diffuse = diffuseFactor * u_LightColor; 

	// calculate specular illumination 
	float specularFactor = 0.05f;
	vec3 viewDir = normalize(u_ViewPosition - WorldPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0);
	vec3 specular = spec * u_LightColor;  

	// calculate color by sampling the texture at the given coordinates
	vec4 texCol = texture(tex, texCoord);
	texCol = vec4(vec3(0.7), 1.0)*vec4(max(vColor,0.0), 1.0);	

	// calculate fog color 
	vec2 u_FogDist = vec2(15.0, 20.0);
	float fogFactor = clamp((u_FogDist.y - distFromPos) / (u_FogDist.y - u_FogDist.x), 0.0, 1.0);

	// putting all together
    vec4 color = texCol*vec4((ambient + specular + diffuse)*vec3(1.0) , 1.0f);
	FragColor = mix(color, vec4(fogColor, 1.0f), 1 - fogFactor);
};