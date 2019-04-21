#pragma once
#include <glm/glm.hpp>
#include "../Engine/Shader.h"
#include "sceneElements.h"
#include "../Engine/ScreenQuad.h"
#include "../imgui/imgui.h"
#include "Skybox.h"

#define INT_CEIL(n,d) (int)ceil((float)n/d)



class CloudsModel
{
public:
	friend class VolumetricClouds;

	CloudsModel(sceneElements * scene, Skybox * sky);
	~CloudsModel();
	
	void update();

	virtual void setGui();

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

	glm::vec3 seed, oldSeed;
	unsigned int perlinTex, worley32, weatherTex;

	sceneElements * scene;
	Skybox * sky;

	void generateWeatherMap();
	void generateModelTextures();
	void initVariables();
	void initShaders();


};

