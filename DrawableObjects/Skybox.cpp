#include "Skybox.h"

#include "sceneElements.h"
#include "../Engine/Window.h"
#include "../imgui/imgui.h"

Skybox::Skybox()
{
	skyColorTop = glm::vec3(0.5, 0.7, 0.8)*1.05f;
	skyColorBottom = glm::vec3(0.9, 0.9, 0.95);

	skyboxShader = new ScreenSpaceShader("shaders/sky.frag");
	skyboxFBO = new FrameBufferObject(Window::SCR_WIDTH, Window::SCR_HEIGHT);

	SunsetPreset1();
	DefaultPreset();
}

void Skybox::setGui() {
	ImGui::Begin("Sky controls: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sky colors controls");
	ImGui::ColorEdit3("Sky top color", (float*)&skyColorTop); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Sky bottom color", (float*)&skyColorBottom); // Edit 3 floats representing a color
	ImGui::End();
}

colorPreset Skybox::SunsetPreset() {
	colorPreset preset;

	preset.cloudColorBottom = glm::vec3(89, 96, 109) / 255.f;
	preset.skyColorTop = glm::vec3(177, 174, 119) / 255.f;
	preset.skyColorBottom = glm::vec3(234, 125, 125) / 255.f;

	preset.lightColor = glm::vec3(255, 171, 125) / 255.f;
	preset.fogColor = glm::vec3(85, 97, 120) / 255.f;

	presetSunset = preset;

	return preset;
}

colorPreset Skybox::SunsetPreset1() {
	colorPreset preset;

	preset.cloudColorBottom = glm::vec3(97, 98, 120) / 255.f;
	preset.skyColorTop = glm::vec3(133, 158, 214) / 255.f;
	preset.skyColorBottom = glm::vec3(241, 161, 161) / 255.f;

	preset.lightColor = glm::vec3(255, 201, 201) / 255.f;
	preset.fogColor = glm::vec3(128, 153, 179) / 255.f;

	presetSunset = preset;

	return preset;
}


colorPreset Skybox::DefaultPreset() {
	colorPreset preset;

	preset.cloudColorBottom = (glm::vec3(65., 70., 80.)*(1.5f / 255.f));

	preset.skyColorTop = glm::vec3(0.5, 0.7, 0.8)*1.05f;
	preset.skyColorBottom = glm::vec3(0.9, 0.9, 0.95);

	preset.lightColor = glm::vec3(255, 255, 230) / 255.f;
	preset.fogColor = glm::vec3(0.5, 0.6, 0.7);

	highSunPreset = preset;

	return preset;
}

void Skybox::mixSkyColorPreset(float v, colorPreset p1, colorPreset p2) {
	float a = std::min(std::max(v, 0.0f), 1.0f);
	float b = 1.0 - a;

	//cloudColorBottom = p1.cloudColorBottom*a + p2.cloudColorBottom*b;
	skyColorTop = p1.skyColorTop*a + p2.skyColorTop*b;
	skyColorBottom = p1.skyColorBottom*a + p2.skyColorBottom*b;
	scene->lightColor = p1.lightColor*a + p2.lightColor*b;
	scene->fogColor = p1.fogColor*a + p2.fogColor*b;
}


void Skybox::draw() {
	sceneElements * s = drawableObject::scene;
	skyboxFBO->bind();

	Shader& shader = skyboxShader->getShader();
	shader.use();

	shader.setVec2("resolution", glm::vec2(Window::SCR_WIDTH, Window::SCR_HEIGHT));
	shader.setMat4("inv_proj", glm::inverse(s->projMatrix));
	shader.setMat4("inv_view", glm::inverse(s->cam->GetViewMatrix()));
	
	shader.setVec3("lightDirection", glm::normalize(s->lightPos - s->cam->Position));

	shader.setVec3("skyColorTop", skyColorTop);
	shader.setVec3("skyColorBottom", skyColorBottom);

	skyboxShader->draw();
}

void Skybox::update() {
	auto sigmoid = [](float v) { return 1 / (1.0 + exp(8.0 - v * 40.0)); };
	mixSkyColorPreset(sigmoid(scene->lightDir.y), highSunPreset, presetSunset);
}

Skybox::~Skybox()
{
	delete skyboxFBO;
	delete skyboxShader;
}
