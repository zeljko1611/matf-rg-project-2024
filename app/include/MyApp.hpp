#pragma once
#include <engine/core/Engine.hpp>

namespace app
{
    class MyApp final : public engine::core::App
    {
    public:
        void app_setup() override;
    };
}
