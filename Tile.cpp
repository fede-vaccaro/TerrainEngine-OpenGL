#include "Tile.h"
#include <GLFW/glfw3.h>

bool Tile::drawFog = true;

float sign(float x) {
	if (x > 0.0f) return 1.0f;
	else if (x < 0.0f) { return -1.0f; }
	else return 0.0f;
}

//Model * Tile::planeModel = new Model("resources/plane.obj", GL_PATCHES);

Tile::Tile(glm::vec2 position, float scale, float dispFactor, TessellationShader * shad, TerrainGenerator * tg, Model * planeModel, unsigned int * textures) : dispFactor(dispFactor), scaleFactor(scale), shad(shad), tg(tg), position(position), planeModel(planeModel), textures(textures)
{
	glm::mat4 id;
	glm::mat4 scaleMatrix = glm::scale(id, glm::vec3(scale, 0.0, scale));
	float eps_x = 0.0f, eps_y = 0.0f;
	if (abs(position.x) > 0.0f) eps_x = -sign(position.x)*0.02f*scale*3.0f;
	if (abs(position.y) > 0.0f) eps_y = -sign(position.y)*0.02*scale*3.0;

	eps = glm::vec2(eps_x, eps_y);

	//std::cout << eps.y << " : eps_y" << std::endl;

	//position.x += eps_x;
	//position.y += eps_y;

	glm::mat4 positionMatrix = glm::translate(id, glm::vec3(position.x + eps.x, 0.0, position.y + eps.y));
	modelMatrix = positionMatrix * scaleMatrix;

	float t1 = glfwGetTime();

	heightMap = tg->generateHeightMap((position+eps)*(1.0f/(scale*tileW)));
	float t2 = glfwGetTime();

	//load texture
	//sand = TextureFromFile("sand.jpg", "resources", false);
	//rock = TextureFromFile("rock.jpg", "resources", false);
	//grass = TextureFromFile("grass.jpg", "resources", false);
	//snow = TextureFromFile("snow.jpg", "resources", false);

	std::cout << "heightmap generated in : " << t2 - t1 << "s" << std::endl;
}

void Tile::drawTile(Camera * camera, glm::mat4 proj, glm::vec3 lightPosition, glm::vec3 lightColor, glm::vec3 fogColor, float waterHeight, float up) {

	if (up != 0.0f) {
		glEnable(GL_CLIP_DISTANCE0);
	}
	glm::mat4 gWorld = modelMatrix;
	glm::mat4 gVP = proj * camera->GetViewMatrix();

	shad->use();
	shad->setVec3("gEyeWorldPos", camera->Position);
	shad->setMat4("gWorld", gWorld);
	shad->setMat4("gVP", gVP);
	shad->setFloat("gDispFactor", dispFactor);
	float correction = 0.0f;
	if (up < 0.0f) correction = 0.050f;
	glm::vec4 clipPlane(0.0, 1.0, 0.0, -waterHeight -correction);
	shad->setVec4("clipPlane", clipPlane*up);
	shad->setVec3("u_LightColor", lightColor);
	shad->setVec3("u_LightPosition", lightPosition);
	shad->setVec3("u_ViewPosition", camera->Position);
	shad->setVec3("fogColor", fogColor);

	shad->setBool("drawFog", Tile::drawFog);

	// set textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightMap);
	shad->setInt("gDisplacementMap", 0);
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

	planeModel->Draw(*shad);
	//glBindVertexArray(VAO);
	//glDrawArrays(GL_PATCHES, 0, 6);
	glDisable(GL_CLIP_DISTANCE0);

}

bool Tile::inTile(Camera camera) {
	float camX = camera.Position.x;
	float camY = camera.Position.z;

	float x = position.x;
	float y = position.y;

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

void Tile::setWater(Water * w) {
	this->water = w;
}


Tile::~Tile()
{

}
