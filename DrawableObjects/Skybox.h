#pragma once

#include <glm/glm.hpp>
#include "../Engine/ScreenQuad.h"
#include "drawableObject.h"
#include <glm/gtc/matrix_transform.hpp>

struct colorPreset {
	glm::vec3 cloudColorBottom, skyColorTop, skyColorBottom, lightColor, fogColor;
};

class Skybox : public drawableObject
{
public:
	friend class VolumetricClouds;
	Skybox();
	~Skybox();

	virtual void draw();

	virtual void setGui();

	void update();

	colorPreset DefaultPreset();
	colorPreset SunsetPreset();
	colorPreset SunsetPreset1();

	void mixSkyColorPreset(float v, colorPreset p1, colorPreset p2);

	unsigned int getSkyTexture() {
		return skyboxFBO->tex;
	}

private:
	glm::vec3 skyColorTop, skyColorBottom;

	ScreenQuad * skyboxShader;
	FrameBufferObject * skyboxFBO;

	colorPreset presetSunset;
};

