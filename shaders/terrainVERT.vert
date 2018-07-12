#version 410 core                                                                               
                                                                                                
layout (location = 0) in vec2 aPos;                                              
                                                                                               
out vec2 WorldPos_CS_in;                                                                        
                                                                                                
void main()                                                                                     
{         
    WorldPos_CS_in = aPos;                                                         
}
