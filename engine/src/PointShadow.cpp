#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PointShadow.hpp>
#include <engine/util/Errors.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::graphics {
void PointShadowMap::initialize(int resolution, float far_plane) {
    m_resolution = resolution;
    m_far_plane = far_plane;
    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_fbo);
    CHECKED_GL_CALL(glGenTextures, 1, &m_depth_cubemap);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, m_depth_cubemap);
    for (unsigned i = 0; i < 6; ++i) {
        CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                        resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
    CHECKED_GL_CALL(glFramebufferTexture, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_cubemap, 0);
    CHECKED_GL_CALL(glDrawBuffer, GL_NONE);
    CHECKED_GL_CALL(glReadBuffer, GL_NONE);
    RG_GUARANTEE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                 "Point-shadow framebuffer is incomplete");
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void PointShadowMap::begin_depth_pass(const glm::vec3 &light_position) {
    const glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 1.f, m_far_plane);
    m_shadow_matrices = {
            projection * glm::lookAt(light_position, light_position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
            projection * glm::lookAt(light_position, light_position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
            projection * glm::lookAt(light_position, light_position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
            projection * glm::lookAt(light_position, light_position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
            projection * glm::lookAt(light_position, light_position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
            projection * glm::lookAt(light_position, light_position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)),
    };
    CHECKED_GL_CALL(glViewport, 0, 0, m_resolution, m_resolution);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
    CHECKED_GL_CALL(glClear, GL_DEPTH_BUFFER_BIT);
}

void PointShadowMap::end_depth_pass(int viewport_width, int viewport_height) const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    CHECKED_GL_CALL(glViewport, 0, 0, viewport_width, viewport_height);
}

void PointShadowMap::bind(uint32_t texture_unit) const {
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0 + texture_unit);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, m_depth_cubemap);
}

void PointShadowMap::destroy() {
    if (m_fbo)
        CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_fbo);
    if (m_depth_cubemap)
        CHECKED_GL_CALL(glDeleteTextures, 1, &m_depth_cubemap);
}
}// namespace engine::graphics
