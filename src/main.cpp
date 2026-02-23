#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <chrono>
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <memory>

#include "draw/buffer.h"
#include "draw/cga_colour.h"
#include "draw/colour.h"
#include "render/mesh.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400
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

    auto texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
        SDL_CreateTexture(
            renderer.get(),
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            WINDOW_WIDTH,
            WINDOW_HEIGHT
        ),
        SDL_DestroyTexture
    );

    if (!texture)
    {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    auto buffer = std::make_unique<Buffer>(WINDOW_WIDTH, WINDOW_HEIGHT);
    size_t colour = 0;
    std::array<Colour, 6> colours = {BLUE, RED, GREEN, MAGENTA, YELLOW, CYAN};

    WorldSpaceMesh world_mesh;
    // 8 corners of a cube centered at origin
    std::vector<glm::vec3> verts = {
        {-0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f}
    };

    // Helper to add faces (2 triangles per face)
    auto add_face = [&](int a, int b, int c, int d) {
        auto triangle1 = glm::vec<3, glm::vec3>(verts[a], verts[b], verts[c]);
        auto triangle2 = glm::vec<3, glm::vec3>(verts[a], verts[c], verts[d]);
        world_mesh.add_triangle(triangle1, colours[colour]); // Note: Assuming your API handles vec3 inputs
        world_mesh.add_triangle(triangle2, colours[colour]);
        colour++;
    };

    add_face(0, 1, 2, 3); // Front
    add_face(1, 5, 6, 2); // Right
    add_face(5, 4, 7, 6); // Back
    add_face(4, 0, 3, 7); // Left
    add_face(3, 2, 6, 7); // Top
    add_face(4, 5, 1, 0); // Bottom

    auto up = glm::vec3(0.0f, 1.0f, 0.0f);
    auto aspect_ratio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
    float angle = 0.0f;
    const float rotation_speed = 0.8f; // Radians per second
    const float radius = 3.0f;
    float elevation_speed = 5.0f;
    float elevation = 1.0f;

    bool quit = false;
    SDL_Event event;

    auto last_time = std::chrono::high_resolution_clock::now();

    while (!quit)
    {
        for (size_t x = 0; x < WINDOW_WIDTH; x++)
        {
            for (size_t y = 0; y < WINDOW_HEIGHT; y++)
            {
                buffer->write_pixel(x, y, BLACK);
            }
        }
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - last_time;
        float dt = elapsed.count();
        last_time = current_time;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_UP)
            {
                elevation += elevation_speed * dt;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_DOWN)
            {
                elevation -= elevation_speed * dt;
            }
        }

        angle += rotation_speed * dt;

        auto pos = glm::vec3(radius * cos(angle), elevation, radius * sin(angle));

        auto camera = Camera(pos, -pos, 70, aspect_ratio, 0.1f, 1000.0f, up);

        auto clip_mesh = world_mesh.to_clip(camera);
        auto ndc_mesh = clip_mesh.to_ndc();
        ndc_mesh.write_to_buffer(*buffer.get());
        SDL_RenderTexture(renderer.get(), texture.get(), NULL, NULL);

        if (!SDL_UpdateTexture(texture.get(), NULL, &buffer->buffer.front(), WINDOW_WIDTH * 4))
        {
            std::cerr << "SDL_UpdateTexture Error: " << SDL_GetError() << std::endl;
        }

        // Present the backbuffer to the screen
        SDL_RenderPresent(renderer.get());
    }

    SDL_Quit();
    return 0;
}