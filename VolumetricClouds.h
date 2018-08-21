#pragma once
#include "ScreenQuad.h"
#include "buffers.h"
#include "computeShader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera.h>

class VolumetricClouds
{
public:
	VolumetricClouds(int SW, int SH, Camera * cam);
	void draw(glm::mat4 view, glm::mat4 proj, glm::vec3 lightPosition, unsigned int depthMap);
	~VolumetricClouds();

	unsigned int getCloudsTexture() {
		return cloudsPostProcessingFBO->tex;
	}

private:
	int SCR_WIDTH, SCR_HEIGHT;
	float coverage;
	Camera * camera;
	ScreenQuad * volumetricCloudsShader, * ppShader;

	FrameBufferObject * cloudsFBO, *cloudsPostProcessingFBO;

	unsigned int perlinTex, worley32, weatherTex;
};

