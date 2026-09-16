#pragma once

#include <engine/core/Engine.hpp>
#include <glm/glm.hpp>

#include <string_view>

namespace app {
class MainController final : public engine::core::Controller {
public:
    std::string_view name() const override { return "F1NightRunController"; }

private:
    enum class Race { Waiting,
                      Countdown,
                      Racing,
                      Safety };

    void initialize() override;
    bool loop() override;
    void poll_events() override;
    void update() override;
    void begin_draw() override;
    void draw() override;
    void end_draw() override;
    void terminate() override;

    void start();
    void reset();
    void camera(const engine::platform::PlatformController &p);

    void box(const engine::resources::Shader *s, glm::vec3 p, glm::vec3 scale,
             glm::vec3 col, float emission, bool depth, int surface_mode = 0) const;
    void formula(const engine::resources::Shader *s, bool d) const;
    void headlights(const engine::resources::Shader *s, bool d, float height,
                    float lateral_offset, float housing_x, float lens_x) const;
    void lamp(const engine::resources::Shader *s, float x, float z, bool d) const;
    void asphalt(const engine::resources::Shader *s, bool d) const;
    void scene(const engine::resources::Shader *s, bool d) const;

    engine::graphics::BoxMesh m_cubes;
    engine::graphics::PlaneMesh m_road;
    engine::graphics::BloomRenderer m_bloom;
    engine::graphics::PointShadowMap m_shadow;

    engine::resources::Texture *m_asphalt_texture{};
    engine::resources::Texture *m_ferrari_texture{};
    engine::resources::Texture *m_street_lights_texture{};
    engine::resources::Model *m_formula_model{};
    engine::resources::Model *m_street_lights_model{};
    engine::resources::Skybox *m_skybox{};

    Race m_race{Race::Waiting};
    float m_action{}, m_car_x{-10}, m_cone{19}, m_point_intensity{1.5f};
    glm::vec3 m_point_pos{1.52f, 4.88f, -6.2f};
    bool m_bloom_on{true}, m_captured{true};

    static bool skybox_is_ready();
};
}// namespace app
