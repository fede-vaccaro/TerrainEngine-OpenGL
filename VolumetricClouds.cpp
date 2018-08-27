#include "VolumetricClouds.h"



VolumetricClouds::VolumetricClouds(int SW, int SH, Camera * cam): SCR_WIDTH(SW), SCR_HEIGHT(SH), camera(cam) {
	volumetricCloudsShader = new ScreenQuad("shaders/raymarch_cube.frag");
	ppShader = new ScreenQuad("shaders/cloud_post.frag");

	cloudsFBO = new FrameBufferObject(SW, SH, 2);
	cloudsPostProcessingFBO = new FrameBufferObject(SW, SH, 2);

	this->coverage = 0.4;

	/////////////////// TEXTURE GENERATION //////////////////

	//compute shaders
	ComputeShader comp("shaders/perlinworley.comp");

	//make texture
	this->perlinTex = Texture3D(128, 128, 128);

	//compute
	comp.use();
	comp.setVec3("u_resolution", glm::vec3(128, 128, 128));
	std::cout << "computing perlinworley!" << std::endl;
	glActiveTexture(GL_TEXTURE0);
	comp.setInt("outVolTex", 0);
	glDispatchCompute((GLuint)128, (GLuint)128, (GLuint)128);
	std::cout << "computed!!" << std::endl;

	//compute shaders
	ComputeShader worley_git("shaders/worley.comp");

	//make texture
	this->worley32 = Texture3D(32, 32, 32);

	//compute
	worley_git.use();
	worley_git.setVec3("u_resolution", glm::vec3(32, 32, 32));
	std::cout << "computing worley 32!" << std::endl;
	glDispatchCompute((GLuint)32, (GLuint)32, (GLuint)32);
	std::cout << "computed!!" << std::endl;

	////////////////////////
	//compute shaders
	ComputeShader weather("shaders/weather.comp");

	//make texture
	this->weatherTex = Texture2D(1024, 1024);

	//compute
	weather.use();
	std::cout << "computing weather!" << std::endl;
	glDispatchCompute((GLuint)1024, (GLuint)1024, (GLuint)1);
	std::cout << "weather computed!!" << std::endl;

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


void VolumetricClouds::draw(glm::mat4 view, glm::mat4 proj, glm::vec3 lightPosition, unsigned int depthMapTex) {

	cloudsFBO->bind();
	Shader & cloudsShader = volumetricCloudsShader->getShader();

	cloudsShader.use();
	cloudsShader.setVec2("iResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
	cloudsShader.setFloat("iTime", glfwGetTime());
	cloudsShader.setMat4("inv_proj", glm::inverse(proj));
	cloudsShader.setMat4("inv_view", glm::inverse(camera->GetViewMatrix()));
	cloudsShader.setVec3("cameraPosition", camera->Position);
	cloudsShader.setFloat("FOV", camera->Zoom);
	cloudsShader.setVec3("lightPosition", lightPosition);
	cloudsShader.setFloat("coverage_multiplier", coverage);

	cloudsShader.setSampler3D("cloud", this->perlinTex, 0);
	cloudsShader.setSampler3D("worley32", this->worley32, 1);
	cloudsShader.setSampler2D("weatherTex", this->weatherTex, 2);
	cloudsShader.setSampler2D("depthMap", depthMapTex, 3);


	//actual draw
	ScreenQuad::drawQuad();
	
	// cloud post processing filtering
	cloudsPostProcessingFBO->bind();
	Shader& cloudsPPShader = ppShader->getShader();

	cloudsPPShader.use();

	cloudsPPShader.setSampler2D("clouds", cloudsFBO->getColorAttachmentTex(0), 0);
	cloudsPPShader.setSampler2D("emissions", cloudsFBO->getColorAttachmentTex(1), 1);
	cloudsPPShader.setSampler2D("depthMap", depthMapTex, 2);
	cloudsPPShader.setVec2("resolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

	glm::mat4 lightModel;
	lightModel = glm::translate(lightModel, lightPosition);
	glm::vec4 pos = proj * view* lightModel * glm::vec4(0.0, 0.0, 0.0, 1.0);
	pos = pos / pos.w;
	pos = pos * 0.5f + 0.5f;

	//std::cout << pos.x << ": X; " << pos.y << " Y;" << std::endl;
	cloudsPPShader.setVec4("lightPos", pos);

	bool isLightInFront = false;
	float lightDotCameraFront = glm::dot(glm::normalize(lightPosition - camera->Position), glm::normalize(camera->Front));
	//std::cout << "light dot camera front= " << lightDotCameraFront << std::endl;
	if (lightDotCameraFront > 0.2) {
		isLightInFront = true;
	}

	cloudsPPShader.setBool("isLightInFront", isLightInFront);
	cloudsPPShader.setFloat("lightDotCameraFront", lightDotCameraFront);

	cloudsPPShader.setFloat("time", glfwGetTime());
	ScreenQuad::drawQuad();
	
}



VolumetricClouds::~VolumetricClouds()
{
}
