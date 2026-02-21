#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <cstddef>
#include <iostream>
#include <memory>

#include "draw/buffer.h"
#include "draw/cga_colour.h"
#include "draw/colour.h"

auto main() -> int
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
        SDL_CreateWindow("SDL3 Window", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL),
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
        SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 800, 600),
        SDL_DestroyTexture
    );

    if (!texture)
    {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    auto buffer = std::make_unique<Buffer>(800, 600);
    auto colour = LIGHT_RED;

    bool quit = false;
    SDL_Event event;

    size_t x = 0;
    size_t y = 0;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB)
            {
                colour = BLUE;
            }
        }
        SDL_RenderTexture(renderer.get(), texture.get(), NULL, NULL);

        buffer->write_pixel(x, y, colour);

        if (!SDL_UpdateTexture(texture.get(), NULL, &buffer->buffer.front(), 800 * 4))
        {
            std::cerr << "SDL_UpdateTexture Error: " << SDL_GetError() << std::endl;
        }

        // Present the backbuffer to the screen
        SDL_RenderPresent(renderer.get());

        y++;
        if (y >= 600)
        {
            y = 0;
            x++;
            if (x >= 800)
            {
                x = 0;
            }
        }
    }

    SDL_Quit();
    return 0;
}