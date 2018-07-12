#pragma once
#include "Tile.h"
#include <camera.h>
#include <vector>
#include <model.h>

enum tPosition {
	C, N, S, E, W, SE, SW, NE, NW, totTiles
};

class TileController
{
public:
	TileController(float scale, float disp, Camera * camera, TessellationShader * shad, Shader * waterShader);
	virtual ~TileController();
	
	void updateTiles();

	void drawTiles(glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor);
	Model * planeModel, * waterModel;
	unsigned int * textures, normalMap, dudvMap;
	std::vector<Tile*> tiles;

	void setWaterHeight(float height) {
		waterHeight = height;
		waterPtr->setPosition(tiles[C]->position, scale*3.0, waterHeight);
	}

	float getWaterHeight() const { return waterHeight; };

	void setOctaves(int o) {
		for (int i = 0; i < tiles.size(); i++) {
			tiles[i]->setOctaves(o);
		}
	}

	int getOctaves() const {
		return tiles[0]->getOctaves();
	}

	void setFreq(float f) {
		for (int i = 0; i < tiles.size(); i++) {
			tiles[i]->setFreq(f);
		}
	}

	float getFreq() const {
		return tiles[0]->getFreq();
	}

	void setDispFactor(float df) {
		for (int i = 0; i < tiles.size(); i++) {
			tiles[i]->setDispFactor(df);
		}
		this->disp = df;
	}

	float getDispFactor() const{
		return disp;
	}

	void setGrassCoverage(float gc) {
		for (int i = 0; i < tiles.size(); i++) {
			tiles[i]->setGrassCoverage(gc);
		}
	}

	float getGrassCoverage() const {
		return tiles[0]->getGrassCoverage();
	}

	void setTessMultiplier(float tm) {
		for (int i = 0; i < tiles.size(); i++) {
			tiles[i]->setTessMultiplier(tm);
		}
	}

	float getTessMultiplier() const {
		return tiles[0]->getTessMultiplier();
	}

	Water * const getWaterPtr() { return waterPtr; }


private:
	Water * waterPtr;
	Camera * camera;
	float scale, disp, waterHeight;
	TessellationShader * shad;
	Shader * waterShader;
	glm::vec2 * position;

	void changeTiles(tPosition currentTile);
	void reset();


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
};

