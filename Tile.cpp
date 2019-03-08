#include "Tile.h"
#include "sceneElements.h"
#include <GLFW/glfw3.h>

bool Tile::drawFog = true;

float sign(float x) {
	if (x > 0.0f) return 1.0f;
	else if (x < 0.0f) { return -1.0f; }
	else return 0.0f;
}


Tile::Tile(float scale, float dispFactor, int gl) : dispFactor(dispFactor), scaleFactor(scale)
{
	float sc = scaleFactor * tileW;
	I = glm::vec2(1.0, 0.0);
	J = glm::vec2(0.0, 1.0);
	I *= sc, J *= sc;

	glm::mat4 id;
	glm::mat4 scaleMatrix = glm::scale(id, glm::vec3(scale, 0.0, scale));
	glm::mat4 positionMatrix = glm::translate(id, glm::vec3(position.x, 0.0, position.y));
	modelMatrix = positionMatrix * scaleMatrix;

	octaves = 10;
	frequency = 0.025;
	grassCoverage = 0.73;
	tessMultiplier = 2.0;

	posBuffer = 0;

	//shad = new TessellationShader("shaders/tessVertexShader.vert", "shaders/tessControlShader.tcs", "shaders/tessEvaluationShader.tes", "shaders/tessFragmentShader.frag");
	shad = new Shader("TerrainTessShader");
	shad->attachShader("shaders/tessVertexShader.vert");
	shad->attachShader("shaders/tessControlShader.tcs");
	shad->attachShader("shaders/tessEvaluationShader.tes");
	shad->attachShader("shaders/tessFragmentShader.frag");
	shad->linkPrograms();
	std::cout << "============= TSHADER CREATED ==============" << std::endl;


	this->gridLength = gl + (gl + 1) % 2; //ensure gridLength is odd

	float s = scale * Tile::tileW;

	this->I = glm::vec2(1, 0)*s;
	this->J = glm::vec2(0, 1)*s;
	res = 6;
	initializePlaneVAO(res, tileW, &planeVAO, &planeVBO, &planeEBO);
	//planeModel = new Model("resources/plane.obj", GL_PATCHES);
	//waterModel = new Model("resources/plane.obj", GL_TRIANGLES);

	//load a bunch of textures
	this->textures = new unsigned int[6];
	textures[0] = TextureFromFile("sand.jpg", "resources");
	textures[1] = TextureFromFile("grass.jpg", "resources");
	textures[2] = TextureFromFile("rdiffuse.jpg", "resources");
	textures[3] = TextureFromFile("snow2.jpg", "resources");
	textures[4] = TextureFromFile("rnormal.jpg", "resources");
	textures[5] = TextureFromFile("terrainTexture.jpg", "resources");

	//dudvMap = TextureFromFile("waterDUDV.png", "resources", false);
	//normalMap = TextureFromFile("normalMap.png", "resources", false);

	//waterHeight = 128.0 + 50.0;

	positionVec.resize(gridLength*gridLength);
	for (int i = 0; i < gridLength; i++) {
		for (int j = 0; j < gridLength; j++) {
			glm::vec2 pos = (float)(j - gridLength / 2)*glm::vec2(I) + (float)(i - gridLength / 2)*glm::vec2(J);
			setPos(i, j, pos);
		}
	}

	setPositionsArray(positionVec);

}


void Tile::draw(){

	sceneElements* se = drawableObject::scene;

	if (up != 0.0f) {
		glEnable(GL_CLIP_DISTANCE0);
	}
	glm::mat4 gWorld = modelMatrix;
	glm::mat4 gVP = se->projMatrix * se->cam.GetViewMatrix();

	shad->use();
	shad->setVec3("gEyeWorldPos", se->cam.Position);
	shad->setMat4("gWorld", gWorld);
	shad->setMat4("gVP", gVP);
	shad->setFloat("gDispFactor", dispFactor);
	float correction = 0.0f;
	if (up < 0.0f) correction = 0.05f * dispFactor;
	float waterHeight = (waterPtr ? waterPtr->getModelMatrix()[1][3] : 100.0);
	glm::vec4 clipPlane(0.0, 1.0, 0.0, -waterHeight - correction);
	shad->setVec4("clipPlane", clipPlane*up);
	shad->setVec3("u_LightColor", se->lightColor);
	shad->setVec3("u_LightPosition", se->lightPos);
	shad->setVec3("u_ViewPosition", se->cam.Position);
	shad->setVec3("fogColor", se->fogColor);
	//shad->setFloat("tessLevel", 0.0f);
	shad->setInt("octaves", octaves);
	shad->setFloat("freq", frequency);
	shad->setFloat("u_grassCoverage", grassCoverage);
	shad->setFloat("waterHeight", waterHeight);
	shad->setFloat("tessMultiplier", tessMultiplier);


	shad->setBool("normals", true);
	shad->setBool("drawFog", Tile::drawFog);


	// set textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	shad->setInt("sand", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	shad->setInt("grass", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	shad->setInt("rock", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	shad->setInt("snow", 4);

	shad->setSampler2D("grass1", textures[5], 5);

	shad->setSampler2D("rockNormal", textures[4], 6);

	int nIstances = positionVec.size();
	//planeModel->Draw(*shad, nIstances);
	drawVertices(nIstances);


	glDisable(GL_CLIP_DISTANCE0);
	up = 0.0;
}

void Tile::drawVertices(int nInstances) {
	glBindVertexArray(planeVAO);
	//shader.use();
	shad->use();
	glDrawElementsInstanced(GL_PATCHES, (res-1)*(res-1)*2*3, GL_UNSIGNED_INT, 0, nInstances);
	glBindVertexArray(0);
}

void Tile::setPositionsUniforms( std::vector<glm::vec2> & pos) {
	shad->use();
	float t1 = glfwGetTime();
	int nIstances = pos.size();
	for (int i = 0; i < nIstances; i++) {
		std::stringstream ss;
		ss << i;
		std::string num = ss.str();
		shad->setVec2("position[" + num + "]", pos[i]);
	}
	float t2 = glfwGetTime();

	std::cout << "Time to submit positions: " << t2 - t1 << "; ";

}

void Tile::setPositionsArray(std::vector<glm::vec2> & pos) {
	if (posBuffer) {
		this->deleteBuffer();
	}

	// vertex Buffer Object
	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec2), &pos[0], GL_STATIC_DRAW);

	glBindVertexArray(planeVAO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glVertexAttribDivisor(3, 1);
	glBindVertexArray(0);
	
}

bool Tile::inTile(Camera camera, glm::vec2 pos) {
	float camX = camera.Position.x;
	float camY = camera.Position.z;

	float x = pos.x;
	float y = pos.y;

	bool inX = false;
	if ((camX < x + scaleFactor * tileW/2.0f) && (camX > x - scaleFactor * tileW/2.0f)) { inX = true; }
	bool inY = false;
	if ((camY < y + scaleFactor * tileW/2.0f) && (camY > y - scaleFactor * tileW/2.0f)) { inY = true; }

	bool result = inX && inY;

	if (result) {

		//std::cout << y << " :y" << std::endl;
		//std::cout << y << " :y" << std::endl;

		//std::cout << y + scaleFactor * tileW / 2.0f << ": y + scalefactor" << std::endl;
		//std::cout << y - scaleFactor * tileW / 2.0f << ": y - scalefactor" << std::endl;
	}

	return result;

}


Tile::~Tile()
{

}

void Tile::updateTiles() {
	sceneElements* se = drawableObject::scene;
	glm::vec2 camPosition(se->cam.Position.x, se->cam.Position.z);
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
	if (inTile(se->cam, posS)) whichTile = S, howManyTiles++;// , std::cout << "IN S" << std::endl;
	if (inTile(se->cam, posE)) whichTile = E, howManyTiles++;// , std::cout << "IN E" << std::endl;
	if (inTile(se->cam, posW)) whichTile = W, howManyTiles++;// , std::cout << "IN W" << std::endl;
	if (inTile(se->cam, posN)) whichTile = N, howManyTiles++;// , std::cout << "IN N" << std::endl;
	//if (tile->inTile(s->cam, posNE)) whichTile = NE, howManyTiles++;
	//if (tile->inTile(s->cam, posNW)) whichTile = NW, howManyTiles++;
	//if (tile->inTile(s->cam, posSE)) whichTile = SE, howManyTiles++;
	//if (tile->inTile(s->cam, posSW)) whichTile = SW, howManyTiles++;

	for (int i = 0; i < positionVec.size(); i++) {
		if (inTile(se->cam, positionVec[i])) {
			int row, col;
			this->getColRow(i, col, row);
			//std::cout << "In position, col: " << col << ", row: " << row << std::endl;
		}

	}

	//std::cout << camPosition.x << " " << camPosition.y << std::endl;

	if (howManyTiles == 1) {
		std::cout << "Changing tile reference system to: " << direction((tPosition)whichTile) << std::endl;
		changeTiles((tPosition)whichTile);
		setPositionsArray(positionVec);
	}
	else if (howManyTiles > 1) {
		std::cout << "You're on a border!" << std::endl;
	}

}


void Tile::reset() {
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

void Tile::changeTiles(tPosition currentTile) {
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

	if (waterPtr) {
		float waterHeight = waterPtr->getModelMatrix()[1][3];
		waterPtr->setPosition(getPos(gridLength / 2, gridLength / 2), scaleFactor*gridLength, waterHeight);
	}
}

void Tile::addColumn(int direction) {
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

void Tile::addRow(int direction) {
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
