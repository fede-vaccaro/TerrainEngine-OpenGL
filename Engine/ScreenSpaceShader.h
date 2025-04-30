#pragma once
#include <glad/glad.h>

#include <string>

#include <ShadingProgram.h>

#include "../DrawableObjects/drawableObject.h"

// refactors those shaders which write only into the screen space (e.g. post processing, ShaderToy.com), so only the fragment shader it's needed and the model is only two triangles
namespace terrain{

class ScreenSpaceShader : drawableObject
{
public:
	ScreenSpaceShader(const std::string& fragmentPath);

	virtual void draw();
	static void drawQuad();

	const auto * getShaderPtr() {
		return &shad;
	}

	auto& getShader() {
		return shad;
	}

	static void disableTests() {
		//glDisable(GL_CLIP_DISTANCE0);
		glDisable(GL_DEPTH_TEST);
	}

	static void enableTests() {
		glEnable(GL_DEPTH_TEST);
	}

	~ScreenSpaceShader() = default;

private:
	gl::ShadingProgram shad;
	static unsigned int quadVAO, quadVBO;
	static bool initialized;

	void initializeQuad();
};

} // namespace terrain
