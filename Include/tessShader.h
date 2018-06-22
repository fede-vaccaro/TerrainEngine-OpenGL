#ifndef TESS_SHADER
#define TESS_SHADER

#include <shaderUtils.h>

class TessellationShader : public Shader {

	TessellationShader(const char* vertexPath, const char* tessControlPath, const char* tessEvalPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {
		// 1. retrieve the vertex/fragment source code from filePath
		std::string tessControlCode;
		std::string tessEvalCode;
		std::ifstream tControlFile;
		std::ifstream tEvalFile;
		// ensure ifstream objects can throw exceptions:
		tControlFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		tEvalFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			tControlFile.open(tessControlPath);
			tEvalFile.open(tessEvalPath);
			std::stringstream tControlStream, tEvalStream;
			// read file's buffer contents into streams
			tControlStream << tControlFile.rdbuf();
			tEvalStream << tEvalFile.rdbuf();
			// close file handlers
			tControlFile.close();
			tEvalFile.close();
			// convert stream into string
			tessControlCode = tControlStream.str();
			tessEvalCode = tEvalStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* tControlCode = tessControlCode.c_str();
		const char * tEvalCode = tessEvalCode.c_str();
		// 2. compile shaders
		unsigned int tControl, tEval;
		int success;
		char infoLog[512];
		// vertex shader
		tControl = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tControl, 1, &tControlCode, NULL);
		glCompileShader(tControl);
		checkCompileErrors(tControl, "TESSELLATION_CONTROL");
		// fragment Shader
		tEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tEval, 1, &tEvalCode, NULL);
		glCompileShader(tEval);
		checkCompileErrors(tEval, "TESSELLATION_EVALUATION");
		// shader Program
		ID = glCreateProgram();
		glAttachShader(ID, tControl);
		glAttachShader(ID, tEval);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(tControl);
		glDeleteShader(tEval);
	}

};


#endif // !TESS_SHADER
