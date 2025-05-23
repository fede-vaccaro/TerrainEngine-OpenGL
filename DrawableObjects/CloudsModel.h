#pragma once
#include <glm/glm.hpp>

#include "sceneElements.h"
#include "Skybox.h"

#include "../Engine/Shader.h"
#include "../Engine/ScreenSpaceShader.h"
#include "../imgui/imgui.h"

#define INT_CEIL(n,d) (int)ceil((float)n/d)

namespace terrain{

//CloudsModel is responsible to collect the attributes and shaders that will be needed to render the volumetric clouds. Also, it creates the noises which models the clouds.
class CloudsModel : public drawableObject
{
public:
	friend class VolumetricClouds;

	CloudsModel(sceneElements * scene, Skybox * sky);
	~CloudsModel();
	
	// fake implementation, it's needed to let this class being a drawableObject to subscribe to GUI class. must be fixed
	virtual void draw() {};

	virtual void update();
	virtual void setGui();

private:
	gl::ShadingProgram _volumetricCloudsShader;
	gl::ShadingProgram _weatherShader;
	ScreenSpaceShader * postProcessingShader;

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

} // namespace terrain