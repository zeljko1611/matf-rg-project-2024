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

        auto* p = Controller::get<engine::platform::PlatformController>();
        bloom.initialize(p->window()->width(), p->window()->height());
        shadow.initialize();
        p->set_enable_cursor(false);

        auto* resources = Controller::get<engine::resources::ResourcesController>();
        asphalt_texture = resources->texture(
            "asphalt", "resources/textures/road--avenue--street/textures/road_Polygon_1_BaseColor.png");
        ferrari_texture = resources->texture(
            "ferrari_paint", "resources/model/ferrari-f1-2019/textures/SF90_Main_BaseColor.png");
        street_lights_texture = resources->texture(
            "street_lights_palette",
            "resources/textures/street_low_poly_street_lights_v3/Palette256.png");
        formula_model = resources->model("formula");
        street_lights_model = resources->model("street_lights");


        skybox = resources->skybox("night_sky", "resources/skyboxes/night_sky");

        auto* c = Controller::get<engine::graphics::GraphicsController>()->camera();
        c->Position = {11, 6.4f, 17};
        c->Yaw = -127;
        c->Pitch = -16;
        c->rotate_camera(0, 0);

        spdlog::info("F1 Drag Race ready: SPACE starts the two-stage event sequence.");
    }

    bool MainController::loop()
    {
        return Controller::get<engine::platform::PlatformController>()
               ->key(engine::platform::KEY_ESCAPE).is_up();
    }

    void MainController::poll_events()
    {
    }

    void MainController::update()
    {
        const auto* p = Controller::get<engine::platform::PlatformController>();
        camera(*p);

        const float elapsed = p->frame_time().current - action;
        if (race == Race::Countdown && elapsed >= 2.f)
        {
            race = Race::Racing;
            spdlog::info("EVENT A: car accelerates.");
        }

        if (race == Race::Racing)
        {
            car_x += 5.2f * p->dt();
            if (elapsed >= 6.f)
            {
                race = Race::Safety;
                spdlog::info("EVENT B: car stops.");
            }
        }
    }

    void MainController::begin_draw()
    {
        auto* r = Controller::get<engine::resources::ResourcesController>();
        auto* d = r->shader("point_shadow_depth");

        shadow.begin_depth_pass(point_pos);
        d->use();
        for (int i = 0; i < 6; ++i)
            d->set_mat4("shadowMatrices[" + std::to_string(i) + "]", shadow.shadow_matrices()[i]);
        d->set_vec3("lightPos", point_pos);
        d->set_float("farPlane", shadow.far_plane());
        scene(d, true);

        const auto* p = Controller::get<engine::platform::PlatformController>();
        shadow.end_depth_pass(p->window()->width(), p->window()->height());
        bloom.begin_scene();
    }

    void MainController::draw()
    {
        auto* r = Controller::get<engine::resources::ResourcesController>();
        auto* s = r->shader("f1_scene");
        auto* g = Controller::get<engine::graphics::GraphicsController>();

        if (skybox)
            g->draw_skybox(r->shader("skybox"), skybox);

        s->use();
        s->set_mat4("projection", g->projection_matrix());
        s->set_mat4("view", g->camera()->view_matrix());
        s->set_vec3("viewPos", g->camera()->Position);
        s->set_vec3("pointPosition", point_pos);
        s->set_vec3("pointColor", {1.f, .72f, .35f});
        s->set_float("pointIntensity", point_intensity);
        s->set_vec3("spotPosition", {car_x + 2.19f, .35f, 0});
        s->set_vec3("spotDirection", {1, -.07f, 0});
        s->set_vec3("spotColor", {.76f, .86f, 1});
        s->set_float("spotCutoff", glm::cos(glm::radians(cone)));
        s->set_float("farPlane", shadow.far_plane());
        s->set_int("depthMap", 4);
        s->set_int("asphaltTexture", 2);
        s->set_int("texture_diffuse1", 0);
        s->set_int("streetLightsTexture", 3);

        asphalt_texture->bind_to_unit(2);
        street_lights_texture->bind_to_unit(3);
        shadow.bind(4);
        scene(s, false);
    }


    void MainController::end_draw()
    {
        auto* r = Controller::get<engine::resources::ResourcesController>();
        auto* blur = r->shader("bloom_blur");
        auto* final = r->shader("bloom_final");

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
    }

    void MainController::reset()
    {
    }

    void MainController::camera(const engine::platform::PlatformController& p)
    {
        if (!captured)
            return;

        auto* c = Controller::get<engine::graphics::GraphicsController>()->camera();
        float dt = p.dt();
        if (p.key(engine::platform::KEY_W).is_down())
            c->move_camera(engine::graphics::Camera::FORWARD, dt);
        if (p.key(engine::platform::KEY_S).is_down())
            c->move_camera(engine::graphics::Camera::BACKWARD, dt);
        if (p.key(engine::platform::KEY_A).is_down())
            c->move_camera(engine::graphics::Camera::LEFT, dt);
        if (p.key(engine::platform::KEY_D).is_down())
            c->move_camera(engine::graphics::Camera::RIGHT, dt);

        auto m = p.mouse();
        c->rotate_camera(m.dx, m.dy);
        c->zoom(m.scroll);
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
