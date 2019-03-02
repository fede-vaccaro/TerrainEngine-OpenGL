#include "Water.h"
#include "sceneElements.h"

Water::Water(glm::vec2 position, float scale, float height): scale(scale), height(height){

	shad = new Shader("shaders/waterVertexShader.vert", "shaders/waterFragmentShader.frag");
	std::cout << "============= CREATING TSHADER ==============" << std::endl;
	waterPlane = new Model("resources/plane.obj", GL_TRIANGLES);

	dudvMap = TextureFromFile("waterDUDV.png", "resources", false);
	normalMap = TextureFromFile("normalMap.png", "resources", false);

	//height = 128.0 + 50.0;

	glm::mat4 identity;
	glm::mat4 scaleMatrix = glm::scale(identity, glm::vec3(scale, scale, scale));
	glm::mat4 transMatrix = glm::translate(identity, glm::vec3(position.x, height, position.y));
	this->modelMatrix = transMatrix * scaleMatrix;

	reflectionFBO = new FrameBufferObject(FBOw, FBOh);
	refractionFBO = new FrameBufferObject(FBOw, FBOh);
}

void Water::bindReflectionFBO() {
	reflectionFBO->bind();
}

void Water::bindRefractionFBO() {
	refractionFBO->bind();
}
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

	waterPlane->Draw(*shad);
	glDisable(GL_BLEND);
}

void Water::unbindFBO() {
	unbindCurrentFrameBuffer(Window::SCR_WIDTH, Window::SCR_WIDTH);
}



Water::~Water()
{
}
