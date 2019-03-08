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

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

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

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorAttachments[i], 0);
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

void initializePlaneVAO(const int res, const int width, GLuint * planeVAO, GLuint * planeVBO, GLuint * planeEBO) {

	//const int res = 3;
	const int nPoints = res * res;
	const int size = nPoints * 3 + nPoints * 3 + nPoints * 2;
	float * vertices = new float[size];
	for (int i = 0; i < res; i++) {
		for (int j = 0; j < res; j++) {
			//add position
			float x = j * (float)width / (res - 1) - width / 2.0;
			float y = 0.0;
			float z = -i * (float)width / (res - 1) + width / 2.0;

			vertices[(i + j * res) * 8] = x; //8 = 3 + 3 + 2, float per point
			vertices[(i + j * res) * 8 + 1] = y;
			vertices[(i + j * res) * 8 + 2] = z;

			//add normal
			float x_n = 0.0;
			float y_n = 1.0;
			float z_n = 0.0;

			vertices[(i + j * res) * 8 + 3] = x_n;
			vertices[(i + j * res) * 8 + 4] = y_n;
			vertices[(i + j * res) * 8 + 5] = z_n;

			//add texcoords
			vertices[(i + j * res) * 8 + 6] = (float)j / (res - 1);
			vertices[(i + j * res) * 8 + 7] = (float)(res - i - 1) / (res - 1);
		}
	}

	const int nTris = (res - 1)*(res - 1) * 2;
	int * trisIndices = new int[nTris * 3];

	for (int i = 0; i < nTris; i++) {
		int trisPerRow = 2 * (res - 1);
		for (int j = 0; j < trisPerRow; j++) {
			if (!(i % 2)) { //upper triangle
				int k = i * 3;
				int triIndex = i % trisPerRow;

				int row = i / trisPerRow;
				int col = triIndex / 2;
				trisIndices[k] = row * res + col;
				trisIndices[k + 1] = ++row*res + col;
				trisIndices[k + 2] = --row* res + ++col;
			}
			else {
				int k = i * 3;
				int triIndex = i % trisPerRow;

				int row = i / trisPerRow;
				int col = triIndex / 2;
				trisIndices[k] = row * res + ++col;
				trisIndices[k + 1] = ++row * res + --col;
				trisIndices[k + 2] = row * res + ++col;
			}
		}
	}

	/*
	for (int i = 0; i < res; i++) {
		for (int j = 0; j < res; j++) {
			for (int k = 0; k < 8; k++) {
				if (k == 3 || k == 6)
					std::cout << std::endl;
				std::cout << vertices[(i + j * res) * 8 + k] << ", ";
			}
			std::cout << std::endl;
		}
	}

	for (int i = 0; i < nTris; i++) {
		for (int k = 0; k < 3; k++)
		{
			std::cout << trisIndices[i * 3 + k] << ", ";
		}
		std::cout << std::endl;

	}

	std::cout << "TRISINDICES: " << nTris * 3 << std::endl;
	*/
	glGenVertexArrays(1, planeVAO);
	glGenBuffers(1, planeVBO);
	glGenBuffers(1, planeEBO);

	glBindVertexArray(*planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, *planeVBO);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTris * 3 * sizeof(unsigned int), trisIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	delete[] vertices;
}

TextureSet::TextureSet(int W, int H, int num)
{
	if (W > 0 && H > 0 && num > 0) {
		nTextures = num;
		texture = new unsigned int[num];
		for (int i = 0; i < num; ++i) {
			texture[i] = Texture2D(W, H);
		}
	}
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
