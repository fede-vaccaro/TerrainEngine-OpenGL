#ifndef SCENELEMENTS_H
#define SCENELEMENTS_H

#include <camera.h>
#include <glm/glm.hpp>
#include "../Engine/buffers.h"
#include <random>

struct sceneElements {

	glm::vec3 lightPos, lightColor, fogColor, seed;
	glm::mat4 projMatrix;
	Camera * cam;
	FrameBufferObject * sceneFBO;
	bool wireframe = false;
};

#endif