#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <camera.h>
#include <iostream>



class Window
{
public:
	//initializer
	Window(int& success, unsigned int SCR_WIDTH = 1600, unsigned int SCR_HEIGHT = 900, std::string name = "TerrainEngine OpenGL");
	~Window();
	GLFWwindow * w;
	GLFWwindow * getWindow() const { return w; }

	void processInput(float frameTime); //input handler

	// screen settings
	static unsigned int SCR_WIDTH;
	static unsigned int SCR_HEIGHT;

	void terminate() {
		glfwTerminate();
	}

	bool isWireframeActive() {
		return Window::wireframe;
	}

	// return if the main loop must continue
	bool continueLoop() {
		return !glfwWindowShouldClose(this->w);
	}

	//put this at the end of the main
	void swapBuffersAndPollEvents() {
		glfwSwapBuffers(this->w);
		glfwPollEvents();
	}

	static Camera * camera;

private:
	int oldState, newState;
	int gladLoader(); // set mouse input and load opengl functions 

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	// avoid infinite key press
	static bool keyBools[10];

	static bool mouseCursorDisabled;

	// wireframe mode
	static bool wireframe;

	//avoid to make the mouse to jump at the start of the program
	static bool firstMouse;// = true;
	static float lastX;
	static float lastY;

	std::string name;
};


