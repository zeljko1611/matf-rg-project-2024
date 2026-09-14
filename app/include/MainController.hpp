#pragma once

#include <engine/core/Engine.hpp>
#include <glm/glm.hpp>

#include <string_view>

namespace app
{
    class MainController final : public engine::core::Controller
    {
    public:
        std::string_view name() const override { return "F1NightRunController"; }

    private:
        enum class Race { Waiting, Countdown, Racing, Safety };

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
        void camera(const engine::platform::PlatformController& p);

        void box(const engine::resources::Shader* s, glm::vec3 p, glm::vec3 scale,
                 glm::vec3 col, float emission, bool depth, int surface_mode = 0) const;
        void formula(const engine::resources::Shader* s, bool d) const;
        void headlights(const engine::resources::Shader* s, bool d, float height,
                        float lateral_offset, float housing_x, float lens_x) const;
        void lamp(const engine::resources::Shader* s, float x, float z, bool d) const;
        void asphalt(const engine::resources::Shader* s, bool d) const;
        void scene(const engine::resources::Shader* s, bool d) const;

        engine::graphics::BoxMesh cubes;
        engine::graphics::PlaneMesh road;
        engine::graphics::BloomRenderer bloom;
        engine::graphics::PointShadowMap shadow;

        engine::resources::Texture* asphalt_texture{};
        engine::resources::Texture* ferrari_texture{};
        engine::resources::Texture* street_lights_texture{};
        engine::resources::Model* formula_model{};
        engine::resources::Model* street_lights_model{};
        engine::resources::Skybox* skybox{};

        Race race{Race::Waiting};
        float action{}, car_x{-10}, cone{19}, point_intensity{1.5f};
        glm::vec3 point_pos{1.52f, 4.88f, -6.2f};
        bool bloom_on{true}, captured{true};

        static bool skybox_is_ready();
    };
} // namespace app
