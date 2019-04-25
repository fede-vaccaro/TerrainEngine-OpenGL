#include "Window.h"

unsigned int Window::SCR_WIDTH = 1600;
unsigned int Window::SCR_HEIGHT = 900;

Camera * Window::camera = 0;
bool Window::keyBools[10] = { false, false,false, false, false, false, false, false, false, false };
bool Window::wireframe = false;
bool Window::firstMouse = true;
float Window::lastX = SCR_WIDTH / 2.0f;
float Window::lastY = SCR_HEIGHT / 2.0f;

bool Window::mouseCursorDisabled = true;

Window::Window(int& success, unsigned int scrW, unsigned int scrH, std::string name) : name(name)
{

	Window::SCR_WIDTH = scrW;
	Window::SCR_HEIGHT = scrH;
	success = 1;
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

	// glfw window creation										 
	this->w = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, name.c_str(), NULL, NULL);
	if (!this->w)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		success = 0;
		return;
	}
	glfwMakeContextCurrent(this->w);
	glfwSetFramebufferSizeCallback(this->w, &Window::framebuffer_size_callback);
	glfwSetCursorPosCallback(this->w, &Window::mouse_callback);
	glfwSetScrollCallback(this->w, &Window::scroll_callback);

	Window::camera = 0;
	oldState = newState = GLFW_RELEASE;
	
	/*
	for (int i = 0; i < 10; i++) {
		Window::keyBools[i] = false;
	}
	Window::wireframe = false;
	Window::firstMouse = true;
	Window::lastX = SCR_WIDTH / 2.0f;
	Window::lastY = SCR_HEIGHT / 2.0f;
	*/

	success = gladLoader() && success;
	if (success) {
		std::cout << "GLFW window correctly initialized!" << std::endl;
	}
}

int Window::gladLoader() {

	// tell GLFW to capture our mouse
	glfwSetInputMode(this->w, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 0;
	}

	return 1;
}

// glfw: whenever the mouse moves, this callback is called
void Window::mouse_callback(GLFWwindow* window, double xpos, double ypos)
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
	if(!mouseCursorDisabled)
		Window::camera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Window::camera->ProcessMouseScroll(yoffset);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
 
void Window::processInput(float frameTime) {
	if (glfwGetKey(this->w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(this->w, true);

	if (glfwGetKey(this->w, GLFW_KEY_W) == GLFW_PRESS)
		camera->ProcessKeyboard(FORWARD, frameTime);
	if (glfwGetKey(this->w, GLFW_KEY_S) == GLFW_PRESS)
		camera->ProcessKeyboard(BACKWARD, frameTime);
	if (glfwGetKey(this->w, GLFW_KEY_A) == GLFW_PRESS)
		camera->ProcessKeyboard(LEFT, frameTime);
	if (glfwGetKey(this->w, GLFW_KEY_D) == GLFW_PRESS)
		camera->ProcessKeyboard(RIGHT, frameTime);
	
	newState = glfwGetMouseButton(this->w, GLFW_MOUSE_BUTTON_RIGHT);

	if (newState == GLFW_RELEASE && oldState == GLFW_PRESS) {
		glfwSetInputMode(this->w, GLFW_CURSOR, (mouseCursorDisabled
		? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL));
		mouseCursorDisabled = !mouseCursorDisabled;
		if (mouseCursorDisabled)
			firstMouse = true;
		//std::cout << "MOUSE R PRESSED!" << std::endl;
	}

	oldState = newState;
	
	// WIREFRAME
	if (glfwGetKey(this->w, GLFW_KEY_T) == GLFW_PRESS) {
		if (keyBools[4] == false) {
			//std::cout << "WIREFRAME" << std::endl;
			wireframe = !wireframe;
			keyBools[4] = true;
		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_T) == GLFW_RELEASE) {
		if (keyBools[4] == true) { keyBools[4] = false; } // Non aggiungere niente qui
	}
}

Window::~Window()
{
	this->terminate();
}
