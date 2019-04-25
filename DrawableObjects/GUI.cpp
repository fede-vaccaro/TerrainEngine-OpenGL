#include "GUI.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include "../Engine/utils.h"

GUI::GUI(Window& window)
{
	// GUI
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void GUI::draw()
{
	
		sceneElements& scene = *this->scene;

		for (drawableObject* obj : subscribers) {
			obj->setGui();
		}

		ImGui::Begin("Scene controls: ");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Other controls");
		if (ImGui::DragFloat3("Light Position", &scene.lightDir[0], 0.01, -1.0, 1.0)) {
			auto saturate = [](float v) { return std::min(std::max(v, 0.0f), 0.8f); };
			scene.lightDir.y = saturate(scene.lightDir.y);
			//skybox.update();
		}
		ImGui::InputFloat3("Camera Position", &(scene.cam->Position[0]), 7);
		ImGui::ColorEdit3("Light color", (float*)&scene.lightColor);
		ImGui::ColorEdit3("Fog color", (float*)&scene.fogColor);
		ImGui::SliderFloat("Camera speed", &scene.cam->MovementSpeed, 0.0, SPEED*3.0);




		ImGui::Checkbox("Wireframe mode", &scene.wireframe);

		if (ImGui::Button("Generate seed"))
			scene.seed = genRandomVec3();
		//ImGui::SameLine();
		//ImGui::Text("Generate a new seed");
		ImGui::SameLine();
		if (ImGui::Button("Use default seed"))
			scene.seed = glm::vec3(0.0, 0.0, 0.0);

		/*ImGui::SameLine();
		if (ImGui::Button("Default Preset")) {
			volumetricClouds.DefaultPreset();
			lightDir.y = 0.5;
		}*/
		//ImGui::SameLine();
		/*if (ImGui::Button("Sunset Preset 1")) {
			skybox.SunsetPreset();
		}
		ImGui::SameLine();
		if (ImGui::Button("Sunset Preset 2")) {
			skybox.SunsetPreset1();
		}*/

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	

	//actual drawing
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::update()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

GUI & GUI::subscribe(drawableObject * subscriber)
{
	subscribers.push_back(subscriber);

	return *this;
}


GUI::~GUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
