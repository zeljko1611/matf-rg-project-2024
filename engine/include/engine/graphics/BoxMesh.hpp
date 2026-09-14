#ifndef MATF_RG_PROJECT_BOX_MESH_HPP
#define MATF_RG_PROJECT_BOX_MESH_HPP

#include <cstdint>

namespace engine::graphics
{
    class BoxMesh
    {
    public:
        void initialize();
        void draw() const;
        void destroy() const;

    private:
        uint32_t m_vao{};
        uint32_t m_vbo{};
    };
}

#endif
