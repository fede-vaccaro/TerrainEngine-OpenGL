#pragma once
#include "shader.h"
#include <glad/glad.h>
#include "../drawableObject.h"

class ScreenQuad : drawableObject
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

	virtual void draw();
	static void drawQuad();

	static void disableTests() {
		//glDisable(GL_CLIP_DISTANCE0);
		glDisable(GL_DEPTH_TEST);
	}

	static void enableTests() {
		glEnable(GL_DEPTH_TEST);
	}



private:
	Shader * shad;
	static unsigned int quadVAO, quadVBO;
	static bool initialized;

	void initializeQuad();
};

