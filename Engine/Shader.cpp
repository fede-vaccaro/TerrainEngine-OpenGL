#include <iostream>
#include <fstream>
#include <sstream>

#include <expected.hpp>

#include "Shader.h"

namespace terrain::gl
{

terrain::expected<Shader> Shader::loadFrom(const std::filesystem::path& shaderPath)
{
	auto maybeShaderCode = _loadTextFromFile(shaderPath);
	if(!maybeShaderCode)
		return maybeShaderCode.error();

	std::string shaderCode = std::move(maybeShaderCode.value());

	auto shadType = shaderTypeFromPath(shaderPath);

	auto shaderId = glCreateShader(static_cast<GLuint>(shadType));
	const char* code_cstr = shaderCode.c_str();

	glShaderSource(shaderId, 1, &code_cstr, NULL);
	glCompileShader(shaderId);

	std::string error_log = checkShaderCompileErrors(shaderId, shadType, shaderPath.filename());

	if(!error_log.empty())
		return terrain::error(std::move(error_log));

	return Shader(shaderPath, shaderId, shadType);	
}

Shader::Shader(Shader && other): _path(std::move(other._path)), _shaderId(other._shaderId), _shaderType(other._shaderType)
{
	other._shaderId = 0;
	other._shaderType = shaderType::INVALID;
}

terrain::expected<std::string> Shader::_loadTextFromFile(const std::filesystem::path& shaderPath) {
	std::stringstream shaderStream;
	std::ifstream shaderFile;

	shaderFile.open(shaderPath);
	shaderStream << shaderFile.rdbuf();

	if (!shaderFile.good())
		return terrain::error("An error occurred while reading " + shaderPath.string());
	
	return shaderStream.str();

}

Shader &Shader::operator=(Shader && other)
{
	_path = std::move(other._path);
	_shaderId = other._shaderId;
	_shaderType = other._shaderType;

	other._shaderId = 0;
	other._shaderType = shaderType::INVALID;

    return *this;
}

Shader::~Shader()
{
	if(_shaderId)
		glDeleteShader(_shaderId);
}

std::string Shader::getName() const
{
	return _path.filename().stem();
}

std::filesystem::path Shader::getPath() const
{
	return _path;
}

shaderType Shader::getType() const 
{
	return _shaderType;
}

GLuint Shader::getId() const
{
	return _shaderId;
}

}