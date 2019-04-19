#ifndef GLAD_H
#define GLAD_H
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

#include "Engine/Window.h"
#include "Engine/shader.h"
#include "Engine/ScreenQuad.h"
#include "Engine/texture.h"
#include "DrawableObjects/VolumetricClouds.h"
#include "DrawableObjects/Terrain.h"
#include "DrawableObjects/Skybox.h"
#include "DrawableObjects/Water.h"

#include <camera.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include "Engine/glError.h"
#include "DrawableObjects/sceneElements.h"
#include "DrawableObjects/drawableObject.h"

#include <iostream>
#include <vector>
#include <functional>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Engine/utils.h"

int main()
{

	// camera
	glm::vec3 startPosition(0.0f, 800.0f, 0.0f);
	Camera camera(startPosition);

	int success;
	Window window(success, 1600, 900);
	if (!success) return -1;

	// GUI
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");

	window.camera = &camera;

	//glm::vec3 fogColor(0.6 + 0.1, 0.71 + 0.1, 0.85 + 0.1);
	glm::vec3 fogColor(0.5,0.6,0.7);
	//fogColor *= 0.7;
	glm::vec3 lightColor(255, 255, 230);
	lightColor /= 255.0;

	FrameBufferObject SceneFBO(Window::SCR_WIDTH, Window::SCR_HEIGHT);
	glm::vec3 lightPosition, seed;
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 5.f, 10000000.0f);

	seed = genRandomVec3();
	//Every scene object need this information to be rendered
	sceneElements scene(lightPosition, lightColor, fogColor, seed, proj, camera, SceneFBO);

	drawableObject::scene = &scene;


	int gridLength = 120;
	Terrain terrain(gridLength);

	float waterHeight = 120.;
	Water water(glm::vec2(0.0, 0.0), gridLength, waterHeight);
	terrain.waterPtr = &water;

	Skybox skybox; //unused
	
	VolumetricClouds volumetricClouds(Window::SCR_WIDTH, Window::SCR_HEIGHT);
	VolumetricClouds reflectionVolumetricClouds(1280, 720); //a different object is needed because it has a state-dependent draw method
	reflectionVolumetricClouds.setPostProcess(false);

	ScreenQuad PostProcessing("shaders/post_processing.frag");
	ScreenQuad fboVisualizer("shaders/visualizeFbo.frag");

	bool useSeed = true;
	glm::vec3 lightDir = glm::vec3(-.5, 0.5, 1.0);

	colorPreset presetHighSun = volumetricClouds.DefaultPreset();
	colorPreset presetSunset = volumetricClouds.SunsetPreset();

	auto sigmoid = [](float v) { return 1/(1.0 + exp(8.0-v*40.0)); };
	volumetricClouds.mixSkyColorPreset(sigmoid(lightDir.y), presetHighSun, presetSunset);

	while (window.continueLoop())
	{
		lightDir = glm::normalize(lightDir);
		scene.lightPos = lightDir*1e9f + camera.Position;
		// input
		window.processInput(1 / ImGui::GetIO().Framerate);

		//update tiles position to make the world infinite
		terrain.updateTilesPositions();

		SceneFBO.bind();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// toggle/untoggle wireframe mode
		if (scene.wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Camera (View Matrix) setting
		glm::mat4 view = scene.cam.GetViewMatrix();
		scene.projMatrix = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 5.f,10000000.0f);

		
		//draw to water reflection buffer object
		water.bindReflectionFBO();
		glClearColor(0.0, 0.4*0.8, 0.7*0.8, 1.0);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.cam.invertPitch();
		scene.cam.Position.y -= 2 * (scene.cam.Position.y - water.getHeight());
		
		terrain.up = 1.0;
		terrain.draw();
		FrameBufferObject const& reflFBO = water.getReflectionFBO();
		
		ScreenQuad::disableTests();

		reflectionVolumetricClouds.draw();
		water.bindReflectionFBO(); //rebind refl buffer; reflVolumetricClouds unbound it

		
		Shader& post = PostProcessing.getShader();
		post.use();
		post.setVec2("resolution", glm::vec2(1280, 720));
		post.setSampler2D("screenTexture", reflFBO.tex, 0);
		post.setSampler2D("depthTex", reflFBO.depthTex, 2);
		post.setSampler2D("cloudTEX", reflectionVolumetricClouds.getCloudsRawTexture(), 1);
		PostProcessing.draw();

		ScreenQuad::enableTests();
		
		scene.cam.invertPitch();
		scene.cam.Position.y += 2 * abs(scene.cam.Position.y - water.getHeight());
		
		//draw to water refraction buffer object
		water.bindRefractionFBO();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		terrain.up = -1.0;
		terrain.draw();

		// draw terrain
		scene.sceneFBO.bind();
		terrain.draw();
		water.draw();

		//disable test for quad rendering
		ScreenQuad::disableTests();

		// scene post processing - blending between main scene texture and clouds texture
		volumetricClouds.draw();

		// blend volumetric clouds rendering with terrain and apply some post process
		unbindCurrentFrameBuffer(); // on-screen drawing
		//Shader& post = PostProcessing.getShader();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		post.use();
		post.setVec2("resolution", glm::vec2(Window::SCR_WIDTH, Window::SCR_HEIGHT));
		post.setVec3("cameraPosition", scene.cam.Position);
		post.setSampler2D("screenTexture", SceneFBO.tex, 0);
		post.setSampler2D("cloudTEX", volumetricClouds.getCloudsTexture(), 1);
		post.setSampler2D("depthTex", SceneFBO.depthTex, 2);
		post.setSampler2D("cloudDistance", volumetricClouds.getCloudsTexture(3), 3);

		post.setBool("wireframe", scene.wireframe);

		post.setMat4("VP", scene.projMatrix * view);
		PostProcessing.draw();



		// Texture visualizer
		Shader& fboVisualizerShader = fboVisualizer.getShader();
		fboVisualizerShader.use();
		fboVisualizerShader.setSampler2D("fboTex", 0, 0);
		//fboVisualizer.draw();
		
		{
			static int counter = 0;

			ImGui::Begin("Scene controls: ");
			volumetricClouds.setGui();
			terrain.setGui();
			water.setGui();

			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Other controls");
			if (ImGui::DragFloat3("Light Position", &lightDir[0], 0.01, -1.0, 1.0)) {
				auto saturate = [](float v) { return std::min(std::max(v, 0.0f), 0.8f); };
				
				lightDir.y = saturate(lightDir.y);
				volumetricClouds.mixSkyColorPreset(sigmoid(lightDir.y), presetHighSun, presetSunset);

			}
			ImGui::InputFloat3("Camera Position", &camera.Position[0], 7);
			ImGui::ColorEdit3("Light color", (float*)&lightColor); 
			ImGui::ColorEdit3("Fog color", (float*)&fogColor);
			ImGui::SliderFloat("Camera speed", &camera.MovementSpeed, 0.0, SPEED*3.0);
			
			

			
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
			if (ImGui::Button("Sunset Preset 1")) {
				presetSunset = volumetricClouds.SunsetPreset();
				volumetricClouds.mixSkyColorPreset(sigmoid(lightDir.y), presetHighSun, presetSunset);
			}
			ImGui::SameLine();
			if (ImGui::Button("Sunset Preset 2")) {
				presetSunset = volumetricClouds.SunsetPreset1();
				volumetricClouds.mixSkyColorPreset(sigmoid(lightDir.y), presetHighSun, presetSunset);
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		//camera.Position += glm::vec3(200, 0, 0);

		//gui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		window.swapBuffersAndPollEvents();
	}

	//
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	// close glfw
	window.terminate();
}
