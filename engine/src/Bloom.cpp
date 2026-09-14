#include <glad/glad.h>
#include <engine/graphics/Bloom.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/util/Errors.hpp>

namespace engine::graphics
{
    void BloomRenderer::initialize(int width, int height)
    {
        m_width = width;
        m_height = height;
        CHECKED_GL_CALL(glGenFramebuffers, 1, &m_hdr_fbo);
        CHECKED_GL_CALL(glGenFramebuffers, 2, m_pingpong_fbos);
        CHECKED_GL_CALL(glGenTextures, 2, m_color_buffers);
        CHECKED_GL_CALL(glGenTextures, 2, m_pingpong_buffers);
        CHECKED_GL_CALL(glGenRenderbuffers, 1, &m_depth_rbo);
        allocate_targets(width, height);
    }

    void BloomRenderer::allocate_targets(int width, int height)
    {
        m_width = width;
        m_height = height;
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_hdr_fbo);
        for (int i = 0; i < 2; ++i)
        {
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_color_buffers[i]);
            CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                            m_color_buffers[i], 0);
        }
        constexpr unsigned attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        CHECKED_GL_CALL(glDrawBuffers, 2, attachments);
        CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, m_depth_rbo);
        CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_rbo);
        RG_GUARANTEE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                     "HDR framebuffer is incomplete");

        for (int i = 0; i < 2; ++i)
        {
            CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_pingpong_fbos[i]);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_pingpong_buffers[i]);
            CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            m_pingpong_buffers[i], 0);
            RG_GUARANTEE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                         "Bloom ping-pong framebuffer is incomplete");
        }
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    }

    void BloomRenderer::resize(int width, int height)
    {
        if (width > 0 && height > 0 && (width != m_width || height != m_height))
        {
            allocate_targets(width, height);
        }
    }

    void BloomRenderer::begin_scene() const
    {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_hdr_fbo);
        CHECKED_GL_CALL(glViewport, 0, 0, m_width, m_height);
        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void BloomRenderer::draw_quad() const
    {
        if (m_quad_vao == 0)
        {
            constexpr float quad[] = {
                -1.f, 1.f, 0.f, 1.f, -1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 1.f, 0.f,
                -1.f, 1.f, 0.f, 1.f, 1.f, -1.f, 1.f, 0.f, 1.f, 1.f, 1.f, 1.f,
            };
            auto* self = const_cast<BloomRenderer*>(this);
            CHECKED_GL_CALL(glGenVertexArrays, 1, &self->m_quad_vao);
            CHECKED_GL_CALL(glGenBuffers, 1, &self->m_quad_vbo);
            CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
            CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_quad_vbo);
            CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
            CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
            CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
            CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
            CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                            reinterpret_cast<void *>(2 * sizeof(float)));
        }
        CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
        CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
        CHECKED_GL_CALL(glBindVertexArray, 0);
    }

    void BloomRenderer::compose(const resources::Shader* blur_shader, const resources::Shader* final_shader,
                                bool enabled, float exposure, int blur_passes) const
    {
        bool horizontal = true;
        bool first_pass = true;
        blur_shader->use();
        for (int i = 0; i < blur_passes; ++i)
        {
            CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_pingpong_fbos[horizontal]);
            blur_shader->set_bool("horizontal", horizontal);
            CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D,
                            first_pass ? m_color_buffers[1] : m_pingpong_buffers[!horizontal]);
            draw_quad();
            horizontal = !horizontal;
            first_pass = false;
        }
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        CHECKED_GL_CALL(glViewport, 0, 0, m_width, m_height);
        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        final_shader->use();
        final_shader->set_bool("bloom", enabled);
        final_shader->set_float("exposure", exposure);
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_color_buffers[0]);
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE1);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_pingpong_buffers[!horizontal]);
        draw_quad();
    }

    void BloomRenderer::destroy()
    {
        if (m_quad_vbo)
            CHECKED_GL_CALL(glDeleteBuffers, 1, &m_quad_vbo);
        if (m_quad_vao)
            CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_quad_vao);
        if (m_depth_rbo)
            CHECKED_GL_CALL(glDeleteRenderbuffers, 1, &m_depth_rbo);
        if (m_hdr_fbo)
            CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_hdr_fbo);
        CHECKED_GL_CALL(glDeleteFramebuffers, 2, m_pingpong_fbos);
        CHECKED_GL_CALL(glDeleteTextures, 2, m_color_buffers);
        CHECKED_GL_CALL(glDeleteTextures, 2, m_pingpong_buffers);
    }
}
