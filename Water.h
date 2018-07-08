#pragma once
#include <glm/glm.hpp>
#include <model.h>
#include <shaderUtils.h>
#include "buffers.h"
#include <GLFW/glfw3.h>

class Water
{
public:
	Water(glm::vec2 position, Shader* shad, float scale, float height, unsigned int dudvMap, unsigned normalMap, Model * waterPlane);
	virtual ~Water();
	void draw(glm::mat4 gVP, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 viewPosition);
	void bindRefractionFBO();
	void bindReflectionFBO();
	void unbindFBO();
	unsigned int reflectionFBO, reflectionTex, reflectionDepth;

	void setPosition(glm::vec2 position, float scale, float height) {
		glm::mat4 identity;
		glm::mat4 scaleMatrix = glm::scale(identity, glm::vec3(scale, scale, scale));
		glm::mat4 transMatrix = glm::translate(identity, glm::vec3(position.x, height, position.y));
		this->modelMatrix = transMatrix * scaleMatrix;
	}

private:

	unsigned int refractionFBO, refractionTex, refractionDepth;
	unsigned int dudvMap, normalMap;
	glm::mat4 modelMatrix;
	Shader * shad;
	Model * waterPlane;
};

