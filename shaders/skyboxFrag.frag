#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
	vec3 color1 = vec3(0.0, 0.4, 0.8)*0.8;

    // Time varying pixel color
    vec3 color2 = vec3(0.5, 0.8, 0.8);
    
    vec3 color3 = color1*0.8;


    vec3 col = mix(color1, color2, exp(-5.0*TexCoords.y));

    col = mix(col, color3, clamp(1.-exp(-2.*(TexCoords.y - 0.5)), 0.0, 1.0)  );

    //FragColor = texture(skybox, TexCoords);

	FragColor = vec4(col, 1.0);
}