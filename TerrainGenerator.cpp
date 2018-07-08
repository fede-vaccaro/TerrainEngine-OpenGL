#include "TerrainGenerator.h"


int TerrainGenerator::seed = (int)rand() % 10;
int TerrainGenerator::resolution = 1024;

float * TerrainGenerator::pointMap = TerrainGenerator::createMapCoords(resolution);

float * TerrainGenerator::createMapCoords(int resolution) {
	/* Create a HeightMap in normalized coords [0,1] */

	
	float * map = new float[resolution*resolution*2];

	float WidthDensity = resolution;
	float HeightDensity = resolution;
	int i = 0;
	for (int z = 0; z < resolution; z++) {
		for (int x = 0; x < resolution * 2; x += 2) {
			map[z*resolution * 2 + x] = (float)(i++ / (WidthDensity / 2) - 1);
			map[z*resolution * 2 + x + 1] = (float)(z / (HeightDensity / 2) - 1);
		}
		i = 0;
	}
	return map;
}

TerrainGenerator::TerrainGenerator()
{
	fbo = createFrameBuffer();
	SCR_WIDTH = 1920;
	SCR_HEIGHT = 1080;
	this->shad = new Shader("shaders/TerrainGeneratorVertex.vert", "shaders/TerrainGeneratorFragment.frag");
	this->pointObj = new Object(TerrainGenerator::pointMap, 2 * resolution * resolution, 2);
	pointObj->setDrawMode(GL_POINTS);

	float vertices[] = {
	   -1.0f, -1.0f, 
	   -1.0f,  1.0f,  
		1.0f, -1.0f, 
		1.0f, -1.0f, 
	   -1.0f,  1.0f, 
		1.0f,  1.0f
	};


}

unsigned int TerrainGenerator::generateHeightMap(glm::vec2 offset) {
	bindFrameBuffer(fbo, resolution, resolution);
	unsigned int tex = createTextureAttachment(resolution-1, resolution-1);
	shad->use();
	shad->setInt("PrimeIndex", seed);
	shad->setInt("resolution", resolution);
	shad->setVec2("offset", offset);
	pointObj->drawObject();

	//glBindVertexArray(VAO);
	//glDrawArrays(GL_PATCHES, 0, 6);

	unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);

	return tex;

}


TerrainGenerator::~TerrainGenerator()
{
}
