#pragma once

#include <engine/core/Engine.hpp>

namespace app {
class GUIController final : public engine::core::Controller {
public:
    std::string_view name() const override { return "GUIController"; }
};
}// namespace app
