#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

namespace terrain::gl
{

class ShadingProgram
{
public:
	ShadingProgram() = default; 
	ShadingProgram(const ShadingProgram&) = delete;
	ShadingProgram(ShadingProgram&& other);

	ShadingProgram& operator=(const ShadingProgram&) = delete;
	ShadingProgram& operator=(ShadingProgram&& other);

	void use();
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec2(const std::string &name, glm::vec2 vector) const;
	void setVec3(const std::string &name, glm::vec3 vector) const;
	void setVec4(const std::string &name, glm::vec4 vector) const;
	void setMat4(const std::string &name, glm::mat4 matrix) const;
	void setSampler2D(const std::string &name, GLuint texture, GLuint id) const;
	void setSampler3D(const std::string &name, GLuint texture, GLuint id) const;

protected:
	std::string _name;
	GLuint _programId;
	std::vector<Shader> _shaders;

private: 
	friend class ProgramBuilder;
};


class ProgramBuilder
{
	ShadingProgram _program{};

public:
	ProgramBuilder() = default;
	explicit ProgramBuilder(const std::string& programName);
	ProgramBuilder& attachShader(Shader s);
	terrain::expected<ShadingProgram> linkPrograms(bool isCompute = false);
	ProgramBuilder& newProgram(const std::string& programName);
};

};

