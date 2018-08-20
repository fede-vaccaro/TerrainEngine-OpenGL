#pragma once
#include "shader.h"
#include <glad/glad.h>

class ScreenQuad
{
public:
	ScreenQuad(const char * fragmentPath);
	~ScreenQuad();

	Shader * const getShaderPtr() {
		return shad;
	}

	Shader & const getShader() {
		return *shad;
	}

	static void drawQuad();

	static void disableTests() {
		glDisable(GL_CLIP_DISTANCE0);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}



private:
	Shader * shad;
	static unsigned int quadVAO, quadVBO;
	static bool initialized;

	void initializeQuad();
};

