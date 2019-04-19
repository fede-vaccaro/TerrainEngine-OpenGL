#pragma once
#include "../Engine/ScreenQuad.h"
#include "../Engine/buffers.h"
#include "../Engine/texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera.h>
#include <algorithm>
#include "drawableObject.h"

struct colorPreset {
	glm::vec3 cloudColorBottom, skyColorTop, skyColorBottom, lightColor, fogColor;
};

class VolumetricClouds : public drawableObject
{
public:
	VolumetricClouds(int SW, int SH);
	virtual void draw();
	virtual void setGui();
	~VolumetricClouds();

	void generateWeatherMap();

	colorPreset DefaultPreset();
	colorPreset SunsetPreset();
	colorPreset SunsetPreset1();

	void mixSkyColorPreset(float v, colorPreset p1, colorPreset p2);

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

	void setCoverage(float c) {
		if (c >= 0.0 && c <= 1.0) {
			coverage = c;
		}
	}

	void setPostProcess(bool v) { postProcess = v; }
	bool getPostProcess() { return postProcess; }


private:
	int SCR_WIDTH, SCR_HEIGHT;
	static float coverage, cloudSpeed, crispiness, curliness, density, absorption;
	static float earthRadius, sphereInnerRadius, sphereOuterRadius;
	static float perlinFrequency;
	static bool enableGodRays;
	static bool enablePowder;
	static glm::vec3 cloudColorTop, cloudColorBottom;
	static glm::vec3 skyColorTop, skyColorBottom;
	
	static glm::vec3 seed, oldSeed;

	bool postProcess;
	
	Shader * volumetricCloudsShader, * weatherShader;
	ScreenQuad * ppShader, * copyShader;
	TextureSet * cloudsFBO;
	FrameBufferObject *cloudsPostProcessingFBO;

	static unsigned int perlinTex, worley32, weatherTex;
};

