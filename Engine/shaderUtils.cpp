#include <glad/glad.h>
#include <iostream>

#include "shaderUtils.h"

namespace terrain::gl
{

constexpr uint64_t kMaxLogSize = 1024 * 1024;

std::string checkShaderCompileErrors(GLuint shaderId, shaderType type, const std::string& shaderName)
{
	GLint success{-1};
	GLchar infoLog[kMaxLogSize];
	GLsizei logSize{0};

	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

	if (!success)
	{	
		glGetShaderInfoLog(shaderId, kMaxLogSize, &logSize, infoLog);
		std::cout << "AN ERROR OCCURRED WITH THE SHADER " << shaderName << " OF TYPE " << type << ": " << std::string(infoLog, infoLog + logSize) << std::endl;
	}

	return std::string(infoLog, infoLog + logSize);
}

std::string checkProgramCompileErrors(GLuint programId, const std::string& programName)
{
	GLint success{-1};
	GLchar infoLog[kMaxLogSize];
	GLsizei logSize{0};

	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success){
		glGetProgramInfoLog(programId, kMaxLogSize, &logSize, infoLog);
		std::cout << "ERROR WITH LINKING PROGRAM " << programName << ": " << std::string(infoLog, infoLog + logSize) << std::endl;
	}

	return std::string(infoLog, infoLog + logSize);
}



std::ostream& operator<<(std::ostream& os, const shaderType& type)
{
	switch (type)
	{
		case shaderType::INVALID:
			os << "INVALID";
			break;
		case shaderType::VERTEX:
			os << "VERTEX";
			break;
		case shaderType::FRAGMENT:
			os << "FRAGMENT";
			break;
		case shaderType::GEOMETRY:
			os << "GEOMETRY";
			break;
		case shaderType::TESS_CONTROL:
			os << "TESS_CONTROL";
			break;
		case shaderType::TESS_EVALUATION:
			os << "TESS_EVALUATION";
			break;
		case shaderType::COMPUTE:
			os << "COMPUTE";
			break;
		default:
			os << "INVALID SHADER ENUM";
	}

	return os;
}

std::string to_string(const shaderType& type)
{
	std::stringstream ss;
	ss << type;
	return ss.str();
}

shaderType shaderTypeFromPath(const std::filesystem::path& path) {
	auto extension = path.extension();

	if (extension == ".vert")
		return shaderType::VERTEX;
	if (extension == ".frag")
		return shaderType::FRAGMENT;
	if (extension == ".tes")
		return shaderType::TESS_EVALUATION;
	if (extension == ".tcs")
		return shaderType::TESS_CONTROL;
	if (extension == ".geom")
		return shaderType::GEOMETRY;
	if (extension == ".comp")
		return shaderType::COMPUTE;

	return shaderType::INVALID;
}

} // namespace terrain::gl