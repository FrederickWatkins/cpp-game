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

#include "mesh/mesh.h"
#include "shader/shader.h"
#include "vertex/vao.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

// NOLINTBEGIN

template <bool has_colour, bool has_normal, size_t num_tex_coords>
auto cube(float side_length) -> std::vector<VertexAttributes<has_colour, has_normal, num_tex_coords>>
{
    using Vertex = VertexAttributes<has_colour, has_normal, num_tex_coords>;
    float half_length = side_length / 2.0F;
    std::vector<Vertex> vertices;
    const size_t num_vertices = 36;
    vertices.reserve(num_vertices);

    // 8 Corner points
    glm::vec3 point0{-half_length, -half_length, half_length};
    glm::vec3 point1{half_length, -half_length, half_length};
    glm::vec3 point2{half_length, half_length, half_length};
    glm::vec3 point3{-half_length, half_length, half_length};
    glm::vec3 point4{-half_length, -half_length, -half_length};
    glm::vec3 point5{half_length, -half_length, -half_length};
    glm::vec3 point6{half_length, half_length, -half_length};
    glm::vec3 point7{-half_length, half_length, -half_length};

    // CGA Palette for faces
    const std::array<glm::vec4, 6> cga = {{
        {1.0F, 0.0F, 0.0F, 1.0F},
        {0.0F, 1.0F, 0.0F, 1.0F},
        {0.0F, 0.0F, 1.0F, 1.0F},
        {0.75F, 0.75F, 0.0F, 1.0F},
        {0.75F, 0.0F, 0.75F, 1.0F},
        {0.0F, 0.75F, 0.75F, 1.0F},
    }};

    // UV coordinates for a single face
    const std::array<glm::vec3, 4> uvs = {{
        {0.0F, 1.0F, 0.0F},
        {1.0F, 1.0F, 0.0F},
        {1.0F, 0.0F, 0.0F},
        {0.0F, 0.0F, 0.0F},
    }};

    auto add_face = [&](std::array<glm::vec3, 4> points, glm::vec3 norm, size_t face_idx) {
        auto push = [&](glm::vec3 pos, glm::vec3 tex_coords) {
            Vertex vertex;
            vertex.position = pos;
            if constexpr (has_normal)
            {
                vertex.normal = norm;
            }
            if constexpr (has_colour)
            {
                vertex.colour = cga[face_idx];
            }
            if constexpr (num_tex_coords >= 1)
            {
                vertex.tex_coords[0] = tex_coords[0];
            }
            if constexpr (num_tex_coords >= 2)
            {
                vertex.tex_coords[1] = tex_coords[1];
            }
            if constexpr (num_tex_coords >= 3)
            {
                vertex.tex_coords[2] = tex_coords[2];
            }
            vertices.push_back(vertex);
        };

        // Two triangles per face
        push(points[0], uvs[0]);
        push(points[1], uvs[1]);
        push(points[2], uvs[2]);
        push(points[0], uvs[0]);
        push(points[2], uvs[2]);
        push(points[3], uvs[3]);
    };

    // Define faces: Front, Back, Top, Bottom, Left, Right
    add_face(std::array{point0, point1, point2, point3}, {0, 0, 1}, 0);
    add_face(std::array{point5, point4, point7, point6}, {0, 0, -1}, 1);
    add_face(std::array{point3, point2, point6, point7}, {0, 1, 0}, 2);
    add_face(std::array{point4, point5, point1, point0}, {0, -1, 0}, 3);
    add_face(std::array{point4, point0, point3, point7}, {-1, 0, 0}, 4);
    add_face(std::array{point1, point5, point6, point2}, {1, 0, 0}, 5);

    return vertices;
}

template <bool has_colour, bool has_normal, size_t num_tex_coords, typename... uniforms>
auto load_mesh(const std::string &path, std::shared_ptr<ShaderProgram> shader_program)
    -> std::vector<Mesh<has_colour, has_normal, num_tex_coords, uniforms...>>
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

    for (unsigned int mesh_idx = 0; mesh_idx < scene->mNumMeshes; mesh_idx++)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        meshes.push_back(
            Mesh<has_colour, has_normal, num_tex_coords, uniforms...>(*scene->mMeshes[mesh_idx], shader_program)
        );
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

    auto worldspace_shader = std::make_shared<ShaderProgram>(shader_wcnn());
    auto ndcspace_shader = std::make_shared<ShaderProgram>(shader_ncnn());

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto vertices = cube<true, false, 0>(50.0f);
    auto mesh1 = std::make_unique<Mesh<true, false, 0, glm::mat4>>(vertices, worldspace_shader);
    //auto mesh1 = std::make_unique<Mesh<true, false, 0, glm::mat4>>(
    //    load_mesh<true, false, 0, glm::mat4>("teapot2.obj", worldspace_shader)[0]
    // );

    auto triangle_vertices = std::vector<VertexAttributes<true, false, 0>>({
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
        {{1.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
        {{0.75f, 0.75f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
    });
    auto ndc_mesh = std::make_unique<Mesh<true, false, 0>>(triangle_vertices, ndcspace_shader);

    bool quit = false;
    SDL_Event event;

    auto start_time = std::chrono::system_clock::now().time_since_epoch();

    std::array modes = {GL_LINE, GL_POINT, GL_FILL};
    size_t mode = 2;

    int window_width = WINDOW_WIDTH;
    int window_height = WINDOW_HEIGHT;

    size_t frames = 0;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {

                std::cout << ((frames * 1000000000) /
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

        std::array<GLint, 1> uniform_locations = {worldspace_shader->get_uniform_location("combined_transform_mat")};
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
        std::array<GLint, 0> ndc_uniform_locations = {};
        ndc_mesh->draw(ndc_uniform_locations);

        // Present the backbuffer to the screen
        SDL_GL_SwapWindow(window.get());

        frames++;
    }
    SDL_Quit();
    return 0;
}
// NOLINTEND
