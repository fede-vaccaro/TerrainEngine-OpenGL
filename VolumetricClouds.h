#pragma once
#include "ScreenQuad.h"
#include "buffers.h"
#include "computeShader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera.h>
#include <algorithm>
#include "drawableObject.h"

class VolumetricClouds : public drawableObject
{
public:
	VolumetricClouds(int SW, int SH);
	virtual void draw();
	~VolumetricClouds();

	unsigned int getCloudsTexture() { 
		return cloudsPostProcessingFBO->getColorAttachmentTex(0);
	}

	unsigned int getCloudsRawTexture(){
		return cloudsFBO->getColorAttachmentTex(0);
	}

	float getCoverage() {
		return coverage;
	}

	void setCoverage(float c) {
		if (c >= 0.0 && c <= 1.0) {
			coverage = c;
		}
	}

	void setReflection(bool v) { reflection = v; }
	bool getReflection() { return reflection; }

private:
	int SCR_WIDTH, SCR_HEIGHT;
	float coverage;
	ScreenQuad * volumetricCloudsShader, * ppShader, * copyShader;
	int frameIter = 0;
	bool reflection;
	FrameBufferObject * cloudsFBO, *cloudsPostProcessingFBO, * lastFrameCloudsFBO;

	unsigned int perlinTex, worley32, weatherTex;

	glm::mat4 oldFrameVP;
};

