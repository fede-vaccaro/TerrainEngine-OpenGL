#include "shader.h"


	Shader::Shader(const char* vertexPath, const char* fragmentPath)
	{
		std::cout << "CREATING BASE SHADER" << std::endl;
		std::string vShaderCode = loadShaderFromFile(vertexPath);
		std::string fShaderCode = loadShaderFromFile(fragmentPath);

		const char * vShaderString = vShaderCode.c_str();
		const char * fShaderString = fShaderCode.c_str();
		// 2. compile shaders
		unsigned int vertex, fragment;
		int success;
		char infoLog[512];
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderString, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderString, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		// shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		std::cout << "Fragment and Vertex Shader successfully compiled and linked into shader program!" << std::endl << std::endl;
	}

	Shader::~Shader() {

	}

	void Shader::use()
	{
		glUseProgram(ID);
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
	void Shader::checkCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
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
		
		if (success) {
			std::cout << type + " SHADER SUCCESSFULLY COMPILED AND/OR LINKED!" << std::endl;
		}
	}

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
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		return shaderCode;

	}


