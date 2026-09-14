#include <glad/glad.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PlaneMesh.hpp>

namespace engine::graphics
{
    void PlaneMesh::initialize()
    {
        constexpr float vertices[] = {
            -1.f, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f,
            -1.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f,
            1.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, 1.f,
            1.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, 1.f,
            1.f, 0.f, -1.f, 0.f, 1.f, 0.f, 1.f, 0.f,
            -1.f, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f,
        };
        CHECKED_GL_CALL(glGenVertexArrays, 1, &m_vao);
        CHECKED_GL_CALL(glGenBuffers, 1, &m_vbo);
        CHECKED_GL_CALL(glBindVertexArray, m_vao);
        CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);
        CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
        CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
        CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
        CHECKED_GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
        CHECKED_GL_CALL(glEnableVertexAttribArray, 2);
        CHECKED_GL_CALL(glVertexAttribPointer, 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        reinterpret_cast<void *>(6 * sizeof(float)));
    }

    void PlaneMesh::draw() const
    {
        CHECKED_GL_CALL(glBindVertexArray, m_vao);
        CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
    }

    void PlaneMesh::destroy() const
    {
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_vbo);
        CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_vao);
    }
}
