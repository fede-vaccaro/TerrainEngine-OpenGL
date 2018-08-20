#pragma once
#include "shader.h"
class TessellationShader : public Shader {
public:
	TessellationShader(const char* vertexPath, const char* tessControlPath, const char* tessEvalPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {
		//std::cout << "CREATING TESSELLATION SHADER" << std::endl;
		std::string tessControlCode = loadShaderFromFile(tessControlPath);
		std::string tessEvaluationCode = loadShaderFromFile(tessEvalPath);

		const char* tControlString = tessControlCode.c_str();
		const char* tEvalString = tessEvaluationCode.c_str();
		//const char* gShadCode = gShaderCode.c_str();
		// 2. compile shaders
		unsigned int tControl, tEval, gShader;
		int success;
		char infoLog[512];
		// Tessellation Control shader
		tControl = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tControl, 1, &tControlString, NULL);
		glCompileShader(tControl);
		checkCompileErrors(tControl, "TESSELLATION_CONTROL", getShaderName(tessControlPath));
		// Tessellation Evaluation Shader
		tEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tEval, 1, &tEvalString, NULL);
		glCompileShader(tEval);
		checkCompileErrors(tEval, "TESSELLATION_EVALUATION", getShaderName(tessEvalPath));
		// Geometry Shader
		//gShader = glCreateShader(GL_GEOMETRY_SHADER);
		//glShaderSource(gShader, 1, &gShadCode, NULL);
		//glCompileShader(gShader);
		//checkCompileErrors(gShader, "GEOMETRY");
		// linking TCS and TES shaders
		glAttachShader(ID, tControl);
		glAttachShader(ID, tEval);
		//glAttachShader(ID, gShader);
		//std::cout << "Geometry shader is not attached!" << std::endl;
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM", "");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(tControl);
		glDeleteShader(tEval);
		//glDeleteShader(gShader);
		std::cout << getShaderName(tessControlPath) << " AND " << getShaderName(tessEvalPath) << " LOADED AND COMPILED!" << std::endl;
	}

};
