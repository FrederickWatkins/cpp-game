#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
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
#include "mesh/mesh.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define BOX_WIDTH 400
#define BOX_HEIGHT 400

template <bool has_colour, bool has_normal, size_t num_tex_coords>
auto cube(float side_length) -> std::vector<VertexAttributes<has_colour, has_normal, num_tex_coords>>
{
    using Vertex = VertexAttributes<has_colour, has_normal, num_tex_coords>;
    float h = side_length / 2.0f;
    std::vector<Vertex> vertices;
    vertices.reserve(36);

    // 8 Corner points
    glm::vec3 p0{-h, -h, h}, p1{h, -h, h}, p2{h, h, h}, p3{-h, h, h}, p4{-h, -h, -h}, p5{h, -h, -h}, p6{h, h, -h},
        p7{-h, h, -h};

    // CGA Palette for faces
    const glm::vec4 cga[] = {
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {0.75f, 0.75f, 0.0f, 1.0f},
        {0.75f, 0.0f, 0.75f, 1.0f},
        {0.0f, 0.75f, 0.75f, 1.0f}
    };

    // UV coordinates for a single face
    const glm::vec3 uvs[] = {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}};

    auto addFace = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 norm, int faceIdx) {
        auto push = [&](glm::vec3 pos, glm::vec3 tex_coords) {
            Vertex v;
            v.position = pos;
            if constexpr (has_normal)
                v.normal = norm;
            if constexpr (has_colour)
                v.colour = cga[faceIdx];
            if constexpr (num_tex_coords >= 1)
                v.tex_coords[0] = tex_coords[0];
            if constexpr (num_tex_coords >= 2)
                v.tex_coords[1] = tex_coords[1];
            if constexpr (num_tex_coords >= 3)
                v.tex_coords[2] = tex_coords[2];
            vertices.push_back(v);
        };

        // Two triangles per face
        push(a, uvs[0]);
        push(b, uvs[1]);
        push(c, uvs[2]);
        push(a, uvs[0]);
        push(c, uvs[2]);
        push(d, uvs[3]);
    };

    // Define faces: Front, Back, Top, Bottom, Left, Right
    addFace(p0, p1, p2, p3, {0, 0, 1}, 0);
    addFace(p5, p4, p7, p6, {0, 0, -1}, 1);
    addFace(p3, p2, p6, p7, {0, 1, 0}, 2);
    addFace(p4, p5, p1, p0, {0, -1, 0}, 3);
    addFace(p4, p0, p3, p7, {-1, 0, 0}, 4);
    addFace(p1, p5, p6, p2, {1, 0, 0}, 5);

    return vertices;
}

template <bool has_colour, bool has_normal, size_t num_tex_coords, typename... uniforms>
auto load_mesh(const std::string &path, ShaderProgram &shader_program) -> std::vector<Mesh<has_colour, has_normal, num_tex_coords, uniforms...>>
{
    Assimp::Importer importer;

    // Logic: Always triangulate. Generate normals only if requested.
    unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals;

    const aiScene *scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("Assimp: " + std::string(importer.GetErrorString()));
    }

    std::vector<Mesh<has_colour, has_normal, num_tex_coords, uniforms...>> meshes;

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        meshes.push_back(Mesh<has_colour, has_normal, num_tex_coords, uniforms...>(*scene->mMeshes[m], shader_program));
    }
    return meshes;
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

    auto worldspace_shader = shader_wcnn();
    auto ndcspace_shader = shader_ncnn();

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto vertices = cube<true, false, 0>(50.0f);
    //auto mesh1 = std::make_unique<Mesh<true, false, 0, glm::mat4>>(vertices, worldspace_shader);
    auto mesh1 = std::make_unique<Mesh<true, false, 0, glm::mat4>>(load_mesh<true, false, 0, glm::mat4>("teapot2.obj", worldspace_shader)[0]);

    auto ndc_vao = std::make_unique<VertexArrayObject<true, false, 0>>();
    auto triangle_vertices = std::vector<VertexAttributes<true, false, 0>>({
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
        {{1.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
        {{0.75f, 0.75f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
    });
    ndc_vao->add_vbo(triangle_vertices);

    bool quit = false;
    SDL_Event event;

    auto start_time = std::chrono::system_clock::now().time_since_epoch();

    std::array modes = {GL_LINE, GL_POINT, GL_FILL};
    size_t mode = 2;

    size_t window_width = WINDOW_WIDTH;
    size_t window_height = WINDOW_HEIGHT;

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
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                window_width = event.window.data1;
                window_height = event.window.data2;
                glViewport(0, 0, window_width, window_height);
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
            static_cast<float>(window_width) / static_cast<float>(window_height),
            10.0f,
            100000.0f
        );
        glm::mat4 transform_mat = proj_mat * view_mat;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 combined_transform_mat =
            transform_mat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * rotation_mat;

        std::array<GLint, 1> uniform_locations = {worldspace_shader.get_uniform_location("combined_transform_mat")};
        mesh1->draw(uniform_locations, combined_transform_mat);
        for (float i = 100.0f; i <= 1000.0f; i += 100.0f)
        {
            for (float j = -500.0f; j <= 500.0f; j += 100.0f)
            {
                for (float k = -300.0f; k <= 100.0f; k += 100.0f)
                {
                    combined_transform_mat =
                        transform_mat * glm::translate(glm::mat4(1.0f), glm::vec3(-i, k, j)) * rotation_mat;
                    mesh1->draw(uniform_locations, combined_transform_mat);
                }
            }
        }
        ndcspace_shader.use();
        ndc_vao->use();
        ndc_vao->draw();

        // Present the backbuffer to the screen
        SDL_GL_SwapWindow(window.get());

        i++;
    }
    SDL_Quit();
    return 0;
}
