#version 430 core                                                                               
                                                                                                
layout (location = 0) in vec3 Position_VS_in;                                           
layout (location = 1) in vec3 Normal_VS_in;   
layout (location = 2) in vec2 TexCoord_VS_in;                                                  
layout (location = 3) in vec2 a_Position;
			  
uniform mat4 gWorld;          
                                                                                                
out vec3 WorldPos_CS_in;                                                                        
out vec2 TexCoord_CS_in;                                                                        
out vec3 Normal_CS_in;                                                                          
                                                                                                
void main()                                                                                     
{         
    WorldPos_CS_in = (gWorld * vec4(Position_VS_in, 1.0)).xyz;
	WorldPos_CS_in.xz += a_Position;//position[gl_InstanceID];
	Normal_CS_in  = Normal_VS_in;                                  
    TexCoord_CS_in = TexCoord_VS_in;                                                            

}
