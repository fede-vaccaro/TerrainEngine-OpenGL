#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "computeShader.h"
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// screen settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float dispFactor = 6.0;

glm::vec3 startPosition(0.0f, 0, 0.0f);

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
	std::cout << "============= CREATING TSHADER ==============" << std::endl;
	TessellationShader tshader("shaders/tessVertexShader.vert", "shaders/tessControlShader.tcs", "shaders/tessEvaluationShader.tes", "shaders/tessFragmentShader.frag");
	std::cout << "============= TSHADER CREATED ==============" << std::endl;

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
		-1.0f, -1.0f, 0.0, 0.0,
		-1.0f,  1.0f, 0.0, 1.0,
		 1.0f, -1.0f, 1.0, 0.0,
		 1.0f, -1.0f, 1.0, 0.0,
		-1.0f,  1.0f, 0.0, 1.0,
		 1.0f,  1.0f, 1.0, 1.0
	};

	float vertices1[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f, 1.0f, 0.0f,
		1.0f,  1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f
	};
	std::cout << "============ OPENING SCREEN SHADER ============" << std::endl;
	Shader screenShader("shaders/screen.vert", "shaders/raymarch_cube.frag");

	std::cout << "============ OPENING POST PROCESSING SHADER ============" << std::endl;
	Shader post("shaders/screen.vert", "shaders/post_processing.frag");


	// screenVAO
	unsigned int screenVAO, screenVBO;
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2* sizeof(float)));
	glEnableVertexAttribArray(1);

	//post processing rbo
	unsigned int fbo = createFrameBuffer();
	//unsigned int texDepth = createDepthBufferAttachment(SCR_WIDTH, SCR_HEIGHT);
	unsigned int rbo = createRenderBufferAttachment(SCR_WIDTH, SCR_HEIGHT);
	unsigned int texFBO = createTextureAttachment(SCR_WIDTH, SCR_HEIGHT);
	unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);


	//compute shaders
	ComputeShader comp("shaders/perlinworley.comp");

	//make texture
	unsigned int perlinTex = Texture3D(128, 128, 128);

	//compute
	comp.use();
	comp.setVec3("u_resolution", glm::vec3(128, 128, 128));
	std::cout << "computing perlinworley!" << std::endl;
	glDispatchCompute((GLuint)128, (GLuint)128, (GLuint)128);
	std::cout << "computed!!" << std::endl;



	//compute shaders
	ComputeShader worley_git("shaders/worley.comp");

	//make texture
	unsigned int worley32 = Texture3D(32, 32, 32);

	//compute
	comp.use();
	comp.setVec3("u_resolution", glm::vec3(32, 32, 32));
	std::cout << "computing worley 32!" << std::endl;
	glDispatchCompute((GLuint)32, (GLuint)32, (GLuint)32);
	std::cout << "computed!!" << std::endl;

	////////////////////////
	//compute shaders
	ComputeShader weather("shaders/weather.comp");

	//make texture
	unsigned int weatherTex = Texture2D(1024, 1024);

	//compute
	weather.use();
	std::cout << "computing weather!" << std::endl;
	glDispatchCompute((GLuint)1024, (GLuint)1024, (GLuint)1);
	std::cout << "weather computed!!" << std::endl;

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	camera.Position = glm::vec3(3000.0, 3000.0, 3000.0);


	int frame = 0;
	while (!glfwWindowShouldClose(window))
	{


		processInput(window);

		//camera.Position += glm::vec3(10, 10, 10);

		glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		float aspect = SCR_WIDTH / SCR_HEIGHT;
		
		glm::mat4 proj = glm::perspective(20.0f, aspect, 0.1f, 100.0f);

		// fbo
		bindFrameBuffer(fbo, SCR_WIDTH, SCR_HEIGHT);
		screenShader.use();
		screenShader.setVec2("iResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
		screenShader.setFloat("iTime", glfwGetTime());
		screenShader.setMat4("proj", proj);
		screenShader.setMat4("view", camera.GetViewMatrix());
		screenShader.setVec3("cameraPosition", camera.Position);
		screenShader.setInt("frame", ++frame);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, perlinTex);
		screenShader.setInt("cloud", 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, worley32);
		screenShader.setInt("worley32", 2);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, weatherTex);
		screenShader.setInt("weatherTex", 1);

		glBindVertexArray(screenVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);


		
		// draw to screen
		post.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texFBO);
		post.setInt("screenTexture", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, perlinTex);
		post.setInt("volTex", 1);

		post.setFloat("time", glfwGetTime());

		glBindVertexArray(screenVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);



		// toggle/untoggle wireframe mode
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			Tile::drawFog = false;
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			Tile::drawFog = true;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

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
			std::cout << "WIREFRAME" << std::endl;
			wireframe = !wireframe;
			keyBools[4] = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
		if (keyBools[4] == true) { keyBools[4] = false; } // Non aggiungere niente qui
	}	
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		if (keyBools[8] == false) {
			//std::cout << "SNOW MODE" << std::endl;
			snow = true;
			keyBools[8] = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE) {
		if (keyBools[8] == true) { keyBools[8] = false; } // Non aggiungere niente qui
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