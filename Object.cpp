#include "Object.h"



void Object::loadData(float * Verts, int size)
{
	data_f = Verts;
	drawMode = GL_TRIANGLES;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//std::cout << sizeof(float) *size << std::endl;
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, data_f, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// location, attribute size, type, normalize, stride, offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void Object::loadData(float * Verts, int size, int stride)
{
	data_f = Verts;
	drawMode = GL_TRIANGLES;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//std::cout << sizeof(float) *size << std::endl;
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, data_f, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// location, attribute size, type, normalize, stride, offset
	glVertexAttribPointer(0, stride, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}
void Object::loadData(float * Verts, int size, int stride, int offset)
{
	data_f = Verts;
	drawMode = GL_TRIANGLES;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//std::cout << sizeof(float) *size << std::endl;
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, data_f, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// location, attribute size, type, normalize, stride, offset
	glVertexAttribPointer(0, stride, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, stride, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);
	glEnableVertexAttribArray(0);
}

void Object::loadData(float * Verts, int size, int stride, int size2, int stride2)
{
	data_f = Verts;
	drawMode = GL_TRIANGLES;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (size+size2), data_f, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// location, attribute size, type, normalize, stride, offset
	glVertexAttribPointer(0, size, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, size2, GL_FLOAT, GL_FALSE, stride2 * sizeof(float), (void*)(sizeof(float)*size));
	glEnableVertexAttribArray(1);
}

void Object::loadData(float * Verts, int size0, int stride0, int size2, int stride2, int size3, int stride3)
{
	data_f = Verts;
	drawMode = GL_TRIANGLES;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Verts), data_f, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// location, attribute size, type, normalize, stride, offset
	glVertexAttribPointer(0, size0, GL_FLOAT, GL_FALSE, (size0 + size2 + size3) * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, size2, GL_FLOAT, GL_FALSE, (size0 + size2 + size3) * sizeof(float), (void*)(stride2*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, size3, GL_FLOAT, GL_FALSE, (size0 + size2 + size3) * sizeof(float), (void*)(stride3*sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Object::loadData(std::vector<float3>* Verts)
{
	this->data = Verts;
	drawMode = GL_TRIANGLES;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float3)*data->size(), data->data(), GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// location, attribute size, type, normalize, stride, offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

Object::Object()
{
}

Object::Object(std::vector<float3>* vertices)
{
	loadData(vertices);
}

Object::Object(float* vertices, int size)
{
	data_f_size = size;
	loadData(vertices, size);
}

Object::Object(float* data, int size, int stride) {
	data_f_size = size;
	loadData(data, size, stride);
}

Object::Object(float * Verts, int size, int stride, int size2, int stride2)
{
	data_f_size = size;
	loadData(Verts, size, stride, size2, stride2);
}

Object::Object(float * Verts, int size, int stride, int size2, int stride2, int size3, int stride3)
{
	data_f_size = size;
	loadData(Verts, size, stride, size2, stride2, size3, stride3);
}


Object::~Object()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	if(EBO != -1)
		glDeleteBuffers(1, &EBO);
}

void Object::setIndices(std::vector<uint3>* ind)
{
	this->indices = ind;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint3)*indices->size(), indices->data(), GL_STATIC_DRAW);
}

void Object::setShader(Shader* shad)
{
	this->shader = shad;
}

void Object::sendUniform(const std::string & name, void * uniform) const
{
}

void Object::setDrawMode(GLenum drawMode)
{
	this->drawMode = drawMode;
}

void Object::drawObject()
{
	int size = this->data_f_size;

	if(data != NULL) {
		size = data->size();
	}
		

	glBindVertexArray(VAO);
	if (EBO != -1  && indices != NULL) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(drawMode, indices->size() * 3, GL_UNSIGNED_INT, 0);
	}
	else if (drawMode == GL_POINTS) {
		glEnable(GL_PROGRAM_POINT_SIZE);
		glDrawArrays(drawMode, 0, size);
	}
	else {
		//std::cout << "DRAWING" << std::endl;
		glDrawArrays(drawMode, 0, size);
	}
	glBindVertexArray(0);
}

Shader * Object::useShader()
{
	shader->use();
	return shader;
}

