#pragma once
#include "shaderUtils.h"
class TessellationShader : public Shader {
public:
	TessellationShader(const char* vertexPath, const char* tessControlPath, const char* tessEvalPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {
		std::cout << "CREATING TESSELLATION SHADER" << std::endl;
		// 1. retrieve the vertex/fragment source code from filePath
		// std::string gShaderPath = "gshader.gs";
		std::string tessControlCode;
		std::string tessEvalCode;
		std::string gShaderCode;
		std::ifstream tControlFile;
		std::ifstream tEvalFile;
		//std::ifstream gShaderFile;
		// ensure ifstream objects can throw exceptions:
		tControlFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		tEvalFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		//gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			tControlFile.open(tessControlPath);
			tEvalFile.open(tessEvalPath);
			//gShaderFile.open(gShaderPath);
			std::stringstream tControlStream, tEvalStream, gShaderStream;
			// read file's buffer contents into streams
			tControlStream << tControlFile.rdbuf();
			tEvalStream << tEvalFile.rdbuf();
			//gShaderStream << gShaderFile.rdbuf();
			// close file handlers
			tControlFile.close();
			tEvalFile.close();
			//gShaderFile.close();
			// convert stream into string
			tessControlCode = tControlStream.str();
			tessEvalCode = tEvalStream.str();
			//gShaderCode = gShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* tControlCode = tessControlCode.c_str();
		const char* tEvalCode = tessEvalCode.c_str();
		//const char* gShadCode = gShaderCode.c_str();
		// 2. compile shaders
		unsigned int tControl, tEval, gShader;
		int success;
		char infoLog[512];
		// Tessellation Control shader
		tControl = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tControl, 1, &tControlCode, NULL);
		glCompileShader(tControl);
		checkCompileErrors(tControl, "TESSELLATION_CONTROL");
		// Tessellation Evaluation Shader
		tEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tEval, 1, &tEvalCode, NULL);
		glCompileShader(tEval);
		checkCompileErrors(tEval, "TESSELLATION_EVALUATION");
		// Geometry Shader
		gShader = glCreateShader(GL_GEOMETRY_SHADER);
		//glShaderSource(gShader, 1, &gShadCode, NULL);
		//glCompileShader(gShader);
		//checkCompileErrors(gShader, "GEOMETRY");
		// linking TCS and TES shaders
		glAttachShader(ID, tControl);
		glAttachShader(ID, tEval);
		//glAttachShader(ID, gShader);
		//std::cout << "Geometry shader is not attached!" << std::endl;
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(tControl);
		glDeleteShader(tEval);
		//glDeleteShader(gShader);
	}

};
