
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>
#include <algorithm>

namespace engine::resources
{
    void Model::draw(const Shader* shader)
    {
        shader->use();
        for (auto& mesh : m_meshes)
        {
            mesh.draw(shader);
        }
    }

    void Model::draw_range(const Shader* shader, uint32_t first_mesh, uint32_t mesh_count)
    {
        shader->use();
        const uint32_t last_mesh = std::min(first_mesh + mesh_count, static_cast<uint32_t>(m_meshes.size()));
        for (uint32_t index = first_mesh; index < last_mesh; ++index)
        {
            m_meshes[index].draw(shader);
        }
    }

    void Model::destroy()
    {
        for (auto& mesh : m_meshes)
        {
            mesh.destroy();
        }
    }
} // namespace engine::resources
