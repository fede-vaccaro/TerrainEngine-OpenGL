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
	TileController(float scale, float disp, Camera * camera, TessellationShader * shad, Shader * waterShader, TerrainGenerator * tg);
	virtual ~TileController();
	
	void updateTiles();

	void drawTiles(glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor);
	Model * planeModel, * waterModel;
	unsigned int * textures, normalMap, dudvMap;
	std::vector<Tile*> tiles;
	float waterHeight;

private:
	Camera * camera;
	float scale, disp;
	TessellationShader * shad;
	Shader * waterShader;
	TerrainGenerator * tg;
	glm::vec2 * position;

	void changeTiles(tPosition currentTile);

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

