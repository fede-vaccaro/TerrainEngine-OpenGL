#include "shader.h"

Shader::Shader(std::string name) : name(name)
{
	linked = false;
	isCompute = false;
	ID = glCreateProgram();
}

Shader::~Shader() {
	glDeleteProgram(ID);
}

Shader::Shader(std::string name, const char * computeShaderPath) : name(name)
{
	linked = false;
	isCompute = false;
	ID = glCreateProgram();

	this->attachShader(BaseShader(computeShaderPath));
	this->linkPrograms();
}


Shader * Shader::attachShader(BaseShader s)
{
	if (!isCompute) {
		glAttachShader(ID, s.getShad());
		if (s.getName() == "COMPUTE")
			isCompute = true;
		this->shaders.push_back(s.getShad());
	}
	else {
		std::cout << "ERROR: TRYING TO LINK A NON COMPUTE SHADER TO COMPUTE PROGRAM" << std::endl;
	}

	return this;
}

void Shader::linkPrograms()
{
	glLinkProgram(ID);

	if (checkCompileErrors(ID, "PROGRAM", "")) {
		linked = true;
		std::cout << "PROGRAM " << name << " CORRECTLY LINKED" << std::endl;
		while (!shaders.empty()) {
			glDeleteShader(shaders.back());
			shaders.pop_back();
		}
	}
	else {
		std::cout << "ERROR WHILE LINKING TO " << name << " PROGRAM" << std::endl;
	}
}


void Shader::use()
{
	if (linked)
		glUseProgram(ID);
	else {
		std::cout << "PROGRAMS NOT LINKED!" << std::endl;
	}
}
// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec2(const std::string &name, glm::vec2 vector) const
{
	unsigned int location = glGetUniformLocation(ID, name.c_str());
	glUniform2fv(location, 1, glm::value_ptr(vector));
}
void Shader::setVec3(const std::string &name, glm::vec3 vector) const
{
	unsigned int location = glGetUniformLocation(ID, name.c_str());
	glUniform3fv(location, 1, glm::value_ptr(vector));
}

void Shader::setVec4(const std::string &name, glm::vec4 vector) const
{
	unsigned int location = glGetUniformLocation(ID, name.c_str());
	glUniform4fv(location, 1, glm::value_ptr(vector));
}

void Shader::setMat4(const std::string &name, glm::mat4 matrix) const
{
	unsigned int mat = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(mat, 1, false, glm::value_ptr(matrix));
}

void Shader::setSampler2D(const std::string &name, unsigned int texture, int id) const
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->setInt(name, id);
}
void Shader::setSampler3D(const std::string &name, unsigned int texture, int id) const
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_3D, texture);
	this->setInt(name, id);
}
/*
void Shader::checkCompileErrors(unsigned int shader, std::string type, std::string shaderName)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR: SHADER" << shaderName << "COMPILATION ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}

	//if (success) {
	//	std::cout << type + " SHADER SUCCESSFULLY COMPILED AND/OR LINKED!" << std::endl;
	//}
}
*/
/*
std::string Shader::loadShaderFromFile(const char* shaderPath) {
	std::string shaderCode;
	std::ifstream shaderFile;
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		shaderFile.open(shaderPath);
		std::stringstream shaderStream;
		// read file's buffer contents into streams
		shaderStream << shaderFile.rdbuf();
		// close file handlers
		shaderFile.close();
		// convert stream into string
		shaderCode = shaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER "<< getShaderName(shaderPath) << " FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	return shaderCode;

}

std::string Shader::getShaderName(const char* path) {
	std::string pathstr = std::string(path);
	const size_t last_slash_idx = pathstr.find_last_of("/");
	if (std::string::npos != last_slash_idx)
	{
		pathstr.erase(0, last_slash_idx + 1);
	}
	return pathstr;
}
*/
