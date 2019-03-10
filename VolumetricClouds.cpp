#include "VolumetricClouds.h"

#define INT_CEIL(n,d) (int)ceil((float)n/d)

VolumetricClouds::VolumetricClouds(int SW, int SH): SCR_WIDTH(SW), SCR_HEIGHT(SH) {
	//volumetricCloudsShader = new ScreenQuad("shaders/volumetric_clouds.frag");
	volumetricCloudsShader = new Shader("volumetricCloudsShader","shaders/volumetric_clouds.comp");
	ppShader = new ScreenQuad("shaders/clouds_post.frag");

	//cloudsFBO = new FrameBufferObject(SW, SH, 4);
	cloudsFBO = new TextureSet(SW, SH, 4);
	cloudsPostProcessingFBO = new FrameBufferObject(Window::SCR_WIDTH, Window::SCR_HEIGHT, 2);

	postProcess = true;
	this->coverage = 0.45;

	/////////////////// TEXTURE GENERATION //////////////////

	//compute shaders
	Shader comp("perlinWorley");
	comp.attachShader("shaders/perlinworley.comp");
	comp.linkPrograms();

	//make texture
	this->perlinTex = Texture3D(128, 128, 128);

	//compute
	comp.use();
	comp.setVec3("u_resolution", glm::vec3(128, 128, 128));
	std::cout << "computing perlinworley!" << std::endl;
	glActiveTexture(GL_TEXTURE0);
	comp.setInt("outVolTex", 0);
	glDispatchCompute(INT_CEIL(128,4), INT_CEIL(128, 4), INT_CEIL(128, 4));
	std::cout << "computed!!" << std::endl;
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glGenerateMipmap(GL_TEXTURE_3D);

	//compute shaders
	Shader worley_git("worleyComp");
	worley_git.attachShader("shaders/worley.comp");
	worley_git.linkPrograms();

	//make texture
	this->worley32 = Texture3D(32, 32, 32);

	//compute
	worley_git.use();
	worley_git.setVec3("u_resolution", glm::vec3(32, 32, 32));
	std::cout << "computing worley 32!" << std::endl;
	glDispatchCompute(INT_CEIL(32, 4), INT_CEIL(32, 4), INT_CEIL(32, 4));
	std::cout << "computed!!" << std::endl;
	glGenerateMipmap(GL_TEXTURE_3D);

	////////////////////////
	//compute shaders
	Shader weather("weatherMap");
	weather.attachShader("shaders/weather.comp");
	weather.linkPrograms();

	//make texture
	this->weatherTex = Texture2D(1024, 1024);

	//compute
	weather.use();
	std::cout << "computing weather!" << std::endl;
	glDispatchCompute(INT_CEIL(1024, 8), INT_CEIL(1024, 8),1);
	std::cout << "weather computed!!" << std::endl;

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
#define TIMETO(CODE, TASK) 	t1 = glfwGetTime(); CODE; t2 = glfwGetTime(); std::cout << "Time to " + std::string(TASK) + " :" << (t2 - t1)*1e3 << "ms" << std::endl;


void VolumetricClouds::draw() {

	float t1, t2;

	//cloudsFBO->bind();
	for (int i = 0; i < cloudsFBO->getNTextures(); ++i) {
		bindTexture2D(cloudsFBO->getColorAttachmentTex(i), i);
	}

	//Shader & cloudsShader = volumetricCloudsShader->getShader();
	Shader & cloudsShader = *volumetricCloudsShader;
	sceneElements* s = drawableObject::scene;

	cloudsShader.use();
	cloudsShader.setVec2("iResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
	cloudsShader.setFloat("iTime", glfwGetTime());
	cloudsShader.setMat4("inv_proj", glm::inverse(s->projMatrix));
	cloudsShader.setMat4("inv_view", glm::inverse(s->cam.GetViewMatrix()));
	cloudsShader.setVec3("cameraPosition", s->cam.Position);
	cloudsShader.setFloat("FOV", s->cam.Zoom);
	cloudsShader.setVec3("lightDirection", glm::normalize(s->lightPos - s->cam.Position));
	cloudsShader.setVec3("lightColor", s->lightColor);
	cloudsShader.setFloat("coverage_multiplier", coverage);
	glm::mat4 vp = s->projMatrix*s->cam.GetViewMatrix();

	cloudsShader.setMat4("invViewProj", glm::inverse(vp));
	cloudsShader.setMat4("gVP", vp);

	cloudsShader.setSampler3D("cloud", this->perlinTex, 0);
	cloudsShader.setSampler3D("worley32", this->worley32, 1);
	cloudsShader.setSampler2D("weatherTex", this->weatherTex, 2);
	cloudsShader.setSampler2D("depthMap", s->sceneFBO.depthTex, 3);

	//actual draw
	//volumetricCloudsShader->draw();
	glDispatchCompute(INT_CEIL(SCR_WIDTH, 16), INT_CEIL(SCR_HEIGHT, 16), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	
	//copy to lastFrameFBO

	if (postProcess) {
		// cloud post processing filtering
		cloudsPostProcessingFBO->bind();
		Shader& cloudsPPShader = ppShader->getShader();

		cloudsPPShader.use();

		cloudsPPShader.setSampler2D("clouds", cloudsFBO->getColorAttachmentTex(0), 0);
		cloudsPPShader.setSampler2D("emissions", cloudsFBO->getColorAttachmentTex(1), 1);
		cloudsPPShader.setSampler2D("depthMap", s->sceneFBO.depthTex, 2);

		cloudsPPShader.setVec2("cloudRenderResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
		cloudsPPShader.setVec2("resolution", glm::vec2(Window::SCR_WIDTH , Window::SCR_HEIGHT));

		glm::mat4 lightModel;
		lightModel = glm::translate(lightModel, s->lightPos);
		glm::vec4 pos = vp * lightModel * glm::vec4(0.0, 60.0, 0.0, 1.0);
		pos = pos / pos.w;
		pos = pos * 0.5f + 0.5f;

		//std::cout << pos.x << ": X; " << pos.y << " Y;" << std::endl;
		cloudsPPShader.setVec4("lightPos", pos);

		bool isLightInFront = false;
		float lightDotCameraFront = glm::dot(glm::normalize(s->lightPos - s->cam.Position), glm::normalize(s->cam.Front));
		//std::cout << "light dot camera front= " << lightDotCameraFront << std::endl;
		if (lightDotCameraFront > 0.2) {
			isLightInFront = true;
		}

		cloudsPPShader.setBool("isLightInFront", isLightInFront);
		cloudsPPShader.setFloat("lightDotCameraFront", lightDotCameraFront);

		cloudsPPShader.setFloat("time", glfwGetTime());
		ppShader->draw();
	}
}



VolumetricClouds::~VolumetricClouds()
{
}
