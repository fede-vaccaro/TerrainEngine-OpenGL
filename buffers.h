#pragma once
#include <glad/glad.h>
#include "Window.h"

void bindFrameBuffer(int frameBuffer, int width, int height);

void unbindCurrentFrameBuffer(int scrWidth, int scrHeight);
void unbindCurrentFrameBuffer();

unsigned int createFrameBuffer();

unsigned int createTextureAttachment(int width, int height);

unsigned int * createColorAttachments(int width, int height, unsigned int nColorAttachments);

unsigned int createDepthTextureAttachment(int width, int height);

unsigned int createDepthBufferAttachment(int width, int height);

unsigned int createRenderBufferAttachment(int width, int height);

void initializePlaneVAO(const int res, const int width, GLuint * planeVAO, GLuint * planeVBO, GLuint * planeEBO);


class FrameBufferObject {
public:
	FrameBufferObject(int W, int H);
	FrameBufferObject(int W, int H, int numColorAttachments);
	unsigned int FBO, renderBuffer, depthTex;
	unsigned int tex;
	unsigned int getColorAttachmentTex(int i) {
		if (i < 0 || i > nColorAttachments) {
			std::cout << "COLOR ATTACHMENT OUT OF RANGE" << std::endl;
			return 0;
		}
		return colorAttachments[i];
	}
	void bind();
private:
	int W, H;
	int nColorAttachments;
	unsigned int * colorAttachments;
};