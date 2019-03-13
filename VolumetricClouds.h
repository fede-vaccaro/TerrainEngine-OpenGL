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
	virtual void setGui();
	~VolumetricClouds();



	unsigned int getCloudsTexture() { 
		return (postProcess ? cloudsPostProcessingFBO->getColorAttachmentTex(0) : getCloudsRawTexture());
	}

	unsigned int getCloudsTexture(int i) {
		return cloudsFBO->getColorAttachmentTex(i);
	}

	unsigned int getCloudsRawTexture(){
		return cloudsFBO->getColorAttachmentTex(0);
	}

	float getCoverage() {
		return coverage;
	}

	float * getCoveragePointer() {
		return &coverage;
	}
	bool * getPostProcPointer() {
		return &postProcess;
	}

	void setCoverage(float c) {
		if (c >= 0.0 && c <= 1.0) {
			coverage = c;
		}
	}

	float * getCloudSpeedPtr() {
		return &cloudSpeed;
	}

	glm::vec3 * getCloudColorTopPtr() {
		return &cloudColorTop;
	}

	glm::vec3 * getCloudColorBottomPtr() {
		return &cloudColorBottom;
	}

	glm::vec3 * getSkyTopColorPtr() {
		return &skyColorTop;
	}

	glm::vec3 * getSkyBottomColorPtr() {
		return &skyColorBottom;
	}

	float * getCloudCrispinessPtr() {
		return &crispiness;
	}

	void setPostProcess(bool v) { postProcess = v; }
	bool getPostProcess() { return postProcess; }


private:
	int SCR_WIDTH, SCR_HEIGHT;
	static float coverage, cloudSpeed, crispiness, density, absorption;
	static glm::vec3 cloudColorTop, cloudColorBottom;
	static glm::vec3 skyColorTop, skyColorBottom;
	
	bool postProcess;
	
	Shader * volumetricCloudsShader;
	ScreenQuad * ppShader, * copyShader;
	TextureSet * cloudsFBO;
	FrameBufferObject *cloudsPostProcessingFBO;

	unsigned int perlinTex, worley32, weatherTex;
};

