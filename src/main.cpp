#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "shader/shader.h"
#include "vertex/vao.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define BOX_WIDTH 400
#define BOX_HEIGHT 400

struct XYZRGBA
{
    glm::vec3 position;
    glm::vec4 colour;
};

auto cube(float side_length) -> std::vector<XYZRGBA>
{
    float h = side_length / 2.0f;
    std::vector<XYZRGBA> vertices;
    vertices.reserve(36);

    // Cube corners
    glm::vec3 p0{-h, -h, h}, p1{h, -h, h}, p2{h, h, h}, p3{-h, h, h}, p4{-h, -h, -h}, p5{h, -h, -h}, p6{h, h, -h},
        p7{-h, h, -h};

    // CGA Color Palette (High Intensity)
    glm::vec4 cga_red{1.0f, 0.33f, 0.33f, 1.0f};
    glm::vec4 cga_green{0.33f, 1.0f, 0.33f, 1.0f};
    glm::vec4 cga_blue{0.33f, 0.33f, 1.0f, 1.0f};
    glm::vec4 cga_yellow{1.0f, 1.0f, 0.33f, 1.0f};
    glm::vec4 cga_magenta{1.0f, 0.33f, 1.0f, 1.0f};
    glm::vec4 cga_cyan{0.33f, 1.0f, 1.0f, 1.0f};

    auto addFace = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec4 color) {
        vertices.push_back({a, color});
        vertices.push_back({b, color});
        vertices.push_back({c, color});
        vertices.push_back({a, color});
        vertices.push_back({c, color});
        vertices.push_back({d, color});
    };

    // Construct faces with distinct colors
    addFace(p0, p1, p2, p3, cga_red);     // Front
    addFace(p5, p4, p7, p6, cga_green);   // Back
    addFace(p3, p2, p6, p7, cga_blue);    // Top
    addFace(p4, p5, p1, p0, cga_yellow);  // Bottom
    addFace(p4, p0, p3, p7, cga_magenta); // Left
    addFace(p1, p5, p6, p2, cga_cyan);    // Right

    return vertices;
}

auto load_mesh_to_vector(const std::string &path) -> std::vector<XYZRGBA>
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Couldn't open file" << std::endl;
        return {};
    }

    std::vector<XYZRGBA> vertices;

    // Iterate through all meshes in the file
    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh *mesh = scene->mMeshes[m];

        for (unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            aiFace &face = mesh->mFaces[f];
            for (unsigned int i = 0; i < face.mNumIndices; i++)
            {
                unsigned int index = face.mIndices[i];

                XYZRGBA v;
                v.position = {mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z};

                // If the mesh has colors, use them; otherwise, maybe color by Face ID?
                if (mesh->HasVertexColors(0))
                {
                    aiColor4D c = mesh->mColors[0][index];
                    v.colour = {c.r, c.g, c.b, c.a};
                }
                else
                {
                    v.colour = {0.5 - (v.position.z / 100.0f), 0.33f, 0.5 + v.position.z / 100.0f, 1.0f};
                }
                vertices.push_back(v);
            }
        }
    }

    return vertices;
}

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

    using GLContextType = std::remove_pointer_t<SDL_GLContext>;

    auto context = std::unique_ptr<GLContextType, decltype(&SDL_GL_DestroyContext)>(
        SDL_GL_CreateContext(window.get()),
        SDL_GL_DestroyContext
    );

    if (!context)
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

    auto worldspace_shader = ShaderProgram(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // std::vector<XYZRGBA> vertices = cube(2.0f);
    std::vector<XYZRGBA> vertices = load_mesh_to_vector("teapot.obj");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto vao1 = std::make_unique<VertexArrayObject<XYZRGBA, 3, 4>>();
    vao1->add_vbo(vertices);

    bool quit = false;
    SDL_Event event;

    auto start_time = std::chrono::system_clock::now().time_since_epoch();

    std::array modes = {GL_LINE, GL_POINT, GL_FILL};
    size_t mode = 2;

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
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE)
            {
                mode++;
                if (mode == 3)
                {
                    mode = 0;
                }
            }
        }

        glPolygonMode(GL_FRONT_AND_BACK, modes[mode]);

        GLfloat curr_time =
            std::chrono::duration<float>(std::chrono::system_clock::now().time_since_epoch() - start_time).count();
        GLfloat y_offset = (sin(curr_time / 1.5f) / 2.0f);

        glm::mat4 rotation_mat =
            glm::rotate(glm::mat4(1.0f), glm::radians(30.0f * curr_time), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 view_mat = glm::lookAt(
            30.0f * glm::vec3(4.0f, 3 * y_offset + 2.0f, 1.0f),
            glm::vec3(0.0f, 30.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        glm::mat4 proj_mat = glm::perspective(
            glm::radians(70.0f),
            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
            1.0f,
            100000.0f
        );
        glm::mat4 transform_mat = proj_mat * view_mat;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 combined_transform_mat =
            transform_mat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * rotation_mat;

        worldspace_shader.use();
        vao1->use();
        GLint combined_transform_mat_location = worldspace_shader.get_uniform_location("combined_transform_mat");
        glUniformMatrix4fv(combined_transform_mat_location, 1, false, glm::value_ptr(combined_transform_mat));
        vao1->draw();
        for (float i = 100.0f; i <= 1000.0f; i += 100.0f)
        {
            for (float j = -500.0f; j <= 500.0f; j += 100.0f)
            {
                for (float k = -300.0f; k <= 100.0f; k += 100.0f)
                {
                    combined_transform_mat =
                        transform_mat * glm::translate(glm::mat4(1.0f), glm::vec3(-i, k, j)) * rotation_mat;
                    glUniformMatrix4fv(
                        combined_transform_mat_location,
                        1,
                        false,
                        glm::value_ptr(combined_transform_mat)
                    );
                    vao1->draw();
                }
            }
        }

        // Present the backbuffer to the screen
        SDL_GL_SwapWindow(window.get());

        i++;
    }
    SDL_Quit();
    return 0;
}
