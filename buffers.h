#pragma once
#include <glad/glad.h>
#include "Window.h"

void bindFrameBuffer(int frameBuffer, int width, int height);

void unbindCurrentFrameBuffer(int scrWidth, int scrHeight);
void unbindCurrentFrameBuffer();

unsigned int createFrameBuffer();

unsigned int createTextureAttachment(int width, int height);

unsigned int createDepthTextureAttachment(int width, int height);

unsigned int createDepthBufferAttachment(int width, int height);

unsigned int createRenderBufferAttachment(int width, int height);

class FrameBufferObject {
public:
	FrameBufferObject(int W, int H);
	unsigned int FBO, renderBuffer, tex, depthTex;
	void bind();
private:
	int W, H;
};