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
		return (postProcess ? cloudsPostProcessingFBO->getColorAttachmentTex(0) : getCloudsRawTexture());
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

	void setPostProcess(bool v) { postProcess = v; }
	bool getPostProcess() { return postProcess; }

private:
	int SCR_WIDTH, SCR_HEIGHT;
	float coverage;
	Shader * volumetricCloudsShader;
	ScreenQuad * ppShader, * copyShader;
	bool postProcess;
	TextureSet * cloudsFBO;
	FrameBufferObject *cloudsPostProcessingFBO;

	unsigned int perlinTex, worley32, weatherTex;
};

