#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D cloudTEX;

#define HDR(col, exps) 1.0 - exp(-col * exps)

void main()
{
	
	//FragColor = vec4(0.5,0.1,0.8,1.0);
	vec4 cloud = texture(cloudTEX, TexCoords);
	vec4 bg = texture(screenTexture, TexCoords);
	
	vec4 col = mix(bg, cloud, cloud.a);

	const float gamma = 2.2;
    const float exposure = 5.0;
    // Exposure tone mapping
    //col = mix(col, HDR(col, exposure), 0.5);

	// gamma and contrast
    col.rgb = mix(col.rgb, pow(col.rgb, vec3(1./gamma)), 0.85);
	col.rgb = (col.rgb - 0.3)*1.5;
    //col.rgb = mix( col.rgb, col.bbb, 0. ); 
     
     // vignette
     vec2 uv = gl_FragCoord.xy / vec2(1600.0, 900.0);
     //col.rgb = mix(col.rgb*col.rgb, col.rgb, pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.05 ));
	col.rgb *= pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.11 );

	FragColor = col;
}  