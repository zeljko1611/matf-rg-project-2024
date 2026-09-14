#ifndef MATF_RG_PROJECT_POINT_SHADOW_HPP
#define MATF_RG_PROJECT_POINT_SHADOW_HPP

#include <array>
#include <cstdint>
#include <glm/glm.hpp>

namespace engine::graphics
{
    /** Creates and manages the depth cubemap used by Point Shadows. */
    class PointShadowMap
    {
    public:
        void initialize(int resolution = 1024, float far_plane = 45.0f);
        void begin_depth_pass(const glm::vec3& light_position);
        void end_depth_pass(int viewport_width, int viewport_height) const;
        void bind(uint32_t texture_unit = 4) const;
        void destroy();

        const std::array<glm::mat4, 6>& shadow_matrices() const { return m_shadow_matrices; }
        float far_plane() const { return m_far_plane; }

    private:
        uint32_t m_fbo{};
        uint32_t m_depth_cubemap{};
        int m_resolution{};
        float m_far_plane{};
        std::array<glm::mat4, 6> m_shadow_matrices{};
    };
}

#endif
