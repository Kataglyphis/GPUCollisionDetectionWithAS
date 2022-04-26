
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

// all IMGUI stuff
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>

#include "Window.h"
#include "App.h"
#include "SceneLoadInfo.h"



struct start_params
{
    int window_width                        = 1200;
    int window_height                       = 768;
    //glm::vec3 start_position                = glm::vec3(0.0f, 0.0, -5.0);//glm::vec3(0.0f, 100.0f, -80.0f);
    glm::vec3 start_position                = glm::vec3(0.0f, -0.5, 0.5);//glm::vec3(0.0f, 100.0f, -80.0f);
    float near_plane                        = 0.1f;
    float far_plane                         = 10000.f;
    float angle                             = 0.0f;
    float fov                               = 60.f;
    glm::vec3 start_up                      = glm::vec3(0.0f, 1.0f, 0.0f);
    float start_yaw                         = 80.f;
    float start_pitch                       = -40.0f;
    float start_move_speed                  = 1.0f;
    float start_turn_speed                  = 0.25f;
    glm::vec3 directional_light_direction   = { -1.f,1.f, 1.f };

#ifdef _DEBUG
    //SceneLoadInfo test_scene = sunTemple;
    //SceneLoadInfo test_scene = cube;
    SceneLoadInfo test_scene = vikingRoom;
#else
    SceneLoadInfo test_scene = sunTemple;
    //SceneLoadInfo test_scene = vikingRoom;
#endif
};



int main() {

    start_params p = start_params();
    std::shared_ptr<Window> main_window = std::make_shared<Window>(p.window_width, p.window_height);
    main_window->initialize();


    float delta_time = 0.0f;
    float last_time = 0.0f;
   
    Camera camera{
        p.start_position,
        p.start_up,
        p.start_yaw,
        p.start_pitch,
        p.start_move_speed,
        p.start_turn_speed,
        p.near_plane,
        p.far_plane,
        p.fov };


    App app{};


    // Init App
    if (app.init(
            main_window,
            &camera,
            p.directional_light_direction,
            p.test_scene
        ) == EXIT_FAILURE) {

        return EXIT_FAILURE;

    }

    AppInput input = {};
    std::vector<bool>keysLastFrame;
    bool* keysThisFrame = main_window->get_keys();

    keysLastFrame.assign(keysThisFrame, keysThisFrame + 1024);

    while (!main_window->get_should_close()) {

        //poll all events incoming from user
        keysLastFrame.assign(keysThisFrame, keysThisFrame + 1024);
        glfwPollEvents();
        keysThisFrame = main_window->get_keys();

        // handle events for the camera
        camera.key_control(keysThisFrame, delta_time);
        camera.mouse_control(main_window->get_x_change(), main_window->get_y_change());


        // Timing
        float now = static_cast<float>(glfwGetTime());
        delta_time = now - last_time;
        last_time = now;


        input.triggerShaderReload = (keysLastFrame[GLFW_KEY_R] != GLFW_PRESS) && (keysThisFrame[GLFW_KEY_R] == GLFW_PRESS);

        // Run Frame
        app.runFrame(input);

    }

    app.clean_up();

    return EXIT_SUCCESS;

}