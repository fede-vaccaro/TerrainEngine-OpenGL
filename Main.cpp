#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "TessShader.h"
#include "texture.h"
#include "TileController.h"
#include "Tile.h"

#include <camera.h>
#include <stb_image.h>
#include <model.h>

#include "TextArea.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// screen settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

float dispFactor = 5.0;

glm::vec3 startPosition(0.0f, dispFactor / 2.0, 0.0f);

bool keyBools[10] = { false, false,false, false, false, false, false, false, false, false };
bool updateShell = true;

// camera
Camera camera(startPosition);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float t1 = 0.0, t2 = 0.0, frameTime = 0.0; // time variables
bool wireframe = false;
glm::mat4 identityMatrix;

std::vector<std::function<float()>> getters;
std::vector<std::function< void(float)> > setters;


int main()
{

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif


	// glfw window creation										 
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tessellation Shader on terrain", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader skyboxShader("shaders/skyboxVert.vert", "shaders/skyboxFrag.frag");
	Shader waterShader("shaders/waterVertexShader.vert", "shaders/waterFragmentShader.frag");
	TessellationShader tshader("shaders/tessVertexShader.vert", "shaders/tessControlShader.tcs", "shaders/tessEvaluationShader.tes", "shaders/tessFragmentShader.frag");

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// skyboxVAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// set light matrices
	glm::mat4 lightModel, idMat;
	glm::vec3 lightPosition = glm::vec3(-5.0f, 10.0f, -5.0f);
	lightModel = glm::translate(lightModel, lightPosition);
	lightModel = glm::scale(lightModel, glm::vec3(0.5, 0.5, 0.5));

	//loading models
	Model waterPlane("resources/plane.obj", GL_TRIANGLES);
	Model plane_("resources/plane.obj", GL_PATCHES);

	vector<std::string> faces =
	{
		"resources/skybox/left.jpg",
		"resources/skybox/right.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	glm::vec3 fogColor(204, 224, 255);
	fogColor /= 255.0;
	glm::vec3 lightColor(255, 255, 190);
	lightColor /= 255.0;

	float scale = 10.0f;

	TileController tc(scale, dispFactor, &camera, &tshader, &waterShader);

	float vertices[] = {
		-1.0f, 0.0f, -1.0f, 0.0, 1.0, 0.0, 0.0, 0.0,
		-1.0f, 0.0f,  1.0f,  0.0, 1.0, 0.0, 1.0, 1.0,
		1.0f, 0.0f, -1.0f,  0.0, 1.0, 0.0, 0.0, 1.0,
		1.0f, 0.0f, -1.0f, 0.0, 1.0, 0.0, 0.0, 0.0,
		-1.0f, 0.0f,  1.0f, 0.0, 1.0, 0.0, 1.0, 0.0,
		1.0f, 0.0f,  1.0f, 0.0, 1.0, 0.0, 1.0, 1.0
	};

	TextArea::setWindow(window);
	screen scr;
	scr.WIDTH = SCR_WIDTH;
	scr.HEIGHT = SCR_HEIGHT;
	TextArea::setScreen(&scr);



	gui = new TextArea(10, 10, 250, 0); // l'altezza è adattata al contenuto
	int octaves = tc.getOctaves();
	float df = tc.getDispFactor();
	float wh = tc.getWaterHeight();
	float gc = tc.getGrassCoverage();
	float f = tc.getFreq();
	float tm = tc.getTessMultiplier();

	if (gui) gui->addElement(std::string("Octaves: "), &octaves);
	if (gui) gui->addElement(std::string("Terrain Height: "), &df);
	if (gui) gui->addElement(std::string("Water height: "), &wh);
	if (gui) gui->addElement(std::string("Grass coverage factor:"), &gc);
	if (gui) gui->addElement(std::string("Frequency: "), &f);
	if (gui) gui->addElement(std::string("Tessellation Multiplier: "), &tm);
	if (gui) gui->addElement(std::string("Delta magnitude: "), &deltaMagnitude);



	getters.push_back(0);
	getters.push_back([&tc] { return tc.getOctaves(); });
	getters.push_back([&tc] { return tc.getDispFactor(); });
	getters.push_back([&tc] { return tc.getWaterHeight(); });
	getters.push_back([&tc] { return tc.getGrassCoverage(); });
	getters.push_back([&tc] { return tc.getFreq(); });
	getters.push_back([&tc] { return tc.getTessMultiplier(); });
	getters.push_back(0);

	setters.push_back(0);
	setters.push_back( [&tc](float value) { tc.setOctaves(value);       });
	setters.push_back( [&tc](float value) { tc.setDispFactor(value);    });
	setters.push_back( [&tc](float value) { tc.setWaterHeight(value);   });
	setters.push_back( [&tc](float value) { tc.setGrassCoverage(value); });
	setters.push_back( [&tc](float value) { tc.setFreq(value);          });
	setters.push_back([&tc](float value) { tc.setTessMultiplier(value);  });
	setters.push_back(0);

	gui_el = ( setters.size() == getters.size() ? setters.size() : -1);


	while (!glfwWindowShouldClose(window))
	{

		octaves = getters[1]();
		df = getters[2]();
		wh = getters[3]();
		gc = getters[4]();
		f = getters[5]();
		tm = getters[6]();
		float dm = deltaMagnitude;

		if (updateShell) {
			clear();
			std::cout << (gui_i == 1 ? "->" : "  ") << "Octaves: " << octaves << std::endl;
			std::cout << (gui_i == 2 ? "->" : "  ") << "Terrain Height : " << df << std::endl;
			std::cout << (gui_i == 3 ? "->" : "  ") << "Water height: " << wh << std::endl;
			std::cout << (gui_i == 4 ? "->" : "  ") << "Grass coverage factor:" << gc << std::endl;
			std::cout << (gui_i == 5 ? "->" : "  ") << "Frequency: " << f << std::endl;
			std::cout << (gui_i == 6 ? "->" : "  ") << "Tessellation Multiplier: " << tm << std::endl;
			std::cout << (gui_i == 7 ? "->" : "  ") << "Delta magnitude: " << dm << std::endl;
			updateShell = false;
		}
		t1 = glfwGetTime();
		float degreePerSecond = 10.0f;
		float dist = 300.0;

		float x = -73, z = -223;
		float angle = glm::atan(x / z);

		//std::cout << camera.Position.x << " " << camera.Position.z << std::endl;

		//x = cos(-angle)*dist;
		//z = sin(-angle)*dist;
		lightPosition = glm::vec3(x*2.0, dispFactor, z*2.0); //rotate light
		lightPosition += camera.Position;
		// input
		processInput(window);

		tc.updateTiles();

		// render
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_MULTISAMPLE);

		glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// toggle/untoggle wireframe mode
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			Tile::drawFog = false;
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			Tile::drawFog = true;
		}

		// Camera (View Matrix) setting
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f);


		// set terrain matrices
		glm::mat4 gWorld, gVP;
		gWorld = glm::scale(gWorld, glm::vec3(10.0, 0.0, 10.0));
		gVP = proj * view;

		Water * const waterPtr = tc.getWaterPtr();
		if (waterPtr) {
			camera.invertPitch();
			//glm::vec3 cameraReflPos(camera.Position.x, reflectionCameraY, camera.Position.z);
			camera.Position.y -= 2 * (camera.Position.y - tc.getWaterHeight());
			glm::mat4 reflectionView = camera.GetViewMatrix();
			glm::mat4 reflgVP = proj * reflectionView;

			waterPtr->bindReflectionFBO();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//draw skyBox
			glDepthFunc(GL_LEQUAL);
			skyboxShader.use();
			glm::mat4 view2 = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation part from the view matrix
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(0.0, -0.1, 0.0));
			skyboxShader.setMat4("model", model);
			skyboxShader.setMat4("view", view2);
			skyboxShader.setMat4("projection", proj);
			glBindVertexArray(skyboxVAO);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);

			// reset camera position
			camera.invertPitch();
			camera.Position.y += 2 * abs(camera.Position.y - tc.getWaterHeight());

			//unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);

			waterPtr->unbindFBO();
		}



		float wps = 0.5;
		//tc.setWaterHeight(wps*glfwGetTime());
	
		tc.drawTiles(proj, lightPosition, lightColor, fogColor);

		//draw skyBox
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		glm::mat4 view2 = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		glm::mat4 model = glm::translate(identityMatrix, glm::vec3(0.0, -0.1, 0.0));
		skyboxShader.setMat4("model", model);
		skyboxShader.setMat4("view", view2);
		skyboxShader.setMat4("projection", proj);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		//unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
		glDisable(GL_CLIP_DISTANCE0);

		glEnable(GL_DEPTH);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (gui) gui->draw();
		glDisable(GL_DEPTH);
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();

		t2 = glfwGetTime();
		frameTime = t2 - t1;
		float timeToSleep = 1000.0f / MAX_FPS - (t2 - t1)*1000.0f;
		if (timeToSleep > 0.0f) {
			Sleep(timeToSleep);
		}
		t2 = glfwGetTime();
		frameTime = t2 - t1;
		//std::cout << 1.0f / frameTime << " FPS" << std::endl;

	}

	// de-allocate resources
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, frameTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, frameTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, frameTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, frameTime);

	// WIREFRAME
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		if (keyBools[4] == false) {
			//std::cout << "WIREFRAME" << std::endl;
			wireframe = !wireframe;
			keyBools[4] = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
		if (keyBools[4] == true) { keyBools[4] = false; } // Non aggiungere niente qui
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		if (keyBools[7] == false) {
			//std::cout << "GUI VISIBILITY" << std::endl;
			if (gui) gui->setVisible(!gui->getVisible());
			keyBools[7] = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) {
		if (keyBools[7] == true) { keyBools[7] = false; } // Non aggiungere niente qui
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		if (keyBools[5] == false) {
			//std::cout << "Decrease delta magnitude" << std::endl;
			deltaMagnitude /= 10.0;
			keyBools[5] = true;

		}
	}
	else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
		if (keyBools[5] == true) { keyBools[5] = false;
		updateShell = true; } // Non aggiungere niente qui
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (keyBools[6] == false) {
			//std::cout << "Increase magnitude" << std::endl;
			deltaMagnitude *= 10.0;
			keyBools[6] = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
		if (keyBools[6] == true) { keyBools[6] = false; updateShell = true;
		} // Non aggiungere niente qui
	}

	// DOWN KEY
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (keyBools[0] == false) {
			//std::cout << "DOWN ARROW PRESSED" << std::endl;
			if(gui) gui->bind(1);
			keyBools[0] = true;
			gui_i = (gui_i + 1) % gui_el;
			updateShell = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
		if (keyBools[0] == true) {
			//std::cout << "DOWN ARROW RELEASED" << std::endl;
			keyBools[0] = false;
		}
	}
	// UP ARROW KEY
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (keyBools[1] == false) {
			//std::cout << "UP ARROW PRESSED" << std::endl;
			if (gui) gui->bind(-1);
			keyBools[1] = true;
			gui_i = (gui_i - 1) % gui_el;
			updateShell = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
		if (keyBools[1] == true) {
			//std::cout << "UP ARROW RELEASED" << std::endl;
			keyBools[1] = false;
		}
	}
	// LEFT ARROW KEY
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (keyBools[2] == false) {
			//std::cout << "LEFT ARROW PRESSED" << std::endl;
			keyBools[2] = true;
			float value = getters[gui_i] ? getters[gui_i]() : 0.0;
			if (setters[gui_i] != 0) setters[gui_i](value - deltaMagnitude);
			updateShell = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
		if (keyBools[2] == true) {
			//std::cout << "LEFT ARROW RELEASED" << std::endl;
			keyBools[2] = false;
		}
	}
	// RIGHT ARROW KEY
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (keyBools[3] == false) {
			//std::cout << "RIGHT ARROW PRESSED" << std::endl;
			keyBools[3] = true;
			std::cout << gui_i << std::endl;
			if (setters[gui_i] != 0) { setters[gui_i](getters[gui_i]() + deltaMagnitude); }
			updateShell = true;

		}
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
		if (keyBools[3] == true) {
			//std::cout << "RIGHT ARROW RELEASED" << std::endl;
			keyBools[3] = false;
		}
	}
}


// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}