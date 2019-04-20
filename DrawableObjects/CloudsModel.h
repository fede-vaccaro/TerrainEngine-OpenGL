#pragma once
#include <glm/glm.hpp>
#include "../Engine/Shader.h"
#include "sceneElements.h"
#include "../Engine/ScreenQuad.h"
#include "../imgui/imgui.h"

#define INT_CEIL(n,d) (int)ceil((float)n/d)

struct colorPreset {
	glm::vec3 cloudColorBottom, skyColorTop, skyColorBottom, lightColor, fogColor;
};

class CloudsModel
{
public:
	friend class VolumetricClouds;

	CloudsModel(sceneElements * scene);
	~CloudsModel();
	
	void update();

	virtual void setGui();

	colorPreset DefaultPreset();
	colorPreset SunsetPreset();
	colorPreset SunsetPreset1();

	void mixSkyColorPreset(float v, colorPreset p1, colorPreset p2);

private:
	Shader * volumetricCloudsShader, *weatherShader;
	ScreenQuad * postProcessingShader;


	float coverage, cloudSpeed, crispiness, curliness, density, absorption;
	float earthRadius, sphereInnerRadius, sphereOuterRadius;
	float perlinFrequency;
	bool enableGodRays;
	bool enablePowder;
	bool postProcess;
	glm::vec3 cloudColorTop, cloudColorBottom;
	glm::vec3 skyColorTop, skyColorBottom;

	glm::vec3 seed, oldSeed;
	unsigned int perlinTex, worley32, weatherTex;

	sceneElements * scene;

	void generateWeatherMap();
	void generateModelTextures();
	void initVariables();
	void initShaders();


};

