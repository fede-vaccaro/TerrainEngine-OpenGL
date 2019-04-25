#pragma once

#include <glm/glm.hpp>
#include "../Engine/ScreenSpaceShader.h"
#include "drawableObject.h"
#include <glm/gtc/matrix_transform.hpp>

struct colorPreset {
	glm::vec3 cloudColorBottom, skyColorTop, skyColorBottom, lightColor, fogColor;
};

//This class handles the sky and its color. It draw the sky in the screen space, as only-fragment shader. 
//It doesn't use the usual skybox because it's like a nice placeholder, if one day it will extended to perform atmospheric light scattering. 
class Skybox : public drawableObject {
public:
	friend class VolumetricClouds;
	Skybox();
	~Skybox();

	virtual void draw();
	virtual void setGui();
	virtual void update();

	colorPreset DefaultPreset();
	colorPreset SunsetPreset();
	colorPreset SunsetPreset1();

	void mixSkyColorPreset(float v, colorPreset p1, colorPreset p2);

	unsigned int getSkyTexture() {
		return skyboxFBO->tex;
	}

private:
	glm::vec3 skyColorTop, skyColorBottom;

	ScreenSpaceShader * skyboxShader;
	FrameBufferObject * skyboxFBO;

	colorPreset presetSunset, highSunPreset;
};

