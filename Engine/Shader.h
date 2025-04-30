#pragma once

#include <filesystem>
#include <string>

#include <glad/glad.h>

#include <expected.hpp>

#include "shaderUtils.h"

namespace terrain::gl
{

class Shader
{
	std::filesystem::path _path;
	GLuint _shaderId{0};
	shaderType _shaderType;

public:
	static terrain::expected<Shader> loadFrom(const std::filesystem::path& path);
	
	Shader(const Shader&) = delete;
	Shader(Shader&& other);

	Shader& operator=(const Shader&) = delete;
	Shader& operator=(Shader&& other);
	
	virtual ~Shader();

	std::filesystem::path getPath() const;
	std::string getName() const;
	shaderType getType() const;
	GLuint getId() const;
	
private:
	static terrain::expected<std::string> _loadTextFromFile(const std::filesystem::path& shaderPath);

	Shader(const std::filesystem::path& path, GLuint shadId, shaderType shadType): _path(path), _shaderId(shadId), _shaderType(shadType) {}
};

} // namespace terrain::gl

