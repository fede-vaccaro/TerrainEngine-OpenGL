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
	glm::vec2 I, J;

	TileController(float scale, float disp, Camera * camera, TessellationShader * shad, Shader * waterShader);
	virtual ~TileController();
	
	void updateTiles();

	void drawTiles(glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor);
	Model * planeModel, * waterModel;
	unsigned int * textures, normalMap, dudvMap;
	Tile * tile;

	Tile * getTile() {
		return tile;
	}

	std::vector<glm::vec2> & getPos(){
		return position;
	}

	void setWaterHeight(float height) {
		waterHeight = height;
		waterPtr->setPosition( getPos(gridLenght/2, gridLenght/2) , scale*gridLenght, waterHeight);
	}

	float getWaterHeight() const { return waterHeight; };

	void setOctaves(int o) {
		tile->setOctaves(o);
	}

	int getOctaves() const {
		return tile->getOctaves();
	}

	void setFreq(float f) {
		tile->setFreq(f);
	}

	float getFreq() const {
		return tile->getFreq();
	}

	void setDispFactor(float df) {
		tile->setDispFactor(df);
		
		this->disp = df;
	}

	float getDispFactor() const{
		return disp;
	}

	void setGrassCoverage(float gc) {
		tile->setGrassCoverage(gc);
		
	}

	float getGrassCoverage() const {
		return tile->getGrassCoverage();
	}

	void setTessMultiplier(float tm) {
		tile->setTessMultiplier(tm);
	}
	
	float getTessMultiplier() const {
		return tile->getTessMultiplier();
	}
	
	bool snowy(glm::vec3 &lightColor) {
		if (snowy_) {
			lightColor = glm::vec3(220, 255, 255) / 255.0f;
		}
		else {
			lightColor = glm::vec3(255, 255, 200) / 255.0f;
		}
		std::swap(textures[1], textures[3]);
		std::swap(textures[2], textures[5]);
		this->snowy_ = !snowy_;
		return !(this->snowy_) ;
	}

	Water * const getWaterPtr() { return waterPtr; }


private:
	bool snowy_ = true;
	int gridLenght;
	Water * waterPtr;
	Camera * camera;
	float scale, disp, waterHeight;
	TessellationShader * shad;
	Shader * waterShader;
	std::vector<glm::vec2> position;

	void setPos(int row, int col, glm::vec2 pos) {
		position[col + row * gridLenght] = pos;
	}

	glm::vec2 getPos(int row, int col) {
		return position[col + row * gridLenght];
	}

	void changeTiles(tPosition currentTile);
	void addColumn(int direction);
	void addRow(int direction);

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

