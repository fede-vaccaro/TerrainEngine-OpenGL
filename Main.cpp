#ifndef GLAD_H
#define GLAD_H
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

#include "Window.h"
#include "shader.h"
//#include "TessShader.h"
//#include "computeShader.h"
#include "ScreenQuad.h"
#include "texture.h"
#include "VolumetricClouds.h"
#include "Tile.h"
#include "Skybox.h"
#include "Water.h"

#include <camera.h>
#include <stb_image.h>
//#include <model.h>

#include "TextArea.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include "glError.h"
#include "sceneElements.h"
#include "drawableObject.h"

#include <map>
#include <stdlib.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <functional>

const int MAX_FPS = 144;

float t1 = 0.0, t2 = 0.0, frameTime = 0.0; // time variables

int main()
{

	// camera
	glm::vec3 startPosition(0.0f, 800.0f, 0.0f);
	Camera camera(startPosition);

	int success;
	Window window(success, 1920, 1080);
	if (!success) return -1;

	window.camera = &camera;

	glm::vec3 fogColor(0.6 + 0.1, 0.71 + 0.1, 0.85 + 0.1);
	fogColor *= 0.7;
	glm::vec3 lightColor(255, 255, 230);
	lightColor /= 255.0;

	float scale = 100.0f,  dispFactor = 16.0;
	//TileController tc(scale, dispFactor, 51);
	int gl = 120;
	Tile terrain(scale, dispFactor, gl);
	Skybox skybox; //unused
	VolumetricClouds volumetricClouds(Window::SCR_WIDTH, Window::SCR_HEIGHT);
	VolumetricClouds reflectionVolumetricClouds(1280, 720); //a different object is needed because it has a state-dependent draw method
	reflectionVolumetricClouds.setReflection(true);

	float waterHeight = 128.0 + 50.0;
	Water water(glm::vec2(0.0, 0.0), scale*gl, waterHeight);

	std::cout << "============ OPENING POST PROCESSING SHADER ============" << std::endl; // its purpose is to merge different framebuffer and add some postproc if needed
	ScreenQuad PostProcessing("shaders/post_processing.frag");

	FrameBufferObject SceneFBO(Window::SCR_WIDTH, Window::SCR_HEIGHT);
	glm::vec3 lightPosition;
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 5.f, 10000000.0f);

	//Every scene object need this information to be rendered
	sceneElements scene(lightPosition, lightColor, fogColor, proj, camera, SceneFBO);

	drawableObject::scene = &scene;

	ScreenQuad fboVisualizer("shaders/visualizeFbo.frag");

	while (window.continueLoop())
	{

		t1 = glfwGetTime();

		scene.lightPos = glm::vec3(-.5, .4, 1.0)*1e9f + camera.Position;
		// input
		window.processInput(frameTime);

		//update tiles position to make the world infinite
		terrain.updateTiles();

		SceneFBO.bind();
		// render
		//glEnable(GL_MULTISAMPLE);
		//glEnable(GL_FRAMEBUFFER_SRGB); //gamma correction

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		// toggle/untoggle wireframe mode
		if (window.isWireframeActive()) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			Tile::drawFog = false;
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			Tile::drawFog = true;
		}

		// Camera (View Matrix) setting
		glm::mat4 view = scene.cam.GetViewMatrix();
		scene.projMatrix = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 5.f,10000000.0f);

		
		//draw to water reflection buffer object
		water.bindReflectionFBO();
		glClearColor(0.0, 0.4*0.8, 0.7*0.8, 1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.cam.invertPitch();
		scene.cam.Position.y -= 2 * (scene.cam.Position.y - waterHeight);
		
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
		scene.cam.Position.y += 2 * abs(scene.cam.Position.y - waterHeight);
		
		//draw to water refraction buffer object
		water.bindRefractionFBO();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
		post.use();
		post.setVec2("resolution", glm::vec2(Window::SCR_WIDTH, Window::SCR_HEIGHT));
		post.setSampler2D("screenTexture", SceneFBO.tex, 0);
		post.setSampler2D("cloudTEX", volumetricClouds.getCloudsTexture(), 1);
		post.setSampler2D("depthTex", SceneFBO.depthTex, 2);
		PostProcessing.draw();

		///////////////
		// Texture visualizer
		Shader& fboVisualizerShader = fboVisualizer.getShader();
		fboVisualizerShader.use();
		fboVisualizerShader.setSampler2D("fboTex", 0, 0);
		//fboVisualizer.draw();
		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		window.swapBuffersAndPollEvents();

		t2 = glfwGetTime();
		frameTime = t2 - t1;
		float timeToSleep = 1000.0f / MAX_FPS - (t2 - t1)*1000.0f;
		if (timeToSleep > 0.0f) {
			//	Sleep(timeToSleep);
		}
		t2 = glfwGetTime();
		frameTime = t2 - t1;
		//std::cout << 1.0f / frameTime << " FPS" << std::endl;

	}

	// close glfw
	window.terminate();
}
