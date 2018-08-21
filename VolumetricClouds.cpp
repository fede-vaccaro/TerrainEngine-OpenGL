#include "VolumetricClouds.h"



VolumetricClouds::VolumetricClouds(int SW, int SH, Camera * cam): SCR_WIDTH(SW), SCR_HEIGHT(SH), camera(cam) {
	volumetricCloudsShader = new ScreenQuad("shaders/raymarch_cube.frag");
	ppShader = new ScreenQuad("shaders/cloud_post.frag");

	cloudsFBO = new FrameBufferObject(SW, SH);
	cloudsPostProcessingFBO = new FrameBufferObject(SW, SH);

	this->coverage = 0.18;

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
	comp.use();
	comp.setVec3("u_resolution", glm::vec3(32, 32, 32));
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

	cloudsPPShader.setSampler2D("cloudTEX", cloudsFBO->tex, 0);
	cloudsPPShader.setFloat("time", glfwGetTime());
	ScreenQuad::drawQuad();
}



VolumetricClouds::~VolumetricClouds()
{
}
