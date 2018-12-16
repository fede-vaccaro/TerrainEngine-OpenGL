#ifndef SCENELEMENTS_H
#define SCENELEMENTS_H

#include <camera.h>
#include <glm/glm.hpp>
#include "buffers.h"

struct sceneElements {

	sceneElements(glm::vec3 &lp, glm::vec3 &lc, glm::vec3& fc, glm::mat4& pm, Camera& c, FrameBufferObject& fbo) :
		lightPos(lp), lightColor(lc), fogColor(fc), projMatrix(pm), cam(c), sceneFBO(fbo) {};

	glm::vec3 &lightPos, &lightColor, &fogColor;
	glm::mat4& projMatrix;
	Camera& cam;
	FrameBufferObject& sceneFBO;
};

#endif