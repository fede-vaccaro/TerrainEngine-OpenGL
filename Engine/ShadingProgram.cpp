#include "ShadingProgram.h"

#include <algorithm>

namespace terrain::gl{

ShadingProgram::ShadingProgram(ShadingProgram &&other): _name(std::move(other._name)), _programId(other._programId), _shaders(std::move(other._shaders))
{
	other._programId = 0;
}

ShadingProgram &ShadingProgram::operator=(ShadingProgram &&other)
{
    this->_name = std::move(other._name);
	this->_programId = std::move(other._programId);
	this->_shaders = std::move(other._shaders);

	other._programId = 0;

	return *this;
}

void ShadingProgram::use()
{
	glUseProgram(_programId);
}

// utility uniform functions
// ------------------------------------------------------------------------
void ShadingProgram::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(_programId, name.c_str()), (int)value);
}

void ShadingProgram::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(_programId, name.c_str()), value);
}

void ShadingProgram::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(_programId, name.c_str()), value);
}
void ShadingProgram::setVec2(const std::string &name, glm::vec2 vector) const
{
	unsigned int location = glGetUniformLocation(_programId, name.c_str());
	glUniform2fv(location, 1, glm::value_ptr(vector));
}
void ShadingProgram::setVec3(const std::string &name, glm::vec3 vector) const
{
	unsigned int location = glGetUniformLocation(_programId, name.c_str());
	glUniform3fv(location, 1, glm::value_ptr(vector));
}

void ShadingProgram::setVec4(const std::string &name, glm::vec4 vector) const
{
	unsigned int location = glGetUniformLocation(_programId, name.c_str());
	glUniform4fv(location, 1, glm::value_ptr(vector));
}

void ShadingProgram::setMat4(const std::string &name, glm::mat4 matrix) const
{
	unsigned int mat = glGetUniformLocation(_programId, name.c_str());
	glUniformMatrix4fv(mat, 1, false, glm::value_ptr(matrix));
}

void ShadingProgram::setSampler2D(const std::string &name, GLuint texture, GLuint id) const
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->setInt(name, id);
}
void ShadingProgram::setSampler3D(const std::string &name, GLuint texture, GLuint id) const
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_3D, texture);
	this->setInt(name, id);
}

ProgramBuilder::ProgramBuilder(const std::string &programName)
{
	_program._name = programName;
	_program._programId = glCreateProgram();
}

ProgramBuilder& ProgramBuilder::newProgram(const std::string &programName)
{
    _program = {};

	_program._name = programName;
	_program._programId = glCreateProgram();

	return *this;
}

ProgramBuilder& ProgramBuilder::attachShader(Shader s)
{
	glAttachShader(_program._programId, s.getId());

	_program._shaders.emplace_back(std::move(s));
	
	return *this;
}

terrain::expected<ShadingProgram> ProgramBuilder::linkPrograms(bool isCompute)
{
	std::string which_shader{};
	
	bool anyCompute = std::any_of(_program._shaders.begin(), _program._shaders.end(), [&which_shader] (const Shader& s) mutable {
		which_shader = s.getName();
		return s.getType() == shaderType::COMPUTE;
	});

	if(!isCompute && anyCompute)
	{
		std::string error = which_shader + " is a ComputeShader: not allowed in a Rendering pipeline program";
		return terrain::error(error);
	}

	glLinkProgram(_program._programId);
	std::string error_log = checkProgramCompileErrors(_program._programId, _program._name);

	if(!error_log.empty())
	{
		std::string error = "An error occurred while linking the program " + std::to_string(_program._programId) + ": \n" + error_log;
		return terrain::error(error);
	}

	return std::move(_program);
}

}