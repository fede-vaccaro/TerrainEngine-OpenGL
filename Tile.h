#pragma once
#include <glm/glm.hpp>
#include <model.h>
#include "texture.h"
#include "TessShader.h"
#include "Water.h"
#include <camera.h>
#include "glError.h"


class Tile
{
public:
	Tile(glm::vec2 position, float scale, float disp, TessellationShader * shad, Model * planeModel, unsigned int * textures);
	virtual ~Tile();
	void drawTile(Camera * camera, glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor, float waterHeight, float up, std::vector<glm::vec2> & pos);
	void setPositionsUniforms(std::vector<glm::vec2> & pos);
	void setPositionsArray(std::vector<glm::vec2> & pos);
	void deleteBuffer() {
		glDeleteBuffers(1, &posBuffer);
		posBuffer = 0;
	}

	glm::vec2 position, eps;
	
	bool inTile(Camera camera, glm::vec2 pos);
	static const int tileW = 5.0;
	Model * planeModel;

	void setOctaves(int oct) {
		if (oct > 0) octaves = oct;
	}

	void setFreq(float freq) {
		if (freq > 0.0f) { this->frequency = freq; }
	}

	void setDispFactor(float disp) {
		if (disp > 0.0f) { dispFactor = disp; }
	}

	void setScale(float scale) {
		glm::mat4 id;
		glm::mat4 scaleMatrix = glm::scale(id, glm::vec3(scale, 0.0, scale));
		glm::mat4 positionMatrix = glm::translate(id, glm::vec3(position.x*scale/this->scaleFactor, 0.0, position.y*scale / this->scaleFactor));
		modelMatrix = positionMatrix * scaleMatrix;
		scaleFactor = scale;
	}

	void setGrassCoverage(float gc) {
		grassCoverage = gc;
	}

	void setTessMultiplier(float tm) {
		if (tm > 0.0) tessMultiplier = tm;
	}

	int getOctaves() const { return octaves; }
	float getFreq() const { return frequency; }
	float getDispFactor() const { return dispFactor; }
	float getScale() const { return scaleFactor; }
	float getGrassCoverage() const { return grassCoverage; }
	float getTessMultiplier() const { return tessMultiplier; }

	static bool drawFog;
private:
	float dispFactor, scaleFactor, frequency, grassCoverage, tessMultiplier;
	int octaves;

	unsigned int * textures, posBuffer;

	TessellationShader * shad;
	glm::mat4 modelMatrix;
};

