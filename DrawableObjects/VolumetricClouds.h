#pragma once
#include "../Engine/ScreenSpaceShader.h"
#include "../Engine/buffers.h"
#include "../Engine/texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera.h>
#include <algorithm>
#include "drawableObject.h"
#include "CloudsModel.h"

//VolumetricClouds handles the FrameBufferObjects (and textures) where the clouds will be rendered, and it's responsible to set up the uniforms and calling the draw command.
class VolumetricClouds : public drawableObject
{
public:
	VolumetricClouds(int SW, int SH, CloudsModel * model);
	virtual void draw();
	~VolumetricClouds();

	enum cloudsTextureNames {fragColor, bloom, alphaness, cloudDistance};

	void generateWeatherMap();

	unsigned int getCloudsTexture() { 
		return (model->postProcess ? cloudsPostProcessingFBO->getColorAttachmentTex(0) : getCloudsRawTexture());
	}

	unsigned int getCloudsTexture(int i) {
		return cloudsFBO->getColorAttachmentTex(i);
	}

	unsigned int getCloudsRawTexture(){
		return cloudsFBO->getColorAttachmentTex(0);
	}



private:
	int SCR_WIDTH, SCR_HEIGHT;

	TextureSet * cloudsFBO;
	FrameBufferObject *cloudsPostProcessingFBO;

	CloudsModel * model;
};

