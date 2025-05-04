#include "ScreenSpaceShader.h"

namespace terrain
{

unsigned int ScreenSpaceShader::quadVAO = 0;
unsigned int ScreenSpaceShader::quadVBO = 0;
bool ScreenSpaceShader::initialized = false;

ScreenSpaceShader::ScreenSpaceShader(const std::string& fragmentPath)
{
	initializeQuad();

	auto shaderBuilder = gl::ProgramBuilder();

	gl::Shader fragmentShader = gl::Shader::loadFrom(fragmentPath).value();

	shad = shaderBuilder
	.newProgram("ScreenQuad_" + fragmentShader.getName())
	.attachShader(gl::Shader::loadFrom("shaders/screen.vert").value())
	.attachShader(std::move(fragmentShader))
	.linkPrograms().value();
}

void ScreenSpaceShader::drawQuad() {
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ScreenSpaceShader::draw() {
	ScreenSpaceShader::drawQuad();
}

void ScreenSpaceShader::initializeQuad() {
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
		ScreenSpaceShader::initialized = true;
	}

}

} // namespace terrain