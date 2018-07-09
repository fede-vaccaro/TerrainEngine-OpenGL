#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shaderUtils.h>
#include "TessShader.h"
#include "texture.h"
//#include "buffers.h"
//#include "Object.h"
#include "TileController.h"

#include <camera.h>
#include <stb_image.h>
#include <model.h>

#include "perlin/HeightMap.h"
#include "TerrainGenerator.h"
#include "Tile.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <map>
#include <stdlib.h>
#include <iostream>
#include <stdlib.h> 


// HEIGHT MAP & TERRAIN
float HeightRange = 60;
bool generateTerrain = true;

const int MAX_FPS = 144,
HEIGHT_SCENE = 200,
WIDTH_SCENE = 200;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// screen settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

float dispFactor = 40.0;

glm::vec3 startPosition(0.0f, dispFactor / 2.0, 0.0f);

// camera
Camera camera(startPosition);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float t1 = 0.0, t2 = 0.0, frameTime = 0.0; // time variables
bool wireframe = false;
glm::mat4 identityMatrix;

int main()
{

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	Shader texViewShader("shaders/waterVertexShader.vert", "shaders/textureFragmentShader.frag");
	Shader skyboxShader("shaders/skyboxVert.vert", "shaders/skyboxFrag.frag");
	Shader waterShader("shaders/waterVertexShader.vert", "shaders/waterFragmentShader.frag");
	Shader lightShader("shaders/lightVertexShader.vert", "lightFragmentShader.frag");
	TessellationShader tshader("shaders/tessVertexShader.vert", "shaders/tessControlShader.tcs", "shaders/tessEvaluationShader.tes", "shaders/tessFragmentShader.frag");
	TessellationShader tshader2("shaders/tessSmoothingVertexShader.vert", "shaders/tessSmoothingControlShader.tcs", "shaders/tessSmoothingEvaluationShader.tes", "shaders/tessSmoothingFragmentShader2.frag");
	TessellationShader lightShader2("shaders/tessSmoothingVertexShader.vert", "shaders/tessSmoothingControlShader.tcs", "shaders/tessSmoothingEvaluationShader.tes", "shaders/lightFragmentShader.frag");

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
	glm::vec3 lightColor(255, 255, 153);
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

	while (!glfwWindowShouldClose(window))
	{

		float xSpeed = 0.025;
		float xOffset = xSpeed * glfwGetTime();

		t1 = glfwGetTime();
		float degreePerSecond = 10.0f;
		float dist = 300.0;

		float x = -20.9, z = -78.6;
		float angle = glm::atan(x / z);

		x = cos(-angle)*dist;
		z = sin(-angle)*dist;
		lightPosition = glm::vec3(x, dispFactor, z); //ruotate light

		// input
		processInput(window);

		// render
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

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
		glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


		// set terrain matrices
		glm::mat4 gWorld, gVP;
		gWorld = glm::scale(gWorld, glm::vec3(10.0, 0.0, 10.0));
		gVP = proj * view;

		Water * waterPtr = tc.tiles[0]->water;
		if (waterPtr) {
			camera.invertPitch();
			//glm::vec3 cameraReflPos(camera.Position.x, reflectionCameraY, camera.Position.z);
			camera.Position.y -= 2 * (camera.Position.y - tc.waterHeight);
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

			// draw light source
			lightShader2.use();
			glm::mat4 rot = glm::rotate(identityMatrix, glm::radians(-t1 * degreePerSecond), glm::vec3(0.0, 1.0, 0.0));
			lightModel = glm::translate(idMat, lightPosition) * rot; //update light model
			lightShader2.setMat4("gWorld", lightModel);
			lightShader2.setMat4("gVP", reflgVP);
			lightShader2.setFloat("gTessellationLevel", 5.0f);
			lightShader2.setVec3("u_LightColor", lightColor);

			// reset camera position
			camera.invertPitch();
			camera.Position.y += 2 * abs(camera.Position.y - tc.waterHeight);

			//unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);

			waterPtr->unbindFBO();
		}


		tc.updateTiles();
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
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		//unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
		glDisable(GL_CLIP_DISTANCE0);





		// draw light source
		lightShader2.use();
		glm::mat4 rot = glm::rotate(identityMatrix, glm::radians(-t1 * degreePerSecond), glm::vec3(0.0, 1.0, 0.0));
		lightModel = glm::translate(idMat, lightPosition) * rot; //update light model
		lightShader2.setMat4("gWorld", lightModel);
		lightShader2.setMat4("gVP", gVP);
		lightShader2.setFloat("gTessellationLevel", 5.0f);
		lightShader2.setVec3("u_LightColor", lightColor);
		//sphere.Draw(lightShader2);

		//draw skyBox
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view2 = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		model = glm::translate(identityMatrix, glm::vec3(0.0, -0.1, 0.0));
		skyboxShader.setMat4("model", model);
		skyboxShader.setMat4("view", view2);
		skyboxShader.setMat4("projection", proj);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		// draw texture planes
		texViewShader.use();

		glm::mat4 scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.10, 0.10, 0.10));
		glm::mat4 waterMatrix = glm::translate(identityMatrix, glm::vec3(-0.50, 0.50, -1.0));
		glm::mat4 rotation = glm::rotate(identityMatrix, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
		waterMatrix = waterMatrix * rotation * scaleMatrix;

		texViewShader.setMat4("mvpMatrix", waterMatrix);
		texViewShader.setMat4("modelMatrix", waterMatrix);
		texViewShader.setMat4("gVP", identityMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tc.tiles[0]->water->reflectionTex);
		texViewShader.setInt("texBuff", 0);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
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
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		wireframe = !wireframe;
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