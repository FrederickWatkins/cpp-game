#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <vector>

#include "shader/shader.h"
#include "vertex/vao.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BOX_WIDTH 400
#define BOX_HEIGHT 400

auto main() -> int
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
        SDL_CreateWindow("SDL3 Window", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL),
        SDL_DestroyWindow
    );

    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    auto renderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(
        SDL_CreateRenderer(window.get(), "opengl"),
        SDL_DestroyRenderer
    );

    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0)
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    auto ndc_shader = ShaderProgram(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    std::vector<std::array<float, 7>> vertices = {
        {{-0.5F, -0.5F, 0.0F, 0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.5F, -0.5F, 0.0F, 1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.0F, 0.5F, 0.0F, 0.0f, 0.0f, 1.0f, 1.0f}},
    };
    std::vector<std::array<float, 7>> vertices2 = {
        {0.0F, 0.0F, -0.5F, 0.0f, 0.0f, 1.0f, 1.0f},
        {0.5F, 0.0F, 0.5F, 0.0f, 0.0f, 1.0f, 1.0f},
        {0.0F, 0.5F, 0.5F, 0.0f, 0.0f, 1.0f, 1.0f},
    };

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto vao1 = std::make_unique<VertexArrayObject<std::array<float, 7>, 3, 4>>();
    vao1->add_vbo(vertices);

    auto vao2 = std::make_unique<VertexArrayObject<std::array<float, 7>, 3, 4>>();
    vao2->add_vbo(vertices2);

    bool quit = false;
    SDL_Event event;

    auto start_time = std::chrono::system_clock::now().time_since_epoch();

    size_t i = 0;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {

                std::cout << ((i * 1000000000) /
                              (std::chrono::system_clock::now().time_since_epoch() - start_time).count())
                          << std::endl;
                quit = true;
            }
        }
        GLfloat curr_time =
            std::chrono::duration<float>(std::chrono::system_clock::now().time_since_epoch() - start_time).count();
        GLfloat red_value = (sin(curr_time) / 4.0f) + 0.5f;
        GLfloat z_offset = (sin(curr_time * 2.0f) / 2.0f);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ndc_shader.use();
        vao1->use();
        GLint vertex_colour_location = ndc_shader.get_uniform_location("ourColour");
        glUniform4f(vertex_colour_location, red_value, 0.0f, 0.0f, 1.0f);
        GLint offset_location = ndc_shader.get_uniform_location("offset");
        glUniform3f(offset_location, 0.0f, 0.0f, z_offset);
        vao1->draw();
        glUniform4f(vertex_colour_location, 0.0f, 1.0f - red_value, 0.0f, 1.0f);
        glUniform3f(offset_location, -0.5f, z_offset, 0.0f);
        vao1->draw();

        ndc_shader.use();
        vao2->use();
        glUniform4f(vertex_colour_location, 0.0f, 0.0f, 1.0f, 1.0f);
        glUniform3f(offset_location, 0.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        vao2->draw();

        // Present the backbuffer to the screen
        SDL_GL_SwapWindow(window.get());

        i++;
    }
    SDL_Quit();
    return 0;
}
