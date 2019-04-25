#pragma once
#include "shader.h"
#include <glad/glad.h>
#include "../DrawableObjects/drawableObject.h"

// refactors those shaders which write only into the screen space (e.g. post processing, ShaderToy.com), so only the fragment shader it's needed and the model is only two triangles
class ScreenSpaceShader : drawableObject
{
public:
	ScreenSpaceShader(const char * fragmentPath);
	~ScreenSpaceShader();

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

