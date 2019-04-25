#include "Terrain.h"
#include "sceneElements.h"
#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"
#include "../Engine/utils.h"

bool Terrain::drawFog = true;

float sign(float x) {
	if (x > 0.0f) return 1.0f;
	else if (x < 0.0f) { return -1.0f; }
	else return 0.0f;
}


Terrain::Terrain(int gl)
{


	glm::mat4 id;
	glm::mat4 scaleMatrix = glm::scale(id, glm::vec3(1.0, 0.0, 1.0));
	glm::mat4 positionMatrix = glm::translate(id, glm::vec3(0., 0.0, 0.));
	modelMatrix = positionMatrix;

	octaves = 13;
	frequency = 0.01;
	grassCoverage = 0.65;
	tessMultiplier = 1.0;
	dispFactor = 20.0;

	fogFalloff = 1.5;

	posBuffer = 0;

	shad = new Shader("TerrainTessShader");
	shad->attachShader("shaders/terrain.vert")
		->attachShader("shaders/terrain.tcs")
		->attachShader("shaders/terrain.tes")
		->attachShader("shaders/terrain.frag")
		->linkPrograms();

	this->gridLength = gl + (gl + 1) % 2; //ensure gridLength is odd


	res = 4;
	initializePlaneVAO(res, tileW, &planeVAO, &planeVBO, &planeEBO);

	//load a bunch of textures
	this->textures = new unsigned int[6];
	textures[0] = TextureFromFile("sand.jpg", "resources");
	textures[1] = TextureFromFile("grass.jpg", "resources");
	textures[2] = TextureFromFile("rdiffuse.jpg", "resources");
	textures[3] = TextureFromFile("snow2.jpg", "resources");
	textures[4] = TextureFromFile("rnormal.jpg", "resources");
	textures[5] = TextureFromFile("terrainTexture.jpg", "resources");

	positionVec.resize(gridLength*gridLength);
	generateTileGrid(glm::vec2(0.0,0.0));

	setPositionsArray(positionVec);

	rockColor = glm::vec4(120, 105, 75, 255)*1.5f / 255.f;
	power = 3.0;
}

void Terrain::generateTileGrid(glm::vec2 offset)
{
	float sc = tileW;

	glm::vec2 I = glm::vec2(1, 0)*sc;
	glm::vec2 J = glm::vec2(0, 1)*sc;

	for (int i = 0; i < gridLength; i++) {
		for (int j = 0; j < gridLength; j++) {
			glm::vec2 pos = (float)(j - gridLength / 2)*glm::vec2(I) + (float)(i - gridLength / 2)*glm::vec2(J);
			setPos(i, j, pos + offset);
		}
	}
}

void Terrain::deleteBuffer(){
	glDeleteBuffers(1, &posBuffer);
	posBuffer = 0;
}

bool Terrain::getWhichTileCameraIs(glm::vec2& result) {

	for (glm::vec2 p : positionVec) {
		if (inTile(*(scene->cam), p)) {
			//std::cout << "You're in Tile: " << p.x << ", " << p.y << std::endl;
			result = p;
			return true;
		}
	}
	return false;
}


void Terrain::draw(){


	sceneElements* se = drawableObject::scene;

	drawFog = !se->wireframe;

	if (up != 0.0f) {
		glEnable(GL_CLIP_DISTANCE0);
	}
	glm::mat4 gWorld = modelMatrix;
	glm::mat4 gVP = se->projMatrix * se->cam->GetViewMatrix();

	shad->use();
	shad->setVec3("gEyeWorldPos", se->cam->Position);
	shad->setMat4("gWorld", gWorld);
	shad->setMat4("gVP", gVP);
	shad->setFloat("gDispFactor", dispFactor);

	float waterHeight = (waterPtr ? waterPtr->getModelMatrix()[3][1] : 100.0);
	glm::vec4 clipPlane(0.0, 1.0, 0.0, -waterHeight);
	shad->setVec4("clipPlane", clipPlane*up);
	shad->setVec3("u_LightColor", se->lightColor);
	shad->setVec3("u_LightPosition", se->lightPos);
	shad->setVec3("u_ViewPosition", se->cam->Position);
	shad->setVec3("fogColor", se->fogColor);
	shad->setVec3("rockColor", rockColor);
	shad->setVec3("seed", se->seed);

	shad->setInt("octaves", octaves);
	shad->setFloat("freq", frequency);
	shad->setFloat("u_grassCoverage", grassCoverage);
	shad->setFloat("waterHeight", waterHeight);
	shad->setFloat("tessMultiplier", tessMultiplier);
	shad->setFloat("fogFalloff", fogFalloff*1.e-6);
	shad->setFloat("power", power);

	shad->setBool("normals", true);
	shad->setBool("drawFog", Terrain::drawFog);


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

	drawVertices(nIstances);


	glDisable(GL_CLIP_DISTANCE0);
	up = 0.0;
}

void Terrain::setGui()
{
	ImGui::Begin("Terrain controls: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Terrain Controls");
	//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	//ImGui::Checkbox("Clouds PostProc + God Rays", this->getPostProcPointer());
	ImGui::SliderInt("Octaves", &octaves, 1, 20);
	ImGui::SliderFloat("Frequency",&frequency, 0.0f, 0.05f);
	ImGui::SliderFloat("Displacement factor", &dispFactor, 0.0f, std::pow(32.f*32.f*32.f, 1/power));
	ImGui::SliderFloat("Grass coverage", &grassCoverage, 0.0f, 1.f);
	ImGui::SliderFloat("Tessellation multiplier", &tessMultiplier, 0.1f, 5.f);
	ImGui::SliderFloat("Fog fall-off", &fogFalloff, 0.0f, 10.);
	ImGui::SliderFloat("Power", &power, 0.0f, 10.);

	//glm::vec3 * cloudBottomColor = this->getCloudColorBottomPtr();
	//ImGui::ColorEdit3("Cloud color", (float*)cloudBottomColor); // Edit 3 floats representing a color

	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sky controls");
	ImGui::ColorEdit3("Rock color", (float*)&rockColor[0]); // Edit 3 floats representing a color
	//ImGui::ColorEdit3("Sky bottom color", (float*)this->getSkyBottomColorPtr()); // Edit 3 floats representing a color
	ImGui::End();
}

void Terrain::drawVertices(int nInstances) {
	glBindVertexArray(planeVAO);
	//shader.use();
	shad->use();
	glDrawElementsInstanced(GL_PATCHES, (res-1)*(res-1)*2*3, GL_UNSIGNED_INT, 0, nInstances);
	glBindVertexArray(0);
}

void Terrain::setPositionsArray(std::vector<glm::vec2> & pos) {
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

bool Terrain::inTile(const Camera camera, glm::vec2 pos) {
	float camX = camera.Position.x;
	float camY = camera.Position.z;

	float x = pos.x;
	float y = pos.y;

	bool inX = false;
	if ((camX <= x + 1.0 * tileW/2.0f) && (camX >= x - 1.0 * tileW/2.0f)) { inX = true; }
	bool inY = false;
	if ((camY <= y + 1.0 * tileW/2.0f) && (camY >= y - 1.0 * tileW/2.0f)) { inY = true; }

	bool result = inX && inY;

	if (result) {

		//std::cout << y << " :y" << std::endl;
		//std::cout << y << " :y" << std::endl;

		//std::cout << y + scaleFactor * tileW / 2.0f << ": y + scalefactor" << std::endl;
		//std::cout << y - scaleFactor * tileW / 2.0f << ": y - scalefactor" << std::endl;
	}

	return result;

}


Terrain::~Terrain()
{

}

void Terrain::updateTilesPositions() {
	sceneElements* se = drawableObject::scene;
	glm::vec2 camPosition(se->cam->Position.x, se->cam->Position.z);
	int whichTile = -1;
	int howManyTiles = 0;

	glm::vec2 currentTile;
	if (getWhichTileCameraIs(currentTile)) {
		glm::vec2 center = getPos(gridLength / 2, gridLength / 2);
		for (glm::vec2& p : positionVec) {
			p += currentTile - center;
		}
		setPositionsArray(positionVec);

		if (waterPtr) {
			glm::vec2 center = getPos(gridLength / 2, gridLength / 2);
			waterPtr->setPosition(center, 1.0*gridLength, waterPtr->getHeight());
		}
	}
}


void Terrain::reset() {
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
