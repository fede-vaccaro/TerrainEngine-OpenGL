#ifndef SHADERUTILS_H
#define SHADERUTILS_H

#include <filesystem>
#include <ostream>
#include <string>

namespace terrain::gl
{
	enum class shaderType: GLuint
	{
		INVALID = GLuint{0},
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
		TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		COMPUTE = GL_COMPUTE_SHADER
	};

	std::ostream& operator<<(std::ostream& os, const shaderType& type);
	std::string to_string(const shaderType& type);
		
	std::string checkShaderCompileErrors(GLuint shader_id, shaderType type, const std::string& shaderName);
	std::string checkProgramCompileErrors(GLuint program_id, const std::string& programName);

	shaderType shaderTypeFromPath(const std::filesystem::path& path);
	
}

#endif