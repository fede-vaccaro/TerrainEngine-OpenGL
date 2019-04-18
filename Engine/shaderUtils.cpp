#include "shaderUtils.h"
#include <glad/glad.h>
#include <iostream>

bool checkCompileErrors(unsigned int shader, std::string type, std::string shaderName)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR: SHADER" << shaderName << "COMPILATION ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}

	//if (success) {
	//	std::cout << type + " SHADER SUCCESSFULLY COMPILED AND/OR LINKED!" << std::endl;
	//}
	return success;
}

std::string getShaderName(const char* path) {
	std::string pathstr = std::string(path);
	const size_t last_slash_idx = pathstr.find_last_of("/");
	if (std::string::npos != last_slash_idx)
	{
		pathstr.erase(0, last_slash_idx + 1);
	}
	return pathstr;
}
shaderType getShaderType(const char* path) {
	std::string type = getShaderName(path);
	const size_t last_slash_idx = type.find_last_of(".");
	if (std::string::npos != last_slash_idx)
	{
		type.erase(0, last_slash_idx + 1);
	}
	if (type == "vert")
		return shaderType(GL_VERTEX_SHADER, "VERTEX");
	if (type == "frag")
		return shaderType(GL_FRAGMENT_SHADER, "FRAGMENT");
	if (type == "tes")
		return shaderType(GL_TESS_EVALUATION_SHADER, "TESS_EVALUATION");
	if (type == "tcs")
		return shaderType(GL_TESS_CONTROL_SHADER, "TESS_CONTROL");
	if (type == "geom")
		return shaderType(GL_GEOMETRY_SHADER, "GEOMETRY");
	if (type == "comp")
		return shaderType(GL_COMPUTE_SHADER, "COMPUTE");
}
