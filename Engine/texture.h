#pragma once
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <string>

#include <stb_image.h>

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
unsigned int loadCubemap(std::vector<std::string> faces);
unsigned int generateTexture2D(int w, int h);
unsigned int generateTexture3D(int w, int h, int d);
void bindTexture2D(unsigned int tex, int unit = 0);