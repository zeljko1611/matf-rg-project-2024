#include <MyApp.hpp>
#include <GUIController.hpp>
#include <MainController.hpp>
#include <engine/core/Engine.hpp>

namespace app
{
    void MyApp::app_setup()
    {
        auto main_controller = register_controller<app::MainController>();
        auto gui_controller = register_controller<app::GUIController>();

        main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        main_controller->before(gui_controller);
    }
}
