#include "VolumetricClouds.h"
#include "imgui/imgui.h"

#define INT_CEIL(n,d) (int)ceil((float)n/d)

float VolumetricClouds::cloudSpeed = 450.0;
float VolumetricClouds::coverage = 0.45;
float VolumetricClouds::crispiness = 40.;
float VolumetricClouds::curliness = .1;
float VolumetricClouds::density = 0.02;
float VolumetricClouds::absorption = 0.35;

float VolumetricClouds::earthRadius = 600000.0;
float VolumetricClouds::sphereInnerRadius = 5000.0;
float VolumetricClouds::sphereOuterRadius = 17000.0;

float VolumetricClouds::perlinFrequency = 0.8;

bool VolumetricClouds::enableGodRays = false;
bool VolumetricClouds::enablePowder = false;

glm::vec3 VolumetricClouds::seed = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 VolumetricClouds::oldSeed = glm::vec3(0.0, 0.0, 0.0);

glm::vec3 VolumetricClouds::cloudColorTop = (glm::vec3(169., 149., 149.)*(1.5f / 255.f));
glm::vec3 VolumetricClouds::cloudColorBottom = (glm::vec3(65., 70., 80.)*(1.5f / 255.f));

glm::vec3 VolumetricClouds::skyColorTop = glm::vec3(0.5, 0.7, 0.8)*1.05f;
glm::vec3 VolumetricClouds::skyColorBottom =  glm::vec3(0.9 , 0.9, 0.95);

unsigned int VolumetricClouds::weatherTex = 0;
unsigned int VolumetricClouds::perlinTex = 0;
unsigned int VolumetricClouds::worley32 = 0;

void VolumetricClouds::generateWeatherMap() {
	bindTexture2D(weatherTex, 0);
	weatherShader->use();
	weatherShader->setVec3("seed", scene->seed);
	weatherShader->setFloat("perlinFrequency", perlinFrequency);
	std::cout << "computing weather!" << std::endl;
	glDispatchCompute(INT_CEIL(1024, 8), INT_CEIL(1024, 8), 1);
	std::cout << "weather computed!!" << std::endl;

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}



colorPreset VolumetricClouds::SunsetPreset() {
	colorPreset preset;

	preset.cloudColorBottom  = glm::vec3(89, 96, 109) / 255.f;
	preset.skyColorTop  = glm::vec3(177, 174, 119) / 255.f;
	preset.skyColorBottom  = glm::vec3(234, 125, 125) / 255.f;

	preset.lightColor  = glm::vec3(255, 171, 125) / 255.f;
	preset.fogColor  = glm::vec3(85, 97, 120) / 255.f;
	return preset;
}

colorPreset VolumetricClouds::DefaultPreset() {
	colorPreset preset;

	preset.cloudColorBottom = (glm::vec3(65., 70., 80.)*(1.5f / 255.f));
	
	preset.skyColorTop  = glm::vec3(0.5, 0.7, 0.8)*1.05f;
	preset.skyColorBottom  = glm::vec3(0.9, 0.9, 0.95);

	preset.lightColor  = glm::vec3(255, 255, 230) / 255.f;
	preset.fogColor  = glm::vec3(0.5, 0.6, 0.7);

	return preset;
}



void VolumetricClouds::mixSkyColorPreset(float v, colorPreset p1, colorPreset p2) {
	float a = std::min(std::max(v, 0.0f), 1.0f);
	float b = 1.0 - a;

	cloudColorBottom = p1.cloudColorBottom*a + p2.cloudColorBottom*b;
	skyColorTop = p1.skyColorTop*a + p2.skyColorTop*b;
	skyColorBottom = p1.skyColorBottom*a + p2.skyColorBottom*b;
	scene->lightColor = p1.lightColor*a + p2.lightColor*b;
	scene->fogColor = p1.fogColor*a + p2.fogColor*b;
}

colorPreset VolumetricClouds::SunsetPreset1() {
	colorPreset preset;

	preset.cloudColorBottom  = glm::vec3(97, 98, 120) / 255.f;
	preset.skyColorTop  = glm::vec3(133, 158, 214) / 255.f;
	preset.skyColorBottom  = glm::vec3(241, 161, 161) / 255.f;

	preset.lightColor  = glm::vec3(255, 201, 201) / 255.f;
	preset.fogColor  = glm::vec3(128, 153, 179) / 255.f;

	return preset;
}

VolumetricClouds::VolumetricClouds(int SW, int SH): SCR_WIDTH(SW), SCR_HEIGHT(SH) {
	//volumetricCloudsShader = new ScreenQuad("shaders/volumetric_clouds.frag");
	volumetricCloudsShader = new Shader("volumetricCloudsShader","shaders/volumetric_clouds.comp");
	ppShader = new ScreenQuad("shaders/clouds_post.frag");

	//cloudsFBO = new FrameBufferObject(SW, SH, 4);
	cloudsFBO = new TextureSet(SW, SH, 4);
	cloudsPostProcessingFBO = new FrameBufferObject(Window::SCR_WIDTH, Window::SCR_HEIGHT, 2);

	postProcess = true;

	/////////////////// TEXTURE GENERATION //////////////////

	if (!perlinTex) {
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
		glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));
		std::cout << "computed!!" << std::endl;
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glGenerateMipmap(GL_TEXTURE_3D);
	}

	if (!worley32) {
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
	}


		////////////////////////
		//compute shaders
		weatherShader = new Shader("weatherMap");
		weatherShader->attachShader("shaders/weather.comp");
		weatherShader->linkPrograms();
		if (!weatherTex) {
		//make texture
		this->weatherTex = Texture2D(1024, 1024);

		//compute
		generateWeatherMap();

		VolumetricClouds::seed = scene->seed;
		VolumetricClouds::oldSeed = seed;
	}
	}
#define TIMETO(CODE, TASK) 	t1 = glfwGetTime(); CODE; t2 = glfwGetTime(); std::cout << "Time to " + std::string(TASK) + " :" << (t2 - t1)*1e3 << "ms" << std::endl;

void VolumetricClouds::setGui() {

	ImGui::Begin("Clouds controls: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Post Proceesing");
	//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::Checkbox("Post Processing (Gaussian Blur)", this->getPostProcPointer());
	ImGui::Checkbox("God Rays", &enableGodRays);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Clouds rendering");

	ImGui::SliderFloat("Coverage", this->getCoveragePointer(), 0.0f, 1.0f);
	ImGui::SliderFloat("Speed", this->getCloudSpeedPtr(), 0.0f, 5.0E3);
	ImGui::SliderFloat("Crispiness", this->getCloudCrispinessPtr(), 0.0f, 120.0f);
	ImGui::SliderFloat("Curliness", &curliness, 0.0f, 3.0f);
	ImGui::SliderFloat("Density", &density, 0.0f, 0.1f);
	ImGui::SliderFloat("Light absorption", &absorption, 0.0f, 1.5f);
	ImGui::Checkbox("Enable sugar powder effect", &enablePowder);

	
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Dome controls");
	ImGui::SliderFloat("Sky dome radius", &earthRadius, 10000.0f, 5000000.0f);
	ImGui::SliderFloat("Clouds bottom height", &sphereInnerRadius, 1000.0f, 15000.0f);
	ImGui::SliderFloat("Clouds top height", &sphereOuterRadius, 1000.0f, 40000.0f);
	
	if(ImGui::SliderFloat("Clouds frequency", &perlinFrequency, 0.0f, 4.0f))
		generateWeatherMap();


	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Clouds conlors");
	glm::vec3 * cloudBottomColor = this->getCloudColorBottomPtr();
	ImGui::ColorEdit3("Cloud color", (float*)cloudBottomColor); // Edit 3 floats representing a color

	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sky colors controls");
	ImGui::ColorEdit3("Sky top color", (float*)this->getSkyTopColorPtr()); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Sky bottom color", (float*)this->getSkyBottomColorPtr()); // Edit 3 floats representing a color
	ImGui::End();
}

void VolumetricClouds::draw() {

	float t1, t2;

	seed = scene->seed;
	if (seed != oldSeed) {
		generateWeatherMap();
		oldSeed = seed;
	}

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
	cloudsShader.setFloat("cloudSpeed", cloudSpeed);
	cloudsShader.setFloat("crispiness", crispiness);
	cloudsShader.setFloat("curliness", curliness);
	cloudsShader.setFloat("absorption", absorption*0.01);
	cloudsShader.setFloat("densityFactor", density);

	//cloudsShader.setBool("enablePowder", enablePowder);
	
	cloudsShader.setFloat("earthRadius", earthRadius);
	cloudsShader.setFloat("sphereInnerRadius", sphereInnerRadius);
	cloudsShader.setFloat("sphereOuterRadius", sphereOuterRadius);

	cloudsShader.setVec3("cloudColorTop", cloudColorTop);
	cloudsShader.setVec3("cloudColorBottom", cloudColorBottom);
	
	cloudsShader.setVec3("skyColorTop", skyColorTop);
	cloudsShader.setVec3("skyColorBottom", skyColorBottom);

	glm::mat4 vp = s->projMatrix*s->cam.GetViewMatrix();
	cloudsShader.setMat4("invViewProj", glm::inverse(vp));
	cloudsShader.setMat4("gVP", vp);

	cloudsShader.setSampler3D("cloud", this->perlinTex, 0);
	cloudsShader.setSampler3D("worley32", this->worley32, 1);
	cloudsShader.setSampler2D("weatherTex", this->weatherTex, 2);
	cloudsShader.setSampler2D("depthMap", s->sceneFBO.depthTex, 3);

	//actual draw
	//volumetricCloudsShader->draw();
	if(!s->wireframe)
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
		cloudsPPShader.setBool("enableGodRays", enableGodRays);
		cloudsPPShader.setFloat("lightDotCameraFront", lightDotCameraFront);

		cloudsPPShader.setFloat("time", glfwGetTime());
		if (!s->wireframe)
			ppShader->draw();
	}
}



VolumetricClouds::~VolumetricClouds()
{
}
