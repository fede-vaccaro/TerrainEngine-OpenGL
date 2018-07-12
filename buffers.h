#pragma once
#include <glad/glad.h>

const int SCR_WIDTH_ = 1920;
const int SCR_HEIGHT_ = 1080;

const int FBW = 1280;
const int FBH = 720;

void bindFrameBuffer(int frameBuffer, int width, int height);

void unbindCurrentFrameBuffer(int scrWidth, int scrHeight);

unsigned int createFrameBuffer();

unsigned int createTextureAttachment(int width, int height);

unsigned int createDepthTextureAttachment(int width, int height);

unsigned int createDepthBufferAttachment(int width, int height);

unsigned int createRenderBufferAttachment(int width, int height);