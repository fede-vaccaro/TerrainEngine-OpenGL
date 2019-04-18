#include "Skybox.h"

#include "sceneElements.h"

Skybox::Skybox()
{
	//shader = new Shader(, );
	shader = new Shader("SkyboxShader");
	shader->attachShader(BaseShader("shaders/skyboxVert.vert"))
		->attachShader(BaseShader("shaders/skyboxFrag.frag"))
		->linkPrograms();
	
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	model = glm::translate(model, glm::vec3(0.0, -0.1, 0.0));

	/*
		vector<std::string> faces =
	{
		"resources/skybox/left.jpg",
		"resources/skybox/right.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);*/

}

void Skybox::draw() {
	sceneElements * s = drawableObject::scene;

	glm::mat4 view = s->cam.GetViewMatrix();
	glm::mat4 proj = s->projMatrix;
	glDepthFunc(GL_LEQUAL);
	shader->use();
	glm::mat4 view2 = glm::mat4(glm::mat3(view)); // remove translation part from the view matrix
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0, -0.1, 0.0));
	shader->setMat4("model", model);
	shader->setMat4("view", view2);
	shader->setMat4("projection", proj);
	glBindVertexArray(skyboxVAO);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}


Skybox::~Skybox()
{
}
