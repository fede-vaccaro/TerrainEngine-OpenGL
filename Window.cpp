#include "Window.h"

Camera * Window::camera = 0;
bool Window::keyBools[10] = { false, false,false, false, false, false, false, false, false, false };
bool Window::wireframe = false;
bool Window::firstMouse = true;
float Window::lastX = SCR_WIDTH / 2.0f;
float Window::lastY = SCR_HEIGHT / 2.0f;


Window::Window(int& success, Camera * cam, std::string name) : name(name)
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

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

	Window::camera = cam;
	/*
	for (int i = 0; i < 10; i++) {
		Window::keyBools[i] = false;
	}
	Window::wireframe = false;
	Window::firstMouse = true;
	Window::lastX = SCR_WIDTH / 2.0f;
	Window::lastY = SCR_HEIGHT / 2.0f;
	*/
	success = 1;
}

int Window::inMain() {

	// tell GLFW to capture our mouse
	glfwSetInputMode(this->w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
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
	/*
	if (glfwGetKey(this->w, GLFW_KEY_Y) == GLFW_PRESS) {
		if (keyBools[8] == false) {
			//std::cout << "SNOW MODE" << std::endl;
			snow = true;
			keyBools[8] = true;
		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_Y) == GLFW_RELEASE) {
		if (keyBools[8] == true) { keyBools[8] = false; } // Non aggiungere niente qui
	}
	if (glfwGetKey(this->w, GLFW_KEY_I) == GLFW_PRESS) {
		if (keyBools[7] == false) {
			//std::cout << "GUI VISIBILITY" << std::endl;
			if (gui) gui->setVisible(!gui->getVisible());
			keyBools[7] = true;
		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_I) == GLFW_RELEASE) {
		if (keyBools[7] == true) { keyBools[7] = false; } // Non aggiungere niente qui
	}
	if (glfwGetKey(this->w, GLFW_KEY_O) == GLFW_PRESS) {
		if (keyBools[5] == false) {
			//std::cout << "Decrease delta magnitude" << std::endl;
			deltaMagnitude /= 10.0;
			keyBools[5] = true;

		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_O) == GLFW_RELEASE) {
		if (keyBools[5] == true) {
			keyBools[5] = false;
			updateShell = true;
		} // Non aggiungere niente qui
	}
	if (glfwGetKey(this->w, GLFW_KEY_P) == GLFW_PRESS) {
		if (keyBools[6] == false) {
			//std::cout << "Increase magnitude" << std::endl;
			deltaMagnitude *= 10.0;
			keyBools[6] = true;
		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_P) == GLFW_RELEASE) {
		if (keyBools[6] == true) {
			keyBools[6] = false; updateShell = true;
		} // Non aggiungere niente qui
	}

	// DOWN KEY
	if (glfwGetKey(this->w, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (keyBools[0] == false) {
			//std::cout << "DOWN ARROW PRESSED" << std::endl;
			if (gui) gui->bind(1);
			keyBools[0] = true;
			gui_i = (gui_i + 1) % gui_el;
			updateShell = true;
		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_DOWN) == GLFW_RELEASE) {
		if (keyBools[0] == true) {
			//std::cout << "DOWN ARROW RELEASED" << std::endl;
			keyBools[0] = false;
		}
	}
	// UP ARROW KEY
	if (glfwGetKey(this->w, GLFW_KEY_UP) == GLFW_PRESS) {
		if (keyBools[1] == false) {
			//std::cout << "UP ARROW PRESSED" << std::endl;
			if (gui) gui->bind(-1);
			keyBools[1] = true;
			gui_i = (gui_i - 1) % gui_el;
			updateShell = true;
		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_UP) == GLFW_RELEASE) {
		if (keyBools[1] == true) {
			//std::cout << "UP ARROW RELEASED" << std::endl;
			keyBools[1] = false;
		}
	}
	// LEFT ARROW KEY
	if (glfwGetKey(this->w, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (keyBools[2] == false) {
			//std::cout << "LEFT ARROW PRESSED" << std::endl;
			keyBools[2] = true;
			float value = getters[gui_i] ? getters[gui_i]() : 0.0;
			if (setters[gui_i] != 0) setters[gui_i](value - deltaMagnitude);
			updateShell = true;
		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_LEFT) == GLFW_RELEASE) {
		if (keyBools[2] == true) {
			//std::cout << "LEFT ARROW RELEASED" << std::endl;
			keyBools[2] = false;
		}
	}
	// RIGHT ARROW KEY
	if (glfwGetKey(this->w, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (keyBools[3] == false) {
			//std::cout << "RIGHT ARROW PRESSED" << std::endl;
			keyBools[3] = true;
			std::cout << gui_i << std::endl;
			if (setters[gui_i] != 0) { setters[gui_i](getters[gui_i]() + deltaMagnitude); }
			updateShell = true;

		}
	}
	else if (glfwGetKey(this->w, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
		if (keyBools[3] == true) {
			//std::cout << "RIGHT ARROW RELEASED" << std::endl;
			keyBools[3] = false;
		}
	}
	*/
}


Window::~Window()
{
}
