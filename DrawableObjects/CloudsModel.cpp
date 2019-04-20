#include "CloudsModel.h"
#include <glad/glad.h>
#include "../Engine/texture.h"

colorPreset CloudsModel::SunsetPreset() {
	colorPreset preset;

	preset.cloudColorBottom = glm::vec3(89, 96, 109) / 255.f;
	preset.skyColorTop = glm::vec3(177, 174, 119) / 255.f;
	preset.skyColorBottom = glm::vec3(234, 125, 125) / 255.f;

	preset.lightColor = glm::vec3(255, 171, 125) / 255.f;
	preset.fogColor = glm::vec3(85, 97, 120) / 255.f;
	return preset;
}

colorPreset CloudsModel::DefaultPreset() {
	colorPreset preset;

	preset.cloudColorBottom = (glm::vec3(65., 70., 80.)*(1.5f / 255.f));

	preset.skyColorTop = glm::vec3(0.5, 0.7, 0.8)*1.05f;
	preset.skyColorBottom = glm::vec3(0.9, 0.9, 0.95);

	preset.lightColor = glm::vec3(255, 255, 230) / 255.f;
	preset.fogColor = glm::vec3(0.5, 0.6, 0.7);

	return preset;
}

void CloudsModel::mixSkyColorPreset(float v, colorPreset p1, colorPreset p2) {
	float a = std::min(std::max(v, 0.0f), 1.0f);
	float b = 1.0 - a;

	cloudColorBottom = p1.cloudColorBottom*a + p2.cloudColorBottom*b;
	skyColorTop = p1.skyColorTop*a + p2.skyColorTop*b;
	skyColorBottom = p1.skyColorBottom*a + p2.skyColorBottom*b;
	scene->lightColor = p1.lightColor*a + p2.lightColor*b;
	scene->fogColor = p1.fogColor*a + p2.fogColor*b;
}

colorPreset CloudsModel::SunsetPreset1() {
	colorPreset preset;

	preset.cloudColorBottom = glm::vec3(97, 98, 120) / 255.f;
	preset.skyColorTop = glm::vec3(133, 158, 214) / 255.f;
	preset.skyColorBottom = glm::vec3(241, 161, 161) / 255.f;

	preset.lightColor = glm::vec3(255, 201, 201) / 255.f;
	preset.fogColor = glm::vec3(128, 153, 179) / 255.f;

	return preset;
}

void CloudsModel::setGui() {

	ImGui::Begin("Clouds controls: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Post Proceesing");
	// ImGui::Text("This is some useful text.");               
	// Display some text (you can use a format strings too)
	ImGui::Checkbox("Post Processing (Gaussian Blur)", &postProcess);
	ImGui::Checkbox("God Rays", &enableGodRays);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Clouds rendering");

	ImGui::SliderFloat("Coverage", &coverage, 0.0f, 1.0f);
	ImGui::SliderFloat("Speed", &cloudSpeed, 0.0f, 5.0E3);
	ImGui::SliderFloat("Crispiness", &crispiness, 0.0f, 120.0f);
	ImGui::SliderFloat("Curliness", &curliness, 0.0f, 3.0f);
	ImGui::SliderFloat("Density", &density, 0.0f, 0.1f);
	ImGui::SliderFloat("Light absorption", &absorption, 0.0f, 1.5f);
	ImGui::Checkbox("Enable sugar powder effect", &enablePowder);


	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Dome controls");
	ImGui::SliderFloat("Sky dome radius", &earthRadius, 10000.0f, 5000000.0f);
	ImGui::SliderFloat("Clouds bottom height", &sphereInnerRadius, 1000.0f, 15000.0f);
	ImGui::SliderFloat("Clouds top height", &sphereOuterRadius, 1000.0f, 40000.0f);

	if (ImGui::SliderFloat("Clouds frequency", &perlinFrequency, 0.0f, 4.0f))
		generateWeatherMap();


	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Clouds conlors");
	glm::vec3 * cloudBottomColor = &cloudColorBottom;
	ImGui::ColorEdit3("Cloud color", (float*)cloudBottomColor); // Edit 3 floats representing a color

	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sky colors controls");
	ImGui::ColorEdit3("Sky top color", (float*)&skyColorTop); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Sky bottom color", (float*)&skyColorBottom); // Edit 3 floats representing a color
	ImGui::End();
}

CloudsModel::CloudsModel(sceneElements * scene) : scene(scene)
{
	initVariables();
	initShaders();
	generateModelTextures();
}

void CloudsModel::initShaders()
{
	volumetricCloudsShader = new Shader("volumetricCloudsShader", "shaders/volumetric_clouds.comp");
	postProcessingShader = new ScreenQuad("shaders/clouds_post.frag");
	//compute shaders
	weatherShader = new Shader("weatherMap");
	weatherShader->attachShader("shaders/weather.comp");
	weatherShader->linkPrograms();
}

void CloudsModel::generateModelTextures()
{
	/////////////////// TEXTURE GENERATION //////////////////
	if (!perlinTex) {
		//compute shaders
		Shader comp("perlinWorley");
		comp.attachShader("shaders/perlinworley.comp");
		comp.linkPrograms();

		//make texture
		this->perlinTex = generateTexture3D(128, 128, 128);
		//compute
		comp.use();
		comp.setVec3("u_resolution", glm::vec3(128, 128, 128));
		std::cout << "computing perlinworley!" << std::endl;
		glActiveTexture(GL_TEXTURE0);
		comp.setInt("outVolTex", 0);
		glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));
		std::cout << "computed!!" << std::endl;
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glGenerateMipmap(GL_TEXTURE_3D);
	}

	if (!worley32) {
		//compute shaders
		Shader worley_git("worleyComp");
		worley_git.attachShader("shaders/worley.comp");
		worley_git.linkPrograms();

		//make texture
		this->worley32 = generateTexture3D(32, 32, 32);

		//compute
		worley_git.use();
		worley_git.setVec3("u_resolution", glm::vec3(32, 32, 32));
		std::cout << "computing worley 32!" << std::endl;
		glDispatchCompute(INT_CEIL(32, 4), INT_CEIL(32, 4), INT_CEIL(32, 4));
		std::cout << "computed!!" << std::endl;
		glGenerateMipmap(GL_TEXTURE_3D);
	}


	////////////////////////

	if (!weatherTex) {
		//make texture
		this->weatherTex = generateTexture2D(1024, 1024);

		//compute
		generateWeatherMap();

		seed = scene->seed;
		oldSeed = seed;
	}
}


CloudsModel::~CloudsModel()
{
	delete volumetricCloudsShader;
	delete postProcessingShader;
	delete weatherShader;
}

void CloudsModel::update()
{
	seed = scene->seed;
	if (seed != oldSeed) {
		generateWeatherMap();
		oldSeed = seed;
	}
}

void CloudsModel::generateWeatherMap() {
	bindTexture2D(weatherTex, 0);
	weatherShader->use();
	weatherShader->setVec3("seed", scene->seed);
	weatherShader->setFloat("perlinFrequency", perlinFrequency);
	std::cout << "computing weather!" << std::endl;
	glDispatchCompute(INT_CEIL(1024, 8), INT_CEIL(1024, 8), 1);
	std::cout << "weather computed!!" << std::endl;

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void CloudsModel::initVariables()
{
	cloudSpeed = 450.0;
	coverage = 0.45;
	crispiness = 40.;
	curliness = .1;
	density = 0.02;
	absorption = 0.35;

	earthRadius = 600000.0;
	sphereInnerRadius = 5000.0;
	sphereOuterRadius = 17000.0;

	perlinFrequency = 0.8;

	enableGodRays = false;
	enablePowder = false;
	postProcess = true;

	seed = glm::vec3(0.0, 0.0, 0.0);
	oldSeed = glm::vec3(0.0, 0.0, 0.0);

	cloudColorTop = (glm::vec3(169., 149., 149.)*(1.5f / 255.f));
	cloudColorBottom = (glm::vec3(65., 70., 80.)*(1.5f / 255.f));

	skyColorTop = glm::vec3(0.5, 0.7, 0.8)*1.05f;
	skyColorBottom = glm::vec3(0.9, 0.9, 0.95);

	weatherTex = 0;
	perlinTex = 0;
	worley32 = 0;
}
