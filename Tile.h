#pragma once
#include <glm/glm.hpp>
#include <model.h>
#include "texture.h"
#include "TessShader.h"
#include "TerrainGenerator.h"
#include "Water.h"
#include <camera.h>
//static TerrainGenerator tg;

class Tile
{
public:
	Tile(glm::vec2 position, float scale, float disp, TessellationShader * shad, TerrainGenerator * tg, Model * planeModel, unsigned int * textures);
	virtual ~Tile();
	void drawTile(Camera * camera, glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor, float waterHeight, float up);
	unsigned int heightMap;
	glm::vec2 position, eps;
	
	bool inTile(Camera camera);
	static const int tileW = 5.0;
	Model * planeModel;
	Water * water;

	void setWater(Water * w);
	static bool drawFog;
private:
	float dispFactor, scaleFactor;
	unsigned int * textures;

	TessellationShader * shad;
	TerrainGenerator * tg;

	glm::mat4 modelMatrix;
};

