#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <memory>

#include "fragment_shader.h"
#include "shader.h" // IWYU pragma: keep

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define BOX_WIDTH 400
#define BOX_HEIGHT 400

auto create_shader_program(const char *vertex_shader_source, const char *fragment_shader_source) -> unsigned int;

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

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    std::array vertices = {-0.5F, -0.5F, 0.0F, 0.5F, -0.5F, 0.0F, 0.0F, 0.5F, 0.0F};
    std::array vertices2 = {0.0F, 0.0F, -0.5F, 0.5F, 0.0F, 0.5F, 0.0F, 0.5F, 0.5F};

    auto shader_program = create_shader_program(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    unsigned int VAO = 0;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO = 0;
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(0);

    unsigned int VAO2 = 0;
    glGenVertexArrays(1, &VAO2);
    VBO = 0;
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

                std::cout << ((i * 1000000000) / (std::chrono::system_clock::now().time_since_epoch() - start_time).count())
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

        glUseProgram(shader_program);
        glBindVertexArray(VAO);
        GLint vertex_colour_location = glGetUniformLocation(shader_program, "ourColour");
        glUniform4f(vertex_colour_location, red_value, 0.0f, 0.0f, 1.0f);
        GLint offset_location = glGetUniformLocation(shader_program, "offset");
        glUniform3f(offset_location, 0.0f, 0.0f, z_offset);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glUniform4f(vertex_colour_location, 0.0f, 1.0f - red_value, 0.0f, 1.0f);
        glUniform3f(offset_location, -0.5f, z_offset, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shader_program);
        glBindVertexArray(VAO2);
        glUniform4f(vertex_colour_location, 0.0f, 0.0f, 1.0f, 1.0f);
        glUniform3f(offset_location, 0.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Present the backbuffer to the screen
        SDL_GL_SwapWindow(window.get());

        i++;
    }
    SDL_Quit();
    return 0;
}

auto create_shader_program(const char *vertex_shader_source, const char *fragment_shader_source) -> unsigned int
{
    unsigned int vertexShader = 0;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertexShader);

    int vertex_success = 0;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_success);
    if (vertex_success == 0)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = 0;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragmentShader);

    int fragment_success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragment_success);
    if (fragment_success == 0)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = 0;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int link_success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_success);
    if (link_success == 0)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::LINK_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}
