#pragma once

#include <glm/glm.hpp>
#include "../Engine/shader.h"
#include "drawableObject.h"
#include <glm/gtc/matrix_transform.hpp>


class Skybox : public drawableObject
{
public:
	Skybox();
	~Skybox();

	virtual void draw();

private:
	unsigned int skyboxVAO, skyboxVBO;
	glm::mat4 model;
	Shader * shader;
};

