#pragma once
#include <string>
#include <glad/glad.h>

struct shaderType {
	//shaderType(shaderType& shaderT) : type(shaderT.type), name(shaderT.name) {}
	shaderType() : type(-1), name("") {}
	shaderType(unsigned int type, std::string name) : type(type), name(name){}
	unsigned int type;
	std::string name;
};

bool checkCompileErrors(unsigned int shader, std::string type, std::string shaderName);
std::string getShaderName(const char* shaderPath);
shaderType getShaderType(const char* path);

class BaseShader
{
public:
	BaseShader(const char * shaderPath);
	virtual ~BaseShader();
	std::string getName() {
		return getShaderName(path.c_str());
	}
	shaderType getType() {
		return shadType;
	}
	unsigned int getShad() {
		return shad;
	}
private:
	std::string loadShaderFromFile(const char* shaderPath);

	std::string path;


	unsigned int shad;
	shaderType shadType;
};

