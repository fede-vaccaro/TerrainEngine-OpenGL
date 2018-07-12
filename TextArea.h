#pragma once
#include <glad/glad.h>
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Object.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include <map>

class guiElement{
private:
	std::string name;
	double* value_d = NULL;
	float* value_f = NULL;
	int* value_i = NULL;
	std::string* value_s = NULL;

public:
	guiElement(std::string name, float*value);
	guiElement(std::string name, double * value);
	guiElement(std::string name, int * value);
	guiElement(std::string name, std::string * value);
	guiElement(std::string name);
	std::string getString();
	void change(int verse);

};

struct screen {
	unsigned short int HEIGHT;
	unsigned short int WIDTH;
};

struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};


class TextArea
{
private:
	float width, height, pos_x, pos_y, width_norm, height_norm, x_pos, y_pos, coord_x, coord_y;
	int screenW, screenH;
	float aspect;
	static screen* SCREEN;
	static GLFWwindow* window;
	static glm::vec4 bgColor;
	static float vertices[];
	glm::mat4 projection;
	GLuint VAO, VBO, Texture;
	Shader* shader, *shader_text;
	Object* points;
	glm::mat4 transformation;
	glm::mat4 scaleMat;
	glm::vec3 textColor;
	glm::vec3 textColorBinded;
	bool visible;
	std::vector<guiElement> *rows;
	std::map<GLchar, Character> Characters;
	int bindedElement = 0;

	void initFreeType();
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);


public:
	static void setScreen(screen * ss);
	static void setWindow(GLFWwindow * w);
	TextArea(int x, int y, int w, int h);
	void calculateMatrix();
	~TextArea();
	void setVisible(bool set);
	void clear();
	void addElement(std::string name, float* value);
	void addElement(std::string name, double* value);
	void addElement(std::string name, int* value);
	void addElement(std::string name, std::string* value);
	void addElement(std::string name);
	void draw();
	void bind(int verse);
	void change(int verse);
	bool getVisible() { return visible; };
};

