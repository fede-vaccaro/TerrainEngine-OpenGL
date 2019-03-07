#include "Water.h"
#include "sceneElements.h"
#include "Tile.h"

Water::Water(glm::vec2 position, float scale, float height): scale(scale), height(height){

	//shad = new Shader("shaders/waterVertexShader.vert", "shaders/waterFragmentShader.frag");
	shad = new Shader("WaterShader");
	shad->attachShader("shaders/waterVertexShader.vert");
	shad->attachShader("shaders/waterFragmentShader.frag");
	shad->linkPrograms();
	std::cout << "============= CREATING WaterSHADER ==============" << std::endl;
	//waterPlane = new Model("resources/plane.obj", GL_TRIANGLES);

	//dudvMap = TextureFromFile("waterDUDV.png", "resources", false);
	//normalMap = TextureFromFile("normalMap.png", "resources", false);

	//height = 128.0 + 50.0;

	glm::mat4 identity;
	glm::mat4 scaleMatrix = glm::scale(identity, glm::vec3(scale, scale, scale));
	glm::mat4 transMatrix = glm::translate(identity, glm::vec3(position.x, height, position.y));
	this->modelMatrix = transMatrix * scaleMatrix;

	reflectionFBO = new FrameBufferObject(FBOw, FBOh);
	refractionFBO = new FrameBufferObject(FBOw, FBOh);

	initializePlaneVAO(2, Tile::tileW, &planeVAO, &planeVBO, &planeEBO);
}

void Water::bindReflectionFBO() {
	reflectionFBO->bind();
}

void Water::bindRefractionFBO() {
	refractionFBO->bind();
}

const int res = 2;

void Water::drawVertices() {
	glBindVertexArray(planeVAO);
	//shader.use();
	shad->use();
	glDrawElements(GL_TRIANGLES, (res - 1)*(res - 1) * 2 * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

const float tileW = 5.0;

/*
void Water::initializePlaneVAO() {

	//const int res = 3;
	const int nPoints = res * res;
	const int size = nPoints * 3 + nPoints * 3 + nPoints * 2;
	float * vertices = new float[size];
	for (int i = 0; i < res; i++) {
		for (int j = 0; j < res; j++) {
			//add position
			float x = j * (float)tileW / (res - 1) - tileW / 2.0;
			float y = 0.0;
			float z = -i * (float)tileW / (res - 1) + tileW / 2.0;

			vertices[(i + j * res) * 8] = x; //8 = 3 + 3 + 2, float per point
			vertices[(i + j * res) * 8 + 1] = y;
			vertices[(i + j * res) * 8 + 2] = z;

			//add normal
			float x_n = 0.0;
			float y_n = 1.0;
			float z_n = 0.0;

			vertices[(i + j * res) * 8 + 3] = x_n;
			vertices[(i + j * res) * 8 + 4] = y_n;
			vertices[(i + j * res) * 8 + 5] = z_n;

			//add texcoords
			vertices[(i + j * res) * 8 + 6] = (float)j / (res - 1);
			vertices[(i + j * res) * 8 + 7] = (float)(res - i - 1) / (res - 1);
		}
	}

	const int nTris = (res - 1)*(res - 1) * 2;
	int * trisIndices = new int[nTris * 3];

	for (int i = 0; i < nTris; i++) {
		int trisPerRow = 2 * (res - 1);
		for (int j = 0; j < trisPerRow; j++) {
			if (!(i % 2)) { //upper triangle
				int k = i * 3;
				int triIndex = i % trisPerRow;

				int row = i / trisPerRow;
				int col = triIndex / 2;
				trisIndices[k] = row * res + col;
				trisIndices[k + 1] = ++row*res + col;
				trisIndices[k + 2] = --row* res + ++col;
			}
			else {
				int k = i * 3;
				int triIndex = i % trisPerRow;

				int row = i / trisPerRow;
				int col = triIndex / 2;
				trisIndices[k] = row * res + ++col;
				trisIndices[k + 1] = ++row * res + --col;
				trisIndices[k + 2] = row * res + ++col;
			}
		}
	}


	for (int i = 0; i < res; i++) {
		for (int j = 0; j < res; j++) {
			for (int k = 0; k < 8; k++) {
				if (k == 3 || k == 6)
					std::cout << std::endl;
				std::cout << vertices[(i + j * res) * 8 + k] << ", ";
			}
			std::cout << std::endl;
		}
	}

	for (int i = 0; i < nTris; i++) {
		for (int k = 0; k < 3; k++)
		{
			std::cout << trisIndices[i * 3 + k] << ", ";
		}
		std::cout << std::endl;

	}

	std::cout << "TRISINDICES: " << nTris * 3 << std::endl;

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTris * 3 * sizeof(unsigned int), trisIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}
*/

void Water::draw() {
	// draw water plane
	shad->use();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	sceneElements * se = drawableObject::scene;

	this->setPosition(glm::vec2(se->cam.Position[0], se->cam.Position[2]), scale, height);

	shad->setMat4("modelMatrix", modelMatrix);
	shad->setMat4("gVP", se->projMatrix*se->cam.GetViewMatrix());

	shad->setVec3("u_LightColor", se->lightColor);
	shad->setVec3("u_LightPosition", se->lightPos);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reflectionFBO->tex);
	shad->setInt("reflectionTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, refractionFBO->tex);
	shad->setInt("refractionTex", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dudvMap);
	shad->setInt("waterDUDV", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	shad->setInt("normalMap", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, refractionFBO->depthTex);
	shad->setInt("depthMap", 4);

	float waveSpeed = 0.25;
	float time = glfwGetTime();

	float moveFactor = waveSpeed * time;
	shad->setFloat("moveFactor", moveFactor);

	shad->setVec3("cameraPosition", se->cam.Position);

	//waterPlane->Draw(*shad);
	this->drawVertices();
	glDisable(GL_BLEND);
}

void Water::unbindFBO() {
	unbindCurrentFrameBuffer(Window::SCR_WIDTH, Window::SCR_WIDTH);
}



Water::~Water()
{
}
