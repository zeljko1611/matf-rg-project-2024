#include <MainController.hpp>

#include <engine/core/Engine.hpp>
#include <engine/platform/PlatformController.hpp>

namespace app
{
    using engine::core::Controller;

    void MainController::initialize()
    {
    }

    bool MainController::loop()
    {
        return true;
    }

    void MainController::poll_events()
    {
    }

    void MainController::update()
    {
    }

    void MainController::begin_draw()
    {
    }

    void MainController::draw()
    {
    }

    void MainController::end_draw()
    {
        Controller::get<engine::platform::PlatformController>()->swap_buffers();
    }

    void MainController::terminate()
    {
    }
}
