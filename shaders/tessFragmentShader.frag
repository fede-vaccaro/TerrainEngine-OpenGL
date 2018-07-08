#version 330 core
in vec3 f_Col;
in vec3 Normal;
in vec2 texCoord;
in vec3 WorldPos;
in float distFromPos;
in float dispFactor;
in float height;

uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform vec3 fogColor;
uniform vec2 offset;
uniform bool drawFog;

uniform sampler2D sand, grass, rock, snow;
uniform sampler2D height_map;

out vec4 FragColor;

void main()
{

	float HEIGHT = 1024.0f; // height map dimensions
	float WIDTH = 1024.0f;
	float SCALE = 0.05f; // step factor

	vec2 uv =  texCoord.xy;
	vec2 du = vec2(1.0/WIDTH, 0);
	vec2 dv= vec2(0, 1.0/HEIGHT);
	// calculate surface partial derivatives 
	float dhdu = SCALE/(2/WIDTH) * (texture(height_map, uv+du).r - texture(height_map, uv-du).r)*dispFactor;
	float dhdv = SCALE/(2/HEIGHT) * (texture(height_map, uv+dv).r - texture(height_map, uv-dv).r)*dispFactor;

	vec3 X = vec3(1.0, dhdu, 1.0);
	vec3 Z = vec3(0.0, dhdv, 1.0);
	vec3 n = normalize(cross(Z,X));
	//vec3 n = normalize(Normal+Z*dhdu+X*dhdv); // add offset to original normal (equivalent to calculate real surface normal)
	// calculate ambient illumination
	float ambientStrength = 0.15; 
    vec3 ambient = ambientStrength * u_LightColor; 

	// calculate diffuse illumination
	vec3 norm = mix(n, Normal, 0.75); 
	norm = normalize(norm);
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float diffuseFactor = max(0.0, dot(lightDir, norm));
	vec3 diffuse = diffuseFactor * u_LightColor; 

	// calculate specular illumination 
	float specularFactor = 0.05f;
	vec3 viewDir = normalize(u_ViewPosition - WorldPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	vec3 specular = spec * u_LightColor;  

	// calculate color by sampling the texture at the given coordinates

	vec4 rockColor = vec4(1.0, 0.0, 0.0, 1.0);//vec4(163, 163, 117, 255)/255.0;

	float trans = 1.0;

	vec4 texCol;
	if(height < dispFactor/2.5 - trans){
		texCol = texture(sand, texCoord*5.0);
	}else if( (height > dispFactor/2.5 - trans) && (height < dispFactor/2.5 + trans) ){
		vec4 texCol1 = texture(sand, texCoord*5.0);
		vec4 texCol2 = texture(grass, texCoord*5.0);
		texCol2 *= vec4(113, 113, 91, 255.0)/255.0;
		texCol = mix(texCol1, texCol2, (height - (dispFactor/2.5 - trans))/(2.0*trans) );

	}else if( (height > dispFactor/2.5 + trans) && (height < dispFactor/2.0 - trans)){
		texCol = texture(grass, texCoord*5.0);
		texCol *= vec4(113, 113, 91, 255.0)/255.0;
	}
	else if( (height > dispFactor/2.0 - trans) && (height < dispFactor/2.0 + trans) ){
		vec4 texCol1 = texture(grass, texCoord*5.0);
		vec4 texCol2 = texture(rock, texCoord*15.0);
		texCol1 *= vec4(113, 113, 91, 255.0)/255.0;
		texCol = mix(texCol1, texCol2, (height - (dispFactor/2.0 - trans))/(2.0*trans) );
	}else if( (height > dispFactor/2.0 + trans) && (height < dispFactor/1.3 - trans) ){
		texCol = texture(rock, texCoord*15.0);
	}	else if( (height > dispFactor/1.3 - trans) && (height < dispFactor/1.3 + trans) ){
		vec4 texCol1 = texture(rock, texCoord*15.0);
		vec4 texCol2 = vec4(1.0);
		texCol = mix(texCol1, texCol2, (height - (dispFactor/1.3 - trans))/(2.0*trans) );
	}
	else{
		texCol = vec4(1.0);
	}
	
	
	//texCol *= vec4(113, 113, 91, 255.0)/255.0;

	


	// calculate fog color 
	vec2 u_FogDist = vec2(20.0, 50.0);
	float fogFactor = clamp((u_FogDist.y - distFromPos) / (u_FogDist.y - u_FogDist.x), 0.0, 1.0);

	// putting all together
    vec4 color = texCol*vec4((ambient + specular + diffuse)*vec3(1.0) , 1.0f);
	if(drawFog){
	FragColor = mix(color, vec4(fogColor, 1.0f), 1 - fogFactor);
	}else{
	FragColor = color;
	}
};