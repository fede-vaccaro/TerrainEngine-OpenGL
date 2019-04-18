#include "ScreenQuad.h"

unsigned int ScreenQuad::quadVAO = 0;
unsigned int ScreenQuad::quadVBO = 0;
bool ScreenQuad::initialized = false;

ScreenQuad::ScreenQuad(const char * fragmentPath)
{
	initializeQuad();
	//shad = new Shader("shaders/screen.vert", fragmentPath);
	shad = new Shader("ScreenQuad_" + getShaderName(fragmentPath));

	shad->attachShader(BaseShader("shaders/screen.vert"));
	shad->attachShader(BaseShader(fragmentPath));
	shad->linkPrograms();
}

void ScreenQuad::drawQuad() {
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ScreenQuad::draw() {
	ScreenQuad::drawQuad();
}

ScreenQuad::~ScreenQuad()
{
}

void ScreenQuad::initializeQuad() {
	if (!initialized) {
		float vertices[] = {
			-1.0f, -1.0f, 0.0, 0.0,
			1.0f, -1.0f, 1.0, 0.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f,  1.0f, 1.0, 1.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f, -1.0f, 1.0, 0.0
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		ScreenQuad::initialized = true;
	}

}
