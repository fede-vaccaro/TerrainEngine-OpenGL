#pragma once
#include <glad/glad.h>
#include <vector>
#include "shader.h"
#include "CustomTypes.h"

class Object
{
private:
	float* data_f = NULL;
	int data_f_size;
	std::vector<float3>* data = NULL;
	std::vector<uint3>* indices = NULL;
	
protected:
	unsigned int VAO = -1,
		VBO = -1,
		EBO = -1;
	GLenum drawMode;
	Shader* shader;
	void loadData(float* data, int size);
	void loadData(float* Verts, int size, int stride);
	void loadData(float* Verts, int size, int stride, int offset);
	void loadData(float* Verts, int size, int stride, int size2, int stride2);
	void loadData(float * Verts, int size, int stride, int size2, int stride2, int size3, int stride3);
	void loadData(std::vector<float3>* data);
	Object();

public:
	Object(std::vector<float3>* vertices);
	Object(float* vertices, int size);
	Object(float* vertices, int size, int stride);
	Object(float* Verts, int size, int stride, int size2, int stride2);
	Object(float * Verts, int size, int stride, int size2, int stride2, int size3, int stride3);

	~Object();
	void setIndices(std::vector<uint3>* ind);
	void setShader(Shader* shad);
	void sendUniform(const std::string & name, void* uniform)const;
	void setDrawMode(GLenum drawMode);
	void drawObject();
	Shader* useShader();
};

