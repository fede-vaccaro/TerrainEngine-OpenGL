#pragma once

#include <glm/glm.hpp>
#include "shader.h"
#include <glm/gtc/matrix_transform.hpp>


class Skybox
{
public:
	Skybox();
	~Skybox();

	void draw(glm::mat4 view, glm::mat4 proj);

private:
	unsigned int skyboxVAO, skyboxVBO;
	glm::mat4 model;
	Shader * shader;
};

