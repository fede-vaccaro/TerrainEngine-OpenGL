#include "buffers.h"
#include "texture.h"
void bindFrameBuffer(int frameBuffer, int width, int height) {
	glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);
}

void unbindCurrentFrameBuffer(int scrWidth, int scrHeight) {//call to switch to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, scrWidth, scrHeight);
}

void unbindCurrentFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Window::SCR_WIDTH, Window::SCR_HEIGHT);
}

unsigned int createFrameBuffer() {
	unsigned int frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	return frameBuffer;
}

unsigned int createTextureAttachment(int width, int height) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	return texture;
}

unsigned int * createColorAttachments(int width, int height, unsigned int nColorAttachments) {
	unsigned int * colorAttachments = new unsigned int[nColorAttachments];
	glGenTextures(nColorAttachments, colorAttachments);

	for (unsigned int i = 0; i < nColorAttachments; i++) {
		glBindTexture(GL_TEXTURE_2D, colorAttachments[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachments[i], 0);
	}
	return colorAttachments;
}

unsigned int createDepthTextureAttachment(int width, int height) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
	return texture;
}

unsigned int createDepthBufferAttachment(int width, int height) {
	unsigned int depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	return depthBuffer;
}

unsigned int createRenderBufferAttachment(int width, int height) {
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	return rbo;
}



FrameBufferObject::FrameBufferObject(int W_, int H_) {
	this->W = W_;
	this->H = H_;
	this->FBO = createFrameBuffer();
	//this->renderBuffer = createRenderBufferAttachment(W, H);
	this->tex = createTextureAttachment(W, H);
	this->depthTex = createDepthTextureAttachment(W, H);

	colorAttachments = NULL;
	nColorAttachments = 1;
}

FrameBufferObject::FrameBufferObject(int W_, int H_, const int nColorAttachments) {
	this->W = W_;
	this->H = H_;
	this->FBO = createFrameBuffer();

	this->tex = NULL;
	this->depthTex = createDepthTextureAttachment(W, H);
	this->colorAttachments = createColorAttachments(W, H, nColorAttachments);
	this->nColorAttachments = nColorAttachments;

	unsigned int * colorAttachmentsFlag = new unsigned int[nColorAttachments];
	for (unsigned int i = 0; i < nColorAttachments; i++) {
		colorAttachmentsFlag[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(nColorAttachments, colorAttachmentsFlag);
	delete colorAttachmentsFlag;
}


void FrameBufferObject::bind() {
	bindFrameBuffer(this->FBO, this->W, this->H);
}

unsigned int FrameBufferObject::getColorAttachmentTex(int i) {
	if (i < 0 || i > nColorAttachments) {
		std::cout << "COLOR ATTACHMENT OUT OF RANGE" << std::endl;
		return 0;
	}
	return colorAttachments[i];
}

TextureSet::TextureSet(int W, int H, int num)
{
	if (W > 0 && H > 0 && num > 0) {
		nTextures = num;
		texture = new unsigned int[num];
		for (int i = 0; i < num; ++i) {
			texture[i] = generateTexture2D(W, H);
		}
	}
}


unsigned int TextureSet::getColorAttachmentTex(int i) {
	if (i < 0 || i > nTextures) {
		std::cout << "COLOR ATTACHMENT OUT OF RANGE" << std::endl;
		return 0;
	}
	return texture[i];
}

void TextureSet::bindTexture(int i, int unit)
{
	bindTexture2D(texture[i], unit);
}

void TextureSet::bind()
{
	for (int i = 0; i < nTextures; ++i)
		bindTexture2D(texture[i], i);
}
