#pragma once
#include <shaderUtils.h>
#include "buffers.h"
#include "Object.h"

class TerrainGenerator
{
public:
	TerrainGenerator();
	virtual ~TerrainGenerator();
	unsigned int generateHeightMap(glm::vec2 offset);
	
	static float * createMapCoords(int resolution);
	Shader * shad;
	Object * pointObj;
	//unsigned int VAO, VBO;
private:
	unsigned int fbo;
	int SCR_WIDTH, SCR_HEIGHT;

	static int seed;
	static int resolution;
	static float * pointMap;

};

