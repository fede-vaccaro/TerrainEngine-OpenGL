#ifndef SCENELEMENTS_H
#define SCENELEMENTS_H

#include <camera.h>
#include <glm/glm.hpp>
#include "../Engine/buffers.h"
#include <random>

struct sceneElements {

	sceneElements(glm::vec3 &lp, glm::vec3 &lc, glm::vec3& fc, glm::vec3& seed, glm::mat4& pm, Camera& c, FrameBufferObject& fbo) :
		lightPos(lp), lightColor(lc), fogColor(fc), projMatrix(pm), cam(c), sceneFBO(fbo), seed(seed) {};

	glm::vec3 &lightPos, &lightColor, &fogColor, &seed;
	glm::mat4& projMatrix;
	Camera& cam;
	FrameBufferObject& sceneFBO;
	bool wireframe = false;
};

#endif