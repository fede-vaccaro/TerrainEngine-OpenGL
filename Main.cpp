#ifndef GLAD_H
#define GLAD_H
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

#include "Window.h"
#include "shader.h"
#include "TessShader.h"
#include "computeShader.h"
#include "ScreenQuad.h"
#include "texture.h"
#include "VolumetricClouds.h"
#include "TileController.h"
#include "Tile.h"
#include "Skybox.h"

#include <camera.h>
#include <stb_image.h>
#include <model.h>

#include "TextArea.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include "glError.h"

#include <map>
#include <stdlib.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <functional>

void clear() {
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}

const int MAX_FPS = 144;

TextArea * gui = 0;
int gui_i = 0;
int gui_el;
float deltaMagnitude = 1.0;
bool snow = false;



bool updateShell = true;

// camera
glm::vec3 startPosition(0.0f, 1500.0f, 0.0f);
Camera camera(startPosition);

int success;
Window window(success, &camera);





float t1 = 0.0, t2 = 0.0, frameTime = 0.0; // time variables
//bool wireframe = false;
glm::mat4 identityMatrix;

std::vector<std::function<float()>> getters;
std::vector<std::function< void(float)> > setters;

int main()
{
	if (!success) return -1;
	if (!window.inMain()) return -1;

	Shader waterShader("shaders/waterVertexShader.vert", "shaders/waterFragmentShader.frag");
	std::cout << "============= CREATING TSHADER ==============" << std::endl;
	TessellationShader tshader("shaders/tessVertexShader.vert", "shaders/tessControlShader.tcs", "shaders/tessEvaluationShader.tes", "shaders/tessFragmentShader.frag");
	std::cout << "============= TSHADER CREATED ==============" << std::endl;


	//loading models
	Model waterPlane("resources/plane.obj", GL_TRIANGLES);
	Model plane_("resources/plane.obj", GL_PATCHES);


	glm::vec3 fogColor(0.6 + 0.1, 0.71 + 0.1, 0.85 + 0.1);
	fogColor *= 0.7;
	glm::vec3 lightColor(255, 255, 230);
	lightColor /= 255.0;

	float scale = 100.0f,  dispFactor = 16.0;
	TileController tc(scale, dispFactor, &camera, &tshader, &waterShader);
	Skybox skybox;
	VolumetricClouds volumetricClouds(Window::SCR_WIDTH, Window::SCR_HEIGHT, &camera);


	TextArea::setWindow(window.w);
	screen scr;
	scr.WIDTH = Window::Window::SCR_WIDTH;
	scr.HEIGHT = Window::Window::SCR_HEIGHT;
	TextArea::setScreen(&scr);

	float coverage = 0.18;

	gui = new TextArea(10, 10, 250, 0); // l'altezza � adattata al contenuto
	int octaves = tc.getOctaves();
	float df = tc.getDispFactor();
	float wh = tc.getWaterHeight();
	float gc = tc.getGrassCoverage();
	float f = tc.getFreq();
	float tm = tc.getTessMultiplier();
	bool snowy = false;

	if (gui) gui->addElement(std::string("Octaves: "), &octaves);
	if (gui) gui->addElement(std::string("Terrain Height: "), &df);
	if (gui) gui->addElement(std::string("Water height: "), &wh);
	if (gui) gui->addElement(std::string("Grass coverage factor:"), &gc);
	if (gui) gui->addElement(std::string("Frequency: "), &f);
	if (gui) gui->addElement(std::string("Tessellation Multiplier: "), &tm);
	if (gui) gui->addElement(std::string("Clouds coverage: "), &coverage);
	if (gui) gui->addElement(std::string("Delta magnitude: "), &deltaMagnitude);


	getters.push_back(0);
	getters.push_back([&tc] { return tc.getOctaves(); });
	getters.push_back([&tc] { return tc.getDispFactor(); });
	getters.push_back([&tc] { return tc.getWaterHeight(); });
	getters.push_back([&tc] { return tc.getGrassCoverage(); });
	getters.push_back([&tc] { return tc.getFreq(); });
	getters.push_back([&tc] { return tc.getTessMultiplier(); });
	getters.push_back([&coverage] { return coverage; });
	getters.push_back(0);

	setters.push_back(0);
	setters.push_back([&tc](float value) { tc.setOctaves(value);       });
	setters.push_back([&tc](float value) { tc.setDispFactor(value);    });
	setters.push_back([&tc](float value) { tc.setWaterHeight(value);   });
	setters.push_back([&tc](float value) { tc.setGrassCoverage(value); });
	setters.push_back([&tc](float value) { tc.setFreq(value);          });
	setters.push_back([&tc](float value) { tc.setTessMultiplier(value);  });
	setters.push_back([&coverage](float value) { coverage = value; });
	setters.push_back(0);

	gui_el = (setters.size() == getters.size() ? setters.size() : -1);

	unsigned int * textures = new unsigned int[4];
	textures[0] = TextureFromFile("sand.jpg", "resources", false);
	textures[1] = TextureFromFile("grass.jpg", "resources", false);
	textures[2] = TextureFromFile("rock4.jpg", "resources", false);
	textures[3] = TextureFromFile("snow2.jpg", "resources", false);


	std::cout << "============ OPENING POST PROCESSING SHADER ============" << std::endl;
	ScreenQuad PostProcessing("shaders/post_processing.frag");

	FrameBufferObject SceneFBO(Window::SCR_WIDTH, Window::SCR_HEIGHT);
	

	while (window.continueLoop())
	{

		octaves = getters[1]();
		df = getters[2]();
		wh = getters[3]();
		gc = getters[4]();
		f = getters[5]();
		tm = getters[6]();
		float dm = deltaMagnitude;

		if (updateShell) {
			//clear();
			std::cout << (gui_i == 1 ? "->" : "  ") << "Octaves: " << octaves << std::endl;
			std::cout << (gui_i == 2 ? "->" : "  ") << "Terrain Height : " << df << std::endl;
			std::cout << (gui_i == 3 ? "->" : "  ") << "Water height: " << wh << std::endl;
			std::cout << (gui_i == 4 ? "->" : "  ") << "Grass coverage factor:" << gc << std::endl;
			std::cout << (gui_i == 5 ? "->" : "  ") << "Frequency: " << f << std::endl;
			std::cout << (gui_i == 6 ? "->" : "  ") << "Tessellation Multiplier: " << tm << std::endl;
			std::cout << (gui_i == 7 ? "->" : "  ") << "Delta magnitude: " << dm << std::endl;
			updateShell = false;
		}

		if (snow) {
			tc.snowy(lightColor);
			snow = false;
		}

		t1 = glfwGetTime();

		glm::vec3 lightPosition;
		lightPosition = glm::vec3(-.7, .5, .75)*1e9f;
		lightPosition += camera.Position;
		// input
		window.processInput(frameTime);

		//update tiles position to make the world infinite
		tc.updateTiles(); 

		SceneFBO.bind();
		// render
		//glEnable(GL_MULTISAMPLE);
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
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 50.f,10000000.0f);

		// draw terrain
		tc.drawTiles(proj, lightPosition, lightColor, fogColor, SceneFBO);

		//disable test for quad rendering
		ScreenQuad::disableTests();

		// scene post processing - blending between main scene texture and clouds texture

		volumetricClouds.draw(view, proj, lightPosition, SceneFBO.depthTex);

		unbindCurrentFrameBuffer();
		Shader& post = PostProcessing.getShader();
		post.use();

		post.setSampler2D("screenTexture", SceneFBO.tex, 0);
		post.setSampler2D("cloudTEX", volumetricClouds.getCloudsTexture(), 1);

		ScreenQuad::drawQuad();

		//GUI
		glEnable(GL_DEPTH);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//if (gui) gui->draw();
		glDisable(GL_DEPTH);

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
