#include "TileController.h"
#include <GLFW/glfw3.h>


TileController::TileController(float scale, float disp, Camera * camera, TessellationShader * shad, Shader * waterShader, TerrainGenerator * tg) : scale(scale), disp(disp), camera(camera), shad(shad), tg(tg), waterShader(waterShader)
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
	textures[2] = TextureFromFile("rock.jpg", "resources", false);
	textures[3] = TextureFromFile("snow.jpg", "resources", false);

	
	dudvMap = TextureFromFile("waterDUDV.png", "resources", false);
	normalMap = TextureFromFile("normalMap.png", "resources", false);

	waterHeight = disp / 2.5;

	for (int i = 0; i < totTiles; i++) {
		tiles[i] = new Tile(position[i], scale, disp, shad, tg, planeModel, textures);
		//tiles[i]->setWater(new Water(tiles[i]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));
	}
	tiles[C]->setWater(new Water(tiles[C]->position, waterShader, scale*3.0, waterHeight, dudvMap, normalMap, waterModel));

};

void TileController::drawTiles(glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor) {
	
	Water * waterPtr = tiles[C]->water;

	// reflection
	waterPtr->bindReflectionFBO();
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->invertPitch();
	camera->Position.y -= 2 * (camera->Position.y - waterHeight);

	for (int j = 0; j < tiles.size(); j++) {
		tiles[j]->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, 1.0f);
	}
	camera->invertPitch();
	camera->Position.y += 2 * abs(camera->Position.y - waterHeight);
	waterPtr->unbindFBO();

	// refraction
	waterPtr->bindRefractionFBO();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int j = 0; j < tiles.size(); j++) {
		tiles[j]->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, -1.0f);
	};
	waterPtr->unbindFBO();

	for (int i = 0; i < tiles.size(); i++)
	{
		tiles[i]->drawTile(camera, proj, lightPosition, lightColor, fogColor, waterHeight, 0.0f);
	};
	waterPtr->draw(proj* (camera->GetViewMatrix()), lightPosition, lightColor, camera->Position);

}

void TileController::updateTiles() {
	glm::vec2 camPosition(camera->Position.x, camera->Position.z);
	int whichTile = -1;
	int howManyTiles = 0;
	bool found = false;
	for (int i = 0; (i < totTiles) && !found; i++) {
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

void TileController::changeTiles(tPosition currentTile) {
	if (currentTile == N) {

		std::cout << "CHANGING TILES: NORTH" << std::endl;

		Water * waterPtr = tiles[C]->water;
		tiles[C]->water = 0;


		tiles[SE] = tiles[E];
		tiles[SW] = tiles[W];
		tiles[S] = tiles[C];

		tiles[E] = tiles[NE]; //to replace 
		tiles[C] = tiles[N]; //to replace
		tiles[W] = tiles[NW]; //to replace

		tiles[C]->water = waterPtr;
		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[NE], *t2 = tiles[N], *t3 = tiles[NW];

		float eps_y = 0.02f*scale*3.0f;


		tiles[NE] = new Tile(t1->position + position[N] + glm::vec2(0.0, eps_y), scale, disp, shad, tg, planeModel, textures);
		//tiles[NE]->setWater(new Water(tiles[NE]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[N] = new Tile(t2->position + position[N] + glm::vec2(0.0, eps_y), scale, disp, shad, tg, planeModel, textures);
		//tiles[N]->setWater(new Water(tiles[N]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[NW] = new Tile(t3->position + position[N] + glm::vec2(0.0, eps_y), scale, disp, shad, tg, planeModel, textures);
		//tiles[NW]->setWater(new Water(tiles[NW]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

	}
	else if (currentTile == S) {
		std::cout << "CHANGING TILES: SOUTH" << std::endl;

		Water * waterPtr = tiles[C]->water;
		tiles[C]->water = 0;

		tiles[NE] = tiles[E];
		tiles[N] = tiles[C];
		tiles[NW] = tiles[W];


		tiles[E] = tiles[SE]; //SE to replace 
		tiles[C] = tiles[S]; //S to replace
		tiles[W] = tiles[SW]; //SW to replace

		tiles[C]->water = waterPtr;
		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[SE], *t2 = tiles[S], *t3 = tiles[SW];

		float eps_y = -0.02f*scale*3.0f;

		tiles[SE] = new Tile(t1->position + position[S] + glm::vec2(0.0, eps_y), scale, disp, shad, tg, planeModel, textures);
		//tiles[SE]->setWater(new Water(tiles[SE]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[S] = new Tile(t2->position + position[S] + glm::vec2(0.0, eps_y), scale, disp, shad, tg, planeModel, textures);
		//tiles[S]->setWater(new Water(tiles[S]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[SW] = new Tile(t3->position + position[S] + glm::vec2(0.0, eps_y), scale, disp, shad, tg, planeModel, textures);
		//tiles[SW]->setWater(new Water(tiles[SW]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

	}
	else if (currentTile == E) {
		std::cout << "CHANGING TILES: EAST" << std::endl;

		Water * waterPtr = tiles[C]->water;
		tiles[C]->water = 0;

		tiles[NW] = tiles[N];
		tiles[W] = tiles[C];
		tiles[SW] = tiles[S];

		tiles[N] = tiles[NE]; //NE to repleace
		tiles[C] = tiles[E]; //E to repleace
		tiles[S] = tiles[SE]; //SE to repleace

		tiles[C]->water = waterPtr;
		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[NE], *t2 = tiles[E], *t3 = tiles[SE];

		float eps_X = -0.02f*scale*3.0f;

		tiles[NE] = new Tile(t1->position + position[E] + glm::vec2(eps_X, 0.0), scale, disp, shad, tg, planeModel, textures);
		//tiles[NE]->setWater(new Water(tiles[NE]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[E] = new Tile(t2->position + position[E] + glm::vec2(eps_X, 0.0), scale, disp, shad, tg, planeModel, textures);
		//tiles[E]->setWater(new Water(tiles[E]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[SE] = new Tile(t3->position + position[E] + glm::vec2(eps_X, 0.0), scale, disp, shad, tg, planeModel, textures);
		//tiles[SE]->setWater(new Water(tiles[SE]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

	}
	else if (currentTile == W) {
		std::cout << "CHANGING TILES: WEST" << std::endl;

		Water * waterPtr = tiles[C]->water;
		tiles[C]->water = 0;

		tiles[NE] = tiles[N];
		tiles[E] = tiles[C];
		tiles[SE] = tiles[S];

		tiles[N] = tiles[NW]; //NW to repleace
		tiles[C] = tiles[W]; //W to repleace
		tiles[S] = tiles[SW]; //SW to repleace

		tiles[C]->water = waterPtr;
		waterPtr->setPosition(tiles[C]->position + tiles[C]->eps, scale*3.0, waterHeight);

		Tile * t1 = tiles[NW], *t2 = tiles[W], *t3 = tiles[SW];

		float eps_X = 0.02f*scale*3.0f;

		tiles[NW] = new Tile(t1->position + position[W] + glm::vec2(eps_X, 0.0), scale, disp, shad, tg, planeModel, textures);
		//tiles[NW]->setWater(new Water(tiles[NW]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[W] = new Tile(t2->position + position[W] + glm::vec2(eps_X, 0.0), scale, disp, shad, tg, planeModel, textures);
		//tiles[W]->setWater(new Water(tiles[W]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

		tiles[SW] = new Tile(t3->position + position[W] + glm::vec2(eps_X, 0.0), scale, disp, shad, tg, planeModel, textures);
		//tiles[SW]->setWater(new Water(tiles[SW]->position, waterShader, scale, disp / 2.0, dudvMap, normalMap, waterModel));

	}

}


TileController::~TileController()
{
}
