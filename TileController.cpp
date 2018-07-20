#include "TileController.h"
#include <GLFW/glfw3.h>


TileController::TileController(float scale, float disp, Camera * camera, TessellationShader * shad, Shader * waterShader) : scale(scale), disp(disp), camera(camera), shad(shad), waterShader(waterShader)
{
	position = new glm::vec2[9];

	float s = scale * Tile::tileW;

	position[C] = glm::vec2(0, 0);
	position[N] = glm::vec2(0, -1)*s;
	position[S] = glm::vec2(0, 1)*s;
	position[E] = glm::vec2(1, 0)*s;
	position[W] = glm::vec2(-1, 0)*s;
	position[SE] = position[S] + position[E];
	position[SW] = position[S] + position[W];
	position[NE] = position[N] + position[E];
	position[NW] = position[N] + position[W];

	tiles.resize(9);

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

	gridLenght = 10;
	tiles.resize(gridLenght*gridLenght);

	/*for (int i = 0; i < totTiles; i++) {
		tiles[i] = new Tile(position[i], scale, disp, shad, planeModel, textures);
	}*/

	for (int i = 0; i < gridLenght; i++) {
		for (int j = 0; j < gridLenght; j++) {
			glm::vec2 pos = (float)(j - gridLenght / 2)*position[S] + (float)(i - gridLenght / 2)*position[E];
			tiles[i + j * gridLenght] = new Tile(pos, scale, disp, shad, planeModel, textures);
		}
	}

	waterPtr = new Water(glm::vec2(0.0,0.0), waterShader, scale*gridLenght, waterHeight, dudvMap, normalMap, waterModel);

};

void TileController::drawTiles(glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor) {
	
	// reflection
	waterPtr->bindReflectionFBO();
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->invertPitch();
	camera->Position.y -= 2 * (camera->Position.y - waterHeight);

	float t1, t2;

	t1 = glfwGetTime();
	for (int j = 0; j < tiles.size(); j++) {
		tiles[j]->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, 1.0f, 15.0f);
	}
	t2 = glfwGetTime();
	//std::cout << t2 - t1 << " : time to reflect" << std::endl;


	camera->invertPitch();
	camera->Position.y += 2 * abs(camera->Position.y - waterHeight);
	waterPtr->unbindFBO();

	// refraction
	waterPtr->bindRefractionFBO();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	t1 = glfwGetTime();
	for (int j = 0; j < tiles.size(); j++) {
		tiles[j]->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, -1.0f, 15.0f);
	};
	waterPtr->unbindFBO();
	t2 = glfwGetTime();
	//std::cout << t2 - t1 << " : time to refract" << std::endl;

	t1 = glfwGetTime();

	for (int i = 0; i < tiles.size(); i++)
	{
		tiles[i]->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, 0.0f);
	};
	t2 = glfwGetTime();
	//std::cout << t2 - t1 << " : time to draw" << std::endl;
	waterPtr->draw(proj* (camera->GetViewMatrix()), lightPosition, lightColor, camera->Position);

}

void TileController::updateTiles() {
	glm::vec2 camPosition(camera->Position.x, camera->Position.z);
	int whichTile = -1;
	int howManyTiles = 0;
	bool found = false;
	for (int i = 0; i < tiles.size() && !found; i++) {
		if (tiles[i]->inTile(*camera)) {
			whichTile = i;
			howManyTiles++;
			//std::cout << "You are in Tile " << direction((tPosition)whichTile) << std::endl;
		}
	}
	//std::cout << "current position is: (" << camPosition.x << ", " << camPosition.y << ")" << std::endl;

	if (howManyTiles <= 1) {
		//	std::cout << "The current Tile is: " << direction((tPosition)whichTile) << std::endl;
		changeTiles((tPosition)whichTile);
	}
	else {
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

		std::cout << "CHANGING TILES: NORTH" << std::endl;

		tiles[SE] = tiles[E];
		tiles[SW] = tiles[W];
		tiles[S] = tiles[C];

		tiles[E] = tiles[NE]; //to replace 
		tiles[C] = tiles[N]; //to replace
		tiles[W] = tiles[NW]; //to replace

		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[NE], *t2 = tiles[N], *t3 = tiles[NW];

		float eps_y = 0.0f;//-0.02f*scale*3.0f;


		tiles[NE] = new Tile(t1->position + position[N] + glm::vec2(0.0, eps_y), scale, disp, shad, planeModel, textures);

		tiles[N] = new Tile(t2->position + position[N] + glm::vec2(0.0, eps_y), scale, disp, shad, planeModel, textures);

		tiles[NW] = new Tile(t3->position + position[N] + glm::vec2(0.0, eps_y), scale, disp, shad, planeModel, textures);
		this->reset();
	}
	else if (currentTile == S) {
		std::cout << "CHANGING TILES: SOUTH" << std::endl;

		tiles[NE] = tiles[E];
		tiles[N] = tiles[C];
		tiles[NW] = tiles[W];


		tiles[E] = tiles[SE]; //SE to replace 
		tiles[C] = tiles[S]; //S to replace
		tiles[W] = tiles[SW]; //SW to replace

		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[SE], *t2 = tiles[S], *t3 = tiles[SW];

		float eps_y = 0.0f;//-0.02f*scale*3.0f;

		tiles[SE] = new Tile(t1->position + position[S] + glm::vec2(0.0, eps_y), scale, disp, shad, planeModel, textures);

		tiles[S] = new Tile(t2->position + position[S] + glm::vec2(0.0, eps_y), scale, disp, shad, planeModel, textures);

		tiles[SW] = new Tile(t3->position + position[S] + glm::vec2(0.0, eps_y), scale, disp, shad, planeModel, textures);
		this->reset();
	}
	else if (currentTile == E) {
		std::cout << "CHANGING TILES: EAST" << std::endl;

		tiles[NW] = tiles[N];
		tiles[W] = tiles[C];
		tiles[SW] = tiles[S];

		tiles[N] = tiles[NE]; //NE to repleace
		tiles[C] = tiles[E]; //E to repleace
		tiles[S] = tiles[SE]; //SE to repleace

		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[NE], *t2 = tiles[E], *t3 = tiles[SE];

		float eps_X = 0.0;// 0.02f*scale*3.0f;

		tiles[NE] = new Tile(t1->position + position[E] + glm::vec2(eps_X, 0.0), scale, disp, shad, planeModel, textures);

		tiles[E] = new Tile(t2->position + position[E] + glm::vec2(eps_X, 0.0), scale, disp, shad, planeModel, textures);

		tiles[SE] = new Tile(t3->position + position[E] + glm::vec2(eps_X, 0.0), scale, disp, shad, planeModel, textures);
		this->reset();
	}
	else if (currentTile == W) {
		std::cout << "CHANGING TILES: WEST" << std::endl;

		tiles[NE] = tiles[N];
		tiles[E] = tiles[C];
		tiles[SE] = tiles[S];

		tiles[N] = tiles[NW]; //NW to repleace
		tiles[C] = tiles[W]; //W to repleace
		tiles[S] = tiles[SW]; //SW to repleace

		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[NW], *t2 = tiles[W], *t3 = tiles[SW];

		float eps_X = 0.0;// 0.02f*scale*3.0f;

		tiles[NW] = new Tile(t1->position + position[W] + glm::vec2(eps_X, 0.0), scale, disp, shad, planeModel, textures);

		tiles[W] = new Tile(t2->position + position[W] + glm::vec2(eps_X, 0.0), scale, disp, shad, planeModel, textures);

		tiles[SW] = new Tile(t3->position + position[W] + glm::vec2(eps_X, 0.0), scale, disp, shad, planeModel, textures);
		this->reset();
	}
}


TileController::~TileController()
{
}


void TileController::addColumn(int direction) {
	if (direction > 0) {
		int gridLenght = 9;
		for (int i = 0; i < gridLenght - 1; i++) {
			for (int j = 0; j < gridLenght; j++) {
				tiles[i + j * gridLenght] = tiles[i + 1 + j * gridLenght];
			}
		}

		int i = gridLenght - 1;
		for (int j = 0; j < gridLenght; j++) {
			glm::vec2 pos = (float)(j - gridLenght / 2)*position[S] + (float)(i - gridLenght / 2)*position[E];
			tiles[i + j*gridLenght] = new Tile(tiles[(gridLenght*gridLenght)/2 + 1]->position + pos, scale, disp, shad, planeModel, textures);
		}
	}
	else if (direction < 0) {

	}
}