#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ComputeShader
{
public:
	unsigned int ID;
	ComputeShader(const char* computePath);
	virtual ~ComputeShader();
	void use();
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec2(const std::string &name, glm::vec2 vector) const;
	void setVec3(const std::string &name, glm::vec3 vector) const;
	void setVec4(const std::string &name, glm::vec4 vector) const;
	void setMat4(const std::string &name, glm::mat4 matrix) const;
protected:
	void checkCompileErrors(unsigned int shader, std::string type, std::string shaderName);
	std::string loadShaderFromFile(const char* shaderPath);
	std::string getShaderName(const char* shaderPath);

};

