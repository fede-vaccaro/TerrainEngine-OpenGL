#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D cloudTEX;

uniform vec2 resolution;

#define HDR(col, exps) 1.0 - exp(-col * exps)

vec3 TonemapACES(vec3 x)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;
	return (x * (A * x + B)) / (x * (C * x + D) + E);
}


void main()
{
	
	//FragColor = vec4(0.5,0.1,0.8,1.0);
	vec4 cloud = texture(cloudTEX, TexCoords);
	vec4 bg = texture(screenTexture, TexCoords);
	
	vec4 col = mix(bg, cloud, cloud.a);

	const float gamma = 2.2;
    const float exposure = 3.0;
    // Exposure tone mapping
    //col = HDR(col, exposure);

	// gamma and contrast
    //col.rgb = pow(col.rgb, vec3(1.0/gamma));
	//col.rgb = (col.rgb - 0.3)*1.5;
    //col.rgb = mix( col.rgb, col.bbb, 0. ); 
     
     // vignette
     vec2 uv = gl_FragCoord.xy / resolution;
     //col.rgb = mix(col.rgb*col.rgb, col.rgb, pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.05 ));
	col.rgb *= pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.11 );
	//col.rgb = TonemapACES(col.rgb);
	FragColor = col;

}  