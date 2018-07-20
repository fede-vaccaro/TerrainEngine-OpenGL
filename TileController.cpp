#include "TileController.h"
#include <GLFW/glfw3.h>


TileController::TileController(float scale, float disp, Camera * camera, TessellationShader * shad, Shader * waterShader) : scale(scale), disp(disp), camera(camera), shad(shad), waterShader(waterShader)
{
	gridLenght = 60;

	//position.resize(gridLenght*gridLenght);

	float s = scale * Tile::tileW;

	this->I = glm::vec2(1, 0)*s;
	this->J = glm::vec2(0, 1)*s;


	planeModel = new Model("resources/plane.obj", GL_PATCHES);
	waterModel = new Model("resources/plane.obj", GL_TRIANGLES);

	this->textures = new unsigned int[4];
	textures[0] = TextureFromFile("sand.jpg", "resources", false);
	textures[1] = TextureFromFile("grass.jpg", "resources", false);
	textures[2] = TextureFromFile("rock4.jpg", "resources", false);
	textures[3] = TextureFromFile("snow2.jpg", "resources", false);

	
	dudvMap = TextureFromFile("waterDUDV.png", "resources", false);
	normalMap = TextureFromFile("normalMap.png", "resources", false);

	waterHeight = 9.0;

	position.resize(gridLenght*gridLenght);
	for (int i = 0; i < gridLenght; i++) {
		for (int j = 0; j < gridLenght; j++) {
			glm::vec2 pos = (float)(j - gridLenght / 2)*glm::vec2(I) + (float)(i - gridLenght / 2)*glm::vec2(J);
			setPos(i, j, pos);
		}
	}

	tile = new Tile(glm::vec2(0,0), scale, disp, shad, planeModel, textures);
	tile->setPositionsArray(position);

	waterPtr = new Water(glm::vec2(0.0,0.0), waterShader, scale*gridLenght, waterHeight, dudvMap, normalMap, waterModel);
};

void TileController::drawTiles(glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor) {
	
	// reflection
	waterPtr->bindReflectionFBO();
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->invertPitch();
	camera->Position.y -= 2 * (camera->Position.y - waterHeight);	
	tile->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, 1.0f, position);
	
	camera->invertPitch();
	camera->Position.y += 2 * abs(camera->Position.y - waterHeight);
	waterPtr->unbindFBO();

	// refraction
	waterPtr->bindRefractionFBO();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	tile->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, -1.0f, position);
	waterPtr->unbindFBO();
	
	// real draw
	tile->drawTile(camera, proj, lightPosition, lightColor, fogColor, 0.0, 0.0, position);
	waterPtr->draw(proj* (camera->GetViewMatrix()), lightPosition, lightColor, camera->Position);

}

void TileController::updateTiles() {
	glm::vec2 camPosition(camera->Position.x, camera->Position.z);
	int whichTile = -1;
	int howManyTiles = 0;
	bool found = false;

	glm::vec2 posC = getPos(gridLenght / 2, gridLenght / 2),
		posS = posC - I,
		posN = posC + I,
		posE = posC + J,
		posW = posC - J;

	//if (tile->inTile(*camera, posC)) std::cout << "IN C" << std::endl;
	if (tile->inTile(*camera, posS)) whichTile = S, howManyTiles++;// , std::cout << "IN S" << std::endl;
	if (tile->inTile(*camera, posE)) whichTile = E, howManyTiles++;// , std::cout << "IN E" << std::endl;
	if (tile->inTile(*camera, posW)) whichTile = W, howManyTiles++;// , std::cout << "IN W" << std::endl;
	if (tile->inTile(*camera, posN)) whichTile = N, howManyTiles++;// , std::cout << "IN N" << std::endl;

	//std::cout << camPosition.x << " " << camPosition.y << std::endl;

	if (howManyTiles == 1) {
		std::cout << "Changing tile reference system to: " << direction((tPosition)whichTile) << std::endl;
		changeTiles((tPosition)whichTile);
		tile->setPositionsArray(position);
	}
	else if (howManyTiles > 1) {
		std::cout << "You're on a border!" << std::endl;
	}

}
void TileController::reset() {
	int octaves = this->getOctaves();
	float freq = this->getFreq();
	float grassCoverage = this->getGrassCoverage();
	float dispFactor = this->getDispFactor();
	float tessMultiplier = this->getTessMultiplier();

	setOctaves(octaves); 
	setFreq(freq);
	setGrassCoverage(grassCoverage); 
	setDispFactor(dispFactor);
	setTessMultiplier(tessMultiplier);
}

void TileController::changeTiles(tPosition currentTile) {
	if (currentTile == N) {
		addRow(1);
	}
	else if (currentTile == S) {
		addRow(-1);
	}
	else if (currentTile == E) {
		addColumn(1);
	}
	else if (currentTile = W) {
		addColumn(-1);
	}

	waterPtr->setPosition(getPos(gridLenght / 2, gridLenght / 2), scale*gridLenght, waterHeight);
}


TileController::~TileController()
{
}


void TileController::addColumn(int direction) {
	if (direction > 0) {

		for (int i = 0; i < gridLenght - 1; i++) {
			for (int j = 0; j < gridLenght; j++) {
				setPos(i, j, getPos(i + 1, j));
			}
		}

		int i = gridLenght - 1;
		for (int j = 0; j < gridLenght; j++) {
			setPos(i, j, getPos(i, j) + this->J);
		}

	}
	else if (direction < 0) {
		for (int i = gridLenght - 1; i > 0; i--) {
			for (int j = 0; j < gridLenght; j++) {
				setPos(i, j, getPos(i - 1, j));
			}
		}

		int i = 0;
		for (int j = 0; j < gridLenght; j++) {
			setPos(i, j, getPos(i, j) - this->J);
		}

	}
}

void TileController::addRow(int direction) {
	if (direction > 0) {

		for (int i = 0; i < gridLenght; i++) {
			for (int j = 0; j < gridLenght - 1; j++) {
				setPos(i, j, getPos(i, j + 1));
			}
		}

		int j = gridLenght - 1;
		for (int i = 0; i < gridLenght; i++) {
			setPos(i, j, getPos(i, j) + this->I);
		}

	}
	else if (direction < 0) {
		for (int i = 0; i < gridLenght; i++) {
			for (int j = gridLenght - 1; j > 0; j--) {
				setPos(i, j, getPos(i, j - 1));
			}
		}

		int j = 0;
		for (int i = 0; i < gridLenght; i++) {
			setPos(i, j, getPos(i, j) - this->I);
		}

	}
}