#include "TileController.h"
#include <GLFW/glfw3.h>


TileController::TileController(float scale, float disp, int gl) : scale(scale), disp(disp)
{

	waterShader = new Shader("shaders/waterVertexShader.vert", "shaders/waterFragmentShader.frag");
	std::cout << "============= CREATING TSHADER ==============" << std::endl;
	shad = new TessellationShader("shaders/tessVertexShader.vert", "shaders/tessControlShader.tcs", "shaders/tessEvaluationShader.tes", "shaders/tessFragmentShader.frag");
	std::cout << "============= TSHADER CREATED ==============" << std::endl;


	this->gridLength = gl + (gl + 1)%2; //ensure gridLength is odd

	float s = scale * Tile::tileW;

	this->I = glm::vec2(1, 0)*s;
	this->J = glm::vec2(0, 1)*s;


	planeModel = new Model("resources/plane.obj", GL_PATCHES);
	waterModel = new Model("resources/plane.obj", GL_TRIANGLES);

	this->textures = new unsigned int[6];
	textures[0] = TextureFromFile("sand.jpg", "resources", false);
	textures[1] = TextureFromFile("grass.jpg", "resources", false);
	textures[2] = TextureFromFile("rdiffuse.jpg", "resources", false);
	textures[3] = TextureFromFile("snow2.jpg", "resources", false);
	textures[4] = TextureFromFile("rnormal.jpg", "resources", false);
	textures[5] = TextureFromFile("terrainTexture.jpg", "resources", false);
	
	dudvMap = TextureFromFile("waterDUDV.png", "resources", false);
	normalMap = TextureFromFile("normalMap.png", "resources", false);

	waterHeight = 128.0 + 50.0;

	position.resize(gridLength*gridLength);
	for (int i = 0; i < gridLength; i++) {
		for (int j = 0; j < gridLength; j++) {
			glm::vec2 pos = (float)(j - gridLength / 2)*glm::vec2(I) + (float)(i - gridLength / 2)*glm::vec2(J);
			setPos(i, j, pos);
		}
	}

	tile = new Tile(glm::vec2(0,0), scale, disp, shad, planeModel, textures);
	tile->setPositionsArray(position);

	waterPtr = new Water(glm::vec2(0.0,0.0), waterShader, scale*gridLength, waterHeight, dudvMap, normalMap, waterModel);
};

void TileController::draw() {
	
	// reflection
	waterPtr->bindReflectionFBO();
	glClearColor(0.0, 0.4*0.8, 0.7*0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sceneElements* s = drawableObject::scene;
	s->cam.invertPitch();
	s->cam.Position.y -= 2 * (s->cam.Position.y - waterHeight);
	//tile->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, 1.0f, position);
	
	s->cam.invertPitch();
	s->cam.Position.y += 2 * abs(s->cam.Position.y - waterHeight);
	//waterPtr->unbindFBO();

	// refraction
	waterPtr->bindRefractionFBO();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	tile->drawTile(&(s->cam), s->projMatrix, s->lightPos, s->lightColor, s->fogColor, waterHeight, -1.0f, position);
	//waterPtr->unbindFBO(); 
	//bind off screen drawing FBO
	s->sceneFBO.bind();
		
	// real draw
	tile->drawTile(&(s->cam), s->projMatrix, s->lightPos, s->lightColor, s->fogColor, waterHeight, 0.0, position);

	waterPtr->setPosition(glm::vec2(s->cam.Position.x, s->cam.Position.z), scale*gridLength, waterHeight);
	waterPtr->draw(s->projMatrix* (s->cam.GetViewMatrix()), s->lightPos, s->lightColor, s->cam.Position);

}

void TileController::updateTiles() {
	sceneElements* s = drawableObject::scene;
	glm::vec2 camPosition(s->cam.Position.x, s->cam.Position.z);
	int whichTile = -1;
	int howManyTiles = 0;
	bool found = false;

	glm::vec2 posC = getPos(gridLength / 2, gridLength / 2),
		posS = posC - I,
		posN = posC + I,
		posE = posC + J,
		posW = posC - J,
		posSE = posS + posE,
		posSW = posS + posW,
		posNE = posN + posE,
		posNW = posN + posW;

	//if (tile->inTile(s->cam, posC)) std::cout << "IN C" << std::endl;
	if (tile->inTile(s->cam, posS)) whichTile = S, howManyTiles++;// , std::cout << "IN S" << std::endl;
	if (tile->inTile(s->cam, posE)) whichTile = E, howManyTiles++;// , std::cout << "IN E" << std::endl;
	if (tile->inTile(s->cam, posW)) whichTile = W, howManyTiles++;// , std::cout << "IN W" << std::endl;
	if (tile->inTile(s->cam, posN)) whichTile = N, howManyTiles++;// , std::cout << "IN N" << std::endl;
	//if (tile->inTile(s->cam, posNE)) whichTile = NE, howManyTiles++;
	//if (tile->inTile(s->cam, posNW)) whichTile = NW, howManyTiles++;
	//if (tile->inTile(s->cam, posSE)) whichTile = SE, howManyTiles++;
	//if (tile->inTile(s->cam, posSW)) whichTile = SW, howManyTiles++;

	for (int i = 0; i < position.size(); i++) {
		if (tile->inTile(s->cam, position[i])) {
			int row, col;
			this->getColRow(i, col, row);
			//std::cout << "In position, col: " << col << ", row: " << row << std::endl;
		}

	}

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
	else if (currentTile = NW) {
		addRow(1);
		addColumn(-1);
	}
	else if (currentTile = NE) {
		addRow(1);
		addColumn(1);
	}
	else if (currentTile = SW) {
		addRow(-1);
		addColumn(-1);
	}
	else if (currentTile = SE) {
		addRow(-1);
		addColumn(1);
	}

	waterPtr->setPosition(getPos(gridLength / 2, gridLength / 2), scale*gridLength, waterHeight);
}


TileController::~TileController()
{
}


void TileController::addColumn(int direction) {
	if (direction > 0) {

		for (int i = 0; i < gridLength - 1; i++) {
			for (int j = 0; j < gridLength; j++) {
				setPos(i, j, getPos(i + 1, j));
			}
		}

		int i = gridLength - 1;
		for (int j = 0; j < gridLength; j++) {
			setPos(i, j, getPos(i, j) + this->J);
		}

	}
	else if (direction < 0) {
		for (int i = gridLength - 1; i > 0; i--) {
			for (int j = 0; j < gridLength; j++) {
				setPos(i, j, getPos(i - 1, j));
			}
		}

		int i = 0;
		for (int j = 0; j < gridLength; j++) {
			setPos(i, j, getPos(i, j) - this->J);
		}

	}
}

void TileController::addRow(int direction) {
	if (direction > 0) {

		for (int i = 0; i < gridLength; i++) {
			for (int j = 0; j < gridLength - 1; j++) {
				setPos(i, j, getPos(i, j + 1));
			}
		}

		int j = gridLength - 1;
		for (int i = 0; i < gridLength; i++) {
			setPos(i, j, getPos(i, j) + this->I);
		}

	}
	else if (direction < 0) {
		for (int i = 0; i < gridLength; i++) {
			for (int j = gridLength - 1; j > 0; j--) {
				setPos(i, j, getPos(i, j - 1));
			}
		}

		int j = 0;
		for (int i = 0; i < gridLength; i++) {
			setPos(i, j, getPos(i, j) - this->I);
		}

	}
}