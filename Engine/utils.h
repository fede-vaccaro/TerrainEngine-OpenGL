#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

void initializePlaneVAO(const int res, const int width, GLuint * planeVAO, GLuint * planeVBO, GLuint * planeEBO);

glm::vec3 genRandomVec3();