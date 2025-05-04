
#include <glad/glad.h>

#include "Lines.h"

namespace terrain::drawable
{

float Lines::_gLineWidth = 1.0f;

void Lines::setLineWidth(float width)
{
    if(width <= 0.0)
        return;

    _gLineWidth = width;
}

Lines::Lines()
{
    auto programBuilder = gl::ProgramBuilder{};

    _shader = programBuilder
    .newProgram("lines")
    .attachShader(gl::Shader::loadFrom("shaders/line.vert").value())
    .attachShader(gl::Shader::loadFrom("shaders/line.frag").value())
    .linkPrograms().value();
}

Lines::~Lines()
{
    glDeleteBuffers(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}

void Lines::addLine(Line line)
{
    _lineVertices.emplace_back(std::move(line.origin));
    _lineVertices.emplace_back(std::move(line.end));
}

void Lines::updateBuffers()
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glBufferData(GL_ARRAY_BUFFER, _lineVertices.size() * sizeof(Line::LineVertex), _lineVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, decltype(Line::LineVertex::pos)::length(), GL_FLOAT, GL_FALSE, sizeof(Line::LineVertex), (void*)offsetof(Line::LineVertex, Line::LineVertex::pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, decltype(Line::LineVertex::color)::length(), GL_FLOAT, GL_FALSE, sizeof(Line::LineVertex), (void*)offsetof(Line::LineVertex, Line::LineVertex::color));
    glEnableVertexAttribArray(1);
}

void Lines::draw()
{
    if(!_lineVertices.empty())
    {
        _shader.use();

        auto mvp = drawableObject::scene->projMatrix * drawableObject::scene->cam->GetViewMatrix();

        _shader.setMat4("u_mvp", mvp);

        glEnable(GL_LINE_SMOOTH);
        glLineWidth(Lines::_gLineWidth);

        glBindVertexArray(_vao);
        
        glDrawArrays(GL_LINES, 0, _lineVertices.size());

        glBindVertexArray(0);
        glDisable(GL_LINE_SMOOTH);
    }
}

} // namespace terrain::drawable