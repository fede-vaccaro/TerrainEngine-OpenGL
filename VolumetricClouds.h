#pragma once
#include "ScreenQuad.h"
#include "buffers.h"
#include "computeShader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera.h>
#include <algorithm>

class VolumetricClouds
{
public:
	VolumetricClouds(int SW, int SH, Camera * cam);
	void draw(glm::mat4 view, glm::mat4 proj, glm::vec3 lightPosition, unsigned int depthMap);
	~VolumetricClouds();

	unsigned int getCloudsTexture() {
		return cloudsPostProcessingFBO->getColorAttachmentTex(0);
	}

	float getCoverage() {
		return coverage;
	}

	void setCoverage(float c) {
		if (c >= 0.0 && c <= 1.0) {
			coverage = c;
		}
	}

private:
	int SCR_WIDTH, SCR_HEIGHT;
	float coverage;
	Camera * camera;
	ScreenQuad * volumetricCloudsShader, * ppShader;

	FrameBufferObject * cloudsFBO, *cloudsPostProcessingFBO;

	unsigned int perlinTex, worley32, weatherTex;
};

