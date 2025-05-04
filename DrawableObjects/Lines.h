#pragma once

#include <glad/glad.h>

#include <vector>

#include <glm/glm.hpp>

#include "drawableObject.h"
#include "../Engine/ShadingProgram.h"

namespace terrain::drawable

{

struct Line
{   
    struct LineVertex
    {
        glm::vec3 pos{};
        glm::vec3 color{};
    };

    Line(const LineVertex& o, const LineVertex& e): origin(o), end(e) {}
    Line(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color): origin(LineVertex{start, color}), end(LineVertex{end, color}) {}

    LineVertex origin;
    LineVertex end;
};

class Lines: public drawableObject
{

    std::vector<Line::LineVertex> _lineVertices;

    GLuint _vao;
    GLuint _vbo;

    gl::ShadingProgram _shader;

    static float _gLineWidth;

public:

    static void setLineWidth(float width);

    Lines();
    ~Lines();

    void addLine(Line l);
    void updateBuffers();
    void draw() override;
};

} // namespace terrain::drawable