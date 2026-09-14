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
    }

    void MainController::start()
    {
    }

    void MainController::reset()
    {
    }

    void MainController::camera(const engine::platform::PlatformController& p)
    {
    }

    void MainController::box(const Shader* s, glm::vec3 p, glm::vec3 scale, glm::vec3 col,
                             float emission, bool depth, int surface_mode) const
    {
    }

    void MainController::formula(const Shader* s, bool d) const
    {
    }

    void MainController::headlights(const engine::resources::Shader* s, bool d, float height,
                                    float lateral_offset, float housing_x, float lens_x) const
    {
    }

    void MainController::lamp(const engine::resources::Shader* s, float x, float z, bool d) const
    {
    }

    void MainController::asphalt(const engine::resources::Shader* s, bool d) const
    {
    }

    void MainController::scene(const engine::resources::Shader* s, bool d) const
    {
    }
}
