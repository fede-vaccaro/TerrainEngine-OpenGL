#pragma once
#include <glm/glm.hpp>
//#include <model.h>
#include "texture.h"
//#include "TessShader.h"
#include "Water.h"
#include <camera.h>
#include "glError.h"
#include "drawableObject.h"
#include "Water.h"

enum tPosition {
	C, N, S, E, W, SE, SW, NE, NW, totTiles
};

class Tile : public drawableObject
{
public:

	Tile(float scale, float disp, int gl);
	virtual ~Tile();
	//void drawTile(Camera * camera, glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor, float waterHeight, float up, std::vector<glm::vec2> & pos);
	virtual void draw();
	virtual void setGui();

	void updateTiles();
	
	void setPositionsUniforms(std::vector<glm::vec2> & pos);
	void setPositionsArray(std::vector<glm::vec2> & pos);
	void deleteBuffer() {
		glDeleteBuffers(1, &posBuffer);
		posBuffer = 0;
	}

	glm::vec2 position, eps;
	float up = 0.0;
	
	bool inTile(Camera camera, glm::vec2 pos);
	static const int tileW = 10.;

	//Model * planeModel;
	Water * waterPtr;

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
	int res;
	//void initializePlaneVAO();
	void drawVertices(int nInstances);
	unsigned int planeVBO, planeVAO, planeEBO;

	float dispFactor, scaleFactor, frequency, grassCoverage, tessMultiplier, fogFalloff;
	int octaves;
	int gridLength;
	glm::vec2 I, J;

	unsigned int * textures, posBuffer;

	Shader * shad;
	glm::mat4 modelMatrix;

	std::vector<glm::vec2> positionVec;

	void setPos(int row, int col, glm::vec2 pos) {
		positionVec[col + row * gridLength] = pos;
	}

	glm::vec2 getPos(int row, int col) {
		return positionVec[col + row * gridLength];
	}

	void changeTiles(tPosition currentTile);
	void addColumn(int direction);
	void addRow(int direction);

	void getColRow(int i, int& col, int& row) {
		col = (i) % gridLength;

		row = (i - col) / gridLength;

		//col--, row--;
	}

	std::string inline direction(tPosition pos) {
		switch (pos)
		{
		default: return "nowhere";
		case C: return "Centre";
		case N: return "North";
		case S: return "South";
		case E: return "East";
		case W: return "West";
		case NE: return "North-East";
		case NW: return "North-West";
		case SE: return "South-East";
		case SW: return "South-West";
			break;
		}
	}

	void reset();
};

