#include <MainController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>


namespace app
{
    using engine::core::Controller;
    using engine::resources::Shader;


    void MainController::initialize()
    {
        engine::graphics::OpenGL::enable_depth_testing();
        engine::graphics::OpenGL::enable_face_culling();
        engine::graphics::OpenGL::set_clear_color(.002f, .004f, .012f);
        cubes.initialize();
        road.initialize();

        auto* platform_controller = Controller::get<engine::platform::PlatformController>();
        bloom.initialize(platform_controller->window()->width(), platform_controller->window()->height());
        shadow.initialize();
        platform_controller->set_enable_cursor(false);

        auto* resources = Controller::get<engine::resources::ResourcesController>();
        asphalt_texture = resources->texture("asphalt");
        ferrari_texture = resources->texture("ferrari_paint");
        street_lights_texture = resources->texture("street_lights_palette");
        formula_model = resources->model("formula");
        street_lights_model = resources->model("street_lights");


        skybox = resources->skybox("night_sky", "resources/skyboxes/night_sky");

        auto* camera = Controller::get<engine::graphics::GraphicsController>()->camera();
        camera->Position = {11, 6.4f, 17};
        camera->Yaw = -127;
        camera->Pitch = -16;
        camera->rotate_camera(0, 0);

        spdlog::info("F1 Drag Race ready: SPACE starts the two-stage event sequence.");
    }

    bool MainController::loop()
    {
        return Controller::get<engine::platform::PlatformController>()
               ->key(engine::platform::KEY_ESCAPE).is_up();
    }

    void MainController::poll_events()
    {
        auto* platform_controller = Controller::get<engine::platform::PlatformController>();
        auto press = [&](engine::platform::KeyId k)
        {
            return platform_controller->key(k).state() == engine::platform::Key::State::JustPressed;
        };

        if (press(engine::platform::KEY_SPACE)) start();
        if (press(engine::platform::KEY_R)) reset();
        if (press(engine::platform::KEY_Z)) cone = std::clamp(cone - 2.f, 10.f, 32.f);
        if (press(engine::platform::KEY_X)) cone = std::clamp(cone + 2.f, 10.f, 32.f);
        if (press(engine::platform::KEY_Q)) point_intensity = std::clamp(point_intensity - 0.2f, 0.f, 3.f);
        if (press(engine::platform::KEY_E)) point_intensity = std::clamp(point_intensity + 0.2f, 0.f, 3.f);
        if (press(engine::platform::KEY_B)) bloom_on = !bloom_on;

        if (press(engine::platform::KEY_F1))
        {
            captured = !captured;
            platform_controller->set_enable_cursor(!captured);
        }
    }

    void MainController::update()
    {
        const auto* platform_controller = Controller::get<engine::platform::PlatformController>();
        camera(*platform_controller);

        const float elapsed = platform_controller->frame_time().current - action;
        if (race == Race::Countdown && elapsed >= 2.f)
        {
            race = Race::Racing;
            spdlog::info("EVENT A: car accelerates.");
        }

        if (race == Race::Racing)
        {
            car_x += 5.2f * platform_controller->dt();
            if (elapsed >= 6.f)
            {
                race = Race::Safety;
                spdlog::info("EVENT B: car stops.");
            }
        }
    }

    void MainController::begin_draw()
    {
        auto* resources = Controller::get<engine::resources::ResourcesController>();
        auto* shader = resources->shader("point_shadow_depth");

        shadow.begin_depth_pass(point_pos);
        shader->use();
        for (int i = 0; i < 6; ++i)
            shader->set_mat4("shadowMatrices[" + std::to_string(i) + "]", shadow.shadow_matrices()[i]);
        shader->set_vec3("lightPos", point_pos);
        shader->set_float("farPlane", shadow.far_plane());
        scene(shader, true);

        const auto* platform_controller = Controller::get<engine::platform::PlatformController>();
        shadow.end_depth_pass(platform_controller->window()->width(), platform_controller->window()->height());
        bloom.begin_scene();
    }

    void MainController::draw()
    {
        auto* resources = Controller::get<engine::resources::ResourcesController>();
        auto* shader = resources->shader("f1_scene");
        auto* graphics = Controller::get<engine::graphics::GraphicsController>();

        if (skybox)
            graphics->draw_skybox(resources->shader("skybox"), skybox);

        shader->use();
        shader->set_mat4("projection", graphics->projection_matrix());
        shader->set_mat4("view", graphics->camera()->view_matrix());
        shader->set_vec3("viewPos", graphics->camera()->Position);
        shader->set_vec3("pointPosition", point_pos);
        shader->set_vec3("pointColor", {1.f, .72f, .35f});
        shader->set_float("pointIntensity", point_intensity);
        shader->set_vec3("spotPosition", {car_x + 2.19f, .35f, 0});
        shader->set_vec3("spotDirection", {1, -.07f, 0});
        shader->set_vec3("spotColor", {.76f, .86f, 1});
        shader->set_float("spotCutoff", glm::cos(glm::radians(cone)));
        shader->set_float("farPlane", shadow.far_plane());
        shader->set_int("depthMap", 4);
        shader->set_int("asphaltTexture", 2);
        shader->set_int("texture_diffuse1", 0);
        shader->set_int("streetLightsTexture", 3);

        asphalt_texture->bind_to_unit(2);
        street_lights_texture->bind_to_unit(3);
        shadow.bind(4);
        scene(shader, false);
    }


    void MainController::end_draw()
    {
        auto* resources = Controller::get<engine::resources::ResourcesController>();
        auto* blur = resources->shader("bloom_blur");
        auto* final = resources->shader("bloom_final");

        blur->use();
        blur->set_int("image", 0);
        final->use();
        final->set_int("scene", 0);
        final->set_int("bloomBlur", 1);
        bloom.compose(blur, final, bloom_on, 1.05f);

        Controller::get<engine::platform::PlatformController>()->swap_buffers();
    }

    void MainController::terminate()
    {
        shadow.destroy();
        bloom.destroy();
        cubes.destroy();
        road.destroy();
    }

    void MainController::start()
    {
        action = Controller::get<engine::platform::PlatformController>()->frame_time().current;
        race = Race::Countdown;
        car_x = -10;
        spdlog::info("ACTION: countdown started. The car moves after 2 seconds and stops after 6.");
    }

    void MainController::reset()
    {
        race = Race::Waiting;
        car_x = -10;
        spdlog::info("Car reset to starting position.");
    }

    void MainController::camera(const engine::platform::PlatformController& p)
    {
        if (!captured)
            return;

        auto* camera = Controller::get<engine::graphics::GraphicsController>()->camera();
        float dt = p.dt();
        if (p.key(engine::platform::KEY_W).is_down())
            camera->move_camera(engine::graphics::Camera::FORWARD, dt);
        if (p.key(engine::platform::KEY_S).is_down())
            camera->move_camera(engine::graphics::Camera::BACKWARD, dt);
        if (p.key(engine::platform::KEY_A).is_down())
            camera->move_camera(engine::graphics::Camera::LEFT, dt);
        if (p.key(engine::platform::KEY_D).is_down())
            camera->move_camera(engine::graphics::Camera::RIGHT, dt);

        auto m = p.mouse();
        camera->rotate_camera(m.dx, m.dy);
        camera->zoom(m.scroll);
    }

    void MainController::box(const Shader* s, glm::vec3 p, glm::vec3 scale, glm::vec3 col,
                             float emission, bool depth, int surface_mode) const
    {
        glm::mat4 m(1);
        m = glm::translate(m, p);
        m = glm::scale(m, scale);
        s->set_mat4("model", m);
        s->set_int("surfaceMode", surface_mode);

        if (!depth)
        {
            s->set_vec3("materialColor", col);
            s->set_float("emissive", emission);
        }

        cubes.draw();
    }

    void MainController::formula(const Shader* s, bool d) const
    {
        glm::mat4 m(1.f);
        m = glm::translate(m, {car_x, -.19f, 0.f});
        m = glm::rotate(m, glm::radians(90.f), {0.f, 1.f, 0.f});
        m = glm::scale(m, {.01f, .01f, .01f});
        s->set_mat4("model", m);
        s->set_int("surfaceMode", 2);

        if (!d)
        {
            s->set_vec3("materialColor", {.8f, .03f, .02f});
            s->set_float("emissive", 0.f);
            ferrari_texture->bind_to_unit(0);
        }

        formula_model->draw(s);
        box(s, {car_x + 2.12f, .34f, -.44f}, {.055f, .035f, .075f}, {.75f, .87f, 1}, 9, d);
        box(s, {car_x + 2.12f, .34f, .44f}, {.055f, .035f, .075f}, {.75f, .87f, 1}, 9, d);
        headlights(s, d, .34f, .44f, 2.05f, 2.19f);
    }

    void MainController::headlights(const engine::resources::Shader* s, bool d, float height,
                                    float lateral_offset, float housing_x, float lens_x) const
    {
        constexpr glm::vec3 housing_color{.045f, .05f, .065f};
        constexpr glm::vec3 lens_color{.82f, .91f, 1.f};

        for (float side : {-lateral_offset, lateral_offset})
        {
            box(s, {car_x + housing_x, height, side}, {.11f, .065f, .115f}, housing_color, 0.f, d);
            box(s, {car_x + lens_x, height, side}, {.018f, .038f, .068f}, lens_color, 7.f, d);
        }
    }

    void MainController::lamp(const engine::resources::Shader* s, float x, float z, bool d) const
    {
        if (street_lights_model)
        {
            glm::mat4 m(1.f);
            m = glm::translate(m, {x, -.1f, z});
            m = glm::rotate(m, glm::radians(z > 0.f ? 180.f : 0.f), {0.f, 1.f, 0.f});
            m = glm::scale(m, {1.65f, 1.65f, 1.65f});
            m = glm::translate(m, {1.2895f, -.0068f, 2.6153f});
            s->set_mat4("model", m);
            s->set_int("surfaceMode", 3);

            if (!d)
            {
                s->set_vec3("materialColor", {.08f, .08f, .08f});
                s->set_float("emissive", 0.f);
            }

            street_lights_model->draw_range(s, 12, 1);
            s->set_int("surfaceMode", 0);

            if (!d)
            {
                s->set_vec3("materialColor", {1.f, .52f, .12f});
                s->set_float("emissive", 2.5f);
            }

            street_lights_model->draw_range(s, 13, 1);
        }
    }

    void MainController::asphalt(const engine::resources::Shader* s, bool d) const
    {
        glm::mat4 m(1.f);
        m = glm::translate(m, {0.f, -.095f, 0.f});
        m = glm::scale(m, {30.f, 1.f, 6.f});
        s->set_mat4("model", m);
        s->set_int("surfaceMode", d ? 0 : 1);

        if (!d)
        {
            s->set_vec3("materialColor", {.08f, .08f, .08f});
            s->set_float("emissive", 0.f);
        }

        road.draw();
    }

    void MainController::scene(const engine::resources::Shader* s, bool d) const
    {
        asphalt(s, d);
        box(s, {0, .03f, -5.85f}, {30, .25f, .10f}, {.72f, .06f, .04f}, 0, d);
        box(s, {0, .03f, 5.85f}, {30, .25f, .10f}, {.72f, .06f, .04f}, 0, d);

        for (float x : {-18.f, -8.f, 2.f, 12.f, 22.f})
        {
            lamp(s, x, -5.3f, d);
            lamp(s, x, 5.3f, d);
        }

        formula(s, d);
    }
}
