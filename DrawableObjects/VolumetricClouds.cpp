#include "VolumetricClouds.h"

#define TIMETO(CODE, TASK) 	t1 = glfwGetTime(); CODE; t2 = glfwGetTime(); std::cout << "Time to " + std::string(TASK) + " :" << (t2 - t1)*1e3 << "ms" << std::endl;



VolumetricClouds::VolumetricClouds(int SW, int SH, CloudsModel * model): SCR_WIDTH(SW), SCR_HEIGHT(SH), model(model) {

	cloudsFBO = new TextureSet(SW, SH, 4);
	cloudsPostProcessingFBO = new FrameBufferObject(Window::SCR_WIDTH, Window::SCR_HEIGHT, 2);

}


void VolumetricClouds::draw() {

	float t1, t2;

	//cloudsFBO->bind();
	for (int i = 0; i < cloudsFBO->getNTextures(); ++i) {
		bindTexture2D(cloudsFBO->getColorAttachmentTex(i), i);
	}

	//Shader & cloudsShader = volumetricCloudsShader->getShader();
	Shader & cloudsShader = *model->volumetricCloudsShader;
	sceneElements* s = drawableObject::scene;

	cloudsShader.use();

	cloudsShader.setVec2("iResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
	cloudsShader.setFloat("iTime", glfwGetTime());
	cloudsShader.setMat4("inv_proj", glm::inverse(s->projMatrix));
	cloudsShader.setMat4("inv_view", glm::inverse(s->cam->GetViewMatrix()));
	cloudsShader.setVec3("cameraPosition", s->cam->Position);
	cloudsShader.setFloat("FOV", s->cam->Zoom);
	cloudsShader.setVec3("lightDirection", glm::normalize(s->lightPos - s->cam->Position));
	cloudsShader.setVec3("lightColor", s->lightColor);
	
	cloudsShader.setFloat("coverage_multiplier", model->coverage);
	cloudsShader.setFloat("cloudSpeed", model->cloudSpeed);
	cloudsShader.setFloat("crispiness", model->crispiness);
	cloudsShader.setFloat("curliness", model->curliness);
	cloudsShader.setFloat("absorption", model->absorption*0.01);
	cloudsShader.setFloat("densityFactor", model->density);

	//cloudsShader.setBool("enablePowder", enablePowder);
	
	cloudsShader.setFloat("earthRadius", model->earthRadius);
	cloudsShader.setFloat("sphereInnerRadius", model->sphereInnerRadius);
	cloudsShader.setFloat("sphereOuterRadius", model->sphereOuterRadius);

	cloudsShader.setVec3("cloudColorTop", model->cloudColorTop);
	cloudsShader.setVec3("cloudColorBottom", model->cloudColorBottom);
	
	cloudsShader.setVec3("skyColorTop", model->sky->skyColorTop);
	cloudsShader.setVec3("skyColorBottom", model->sky->skyColorBottom);

	glm::mat4 vp = s->projMatrix*s->cam->GetViewMatrix();
	cloudsShader.setMat4("invViewProj", glm::inverse(vp));
	cloudsShader.setMat4("gVP", vp);

	cloudsShader.setSampler3D("cloud", model->perlinTex, 0);
	cloudsShader.setSampler3D("worley32", model->worley32, 1);
	cloudsShader.setSampler2D("weatherTex", model->weatherTex, 2);
	cloudsShader.setSampler2D("depthMap", s->sceneFBO->depthTex, 3);

	cloudsShader.setSampler2D("sky", model->sky->getSkyTexture(), 4);


	//actual draw
	//volumetricCloudsShader->draw();
	if(!s->wireframe)
		glDispatchCompute(INT_CEIL(SCR_WIDTH, 16), INT_CEIL(SCR_HEIGHT, 16), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	
	//copy to lastFrameFBO

	if (model->postProcess) {
		// cloud post processing filtering
		cloudsPostProcessingFBO->bind();
		Shader& cloudsPPShader = model->postProcessingShader->getShader();

		cloudsPPShader.use();

		cloudsPPShader.setSampler2D("clouds", cloudsFBO->getColorAttachmentTex(VolumetricClouds::fragColor), 0);
		cloudsPPShader.setSampler2D("emissions", cloudsFBO->getColorAttachmentTex(VolumetricClouds::bloom), 1);
		cloudsPPShader.setSampler2D("depthMap", s->sceneFBO->depthTex, 2);

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
		float lightDotCameraFront = glm::dot(glm::normalize(s->lightPos - s->cam->Position), glm::normalize(s->cam->Front));
		//std::cout << "light dot camera front= " << lightDotCameraFront << std::endl;
		if (lightDotCameraFront > 0.2) {
			isLightInFront = true;
		}

		cloudsPPShader.setBool("isLightInFront", isLightInFront);
		cloudsPPShader.setBool("enableGodRays", model->enableGodRays);
		cloudsPPShader.setFloat("lightDotCameraFront", lightDotCameraFront);

		cloudsPPShader.setFloat("time", glfwGetTime());
		if (!s->wireframe)
			model->postProcessingShader->draw();
	}
}



VolumetricClouds::~VolumetricClouds()
{
}
