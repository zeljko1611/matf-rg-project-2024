#ifndef MATF_RG_PROJECT_BLOOM_HPP
#define MATF_RG_PROJECT_BLOOM_HPP

#include <cstdint>

namespace engine::resources {
class Shader;
}

namespace engine::graphics {
/**
     * Off-screen HDR framebuffer with two color attachments and a separable Gaussian blur.
     * Applications draw the lit scene between begin_scene() and compose().
     */
class BloomRenderer {
public:
    void initialize(int width, int height);
    void resize(int width, int height);
    void begin_scene() const;
    void compose(const resources::Shader *blur_shader, const resources::Shader *final_shader,
                 bool enabled, float exposure, int blur_passes = 10) const;
    void destroy();

private:
    void allocate_targets(int width, int height);
    void draw_quad() const;

    uint32_t m_hdr_fbo{};
    uint32_t m_color_buffers[2]{};
    uint32_t m_pingpong_fbos[2]{};
    uint32_t m_pingpong_buffers[2]{};
    uint32_t m_depth_rbo{};
    uint32_t m_quad_vao{};
    uint32_t m_quad_vbo{};
    int m_width{};
    int m_height{};
};
}// namespace engine::graphics

#endif
