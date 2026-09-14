#pragma once

#include <engine/core/Engine.hpp>
#include <glm/glm.hpp>
#include <string_view>

namespace app
{
    class MainController final : public engine::core::Controller
    {
    public:
        std::string_view name() const override { return "MainController"; }

    private:
        void initialize() override;
        bool loop() override;
        void poll_events() override;
        void update() override;
        void begin_draw() override;
        void draw() override;
        void end_draw() override;
        void terminate() override;
    };
}
