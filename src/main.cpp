#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <chrono>
#include <glad/glad.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "scene/mesh.hpp"
#include "scene/scene.hpp"
#include "shader/shader.hpp"

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

template <bool NDC, bool has_colour, bool has_lighting, size_t num_tex_coords>
auto load_scene(
    const std::string &path
) -> std::vector<Mesh<has_colour, has_lighting, num_tex_coords>>
{
    Assimp::Importer importer;

    // Logic: Always triangulate. Generate normals only if requested.
    unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals;

    const aiScene *scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("Assimp: " + std::string(importer.GetErrorString()));
    }

    std::vector<Mesh<has_colour, has_lighting, num_tex_coords>> meshes;

    for (unsigned int mesh_idx = 0; mesh_idx < scene->mNumMeshes; mesh_idx++)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        meshes.push_back(Mesh<has_colour, has_lighting, num_tex_coords>(*scene->mMeshes[mesh_idx]));
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

    auto lighting_shader = std::make_shared<ShaderProgram<false, true, true, 0>>();
    auto lighting_uniforms = lighting_shader->get_uniforms();
    auto worldspace_shader = std::make_shared<ShaderProgram<false, true, false, 0>>();
    auto worldspace_uniforms = worldspace_shader->get_uniforms();
    auto ndcspace_shader = std::make_shared<ShaderProgram<true, true, false, 0>>();
    auto ndc_uniforms = ndcspace_shader->get_uniforms();

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto camera = std::make_shared<Camera>(
        glm::vec3(90.0f, 60.0f, 60.0f),
        glm::quatLookAt(glm::normalize(glm::vec3(-30.0f, -10.0f, -20.0f)), glm::vec3(0.0f, 1.0f, 0.0f)),
        70.0f,
        16.0f / 9.0f,
        1.0f,
        1000.0f
    );
    auto light = std::make_shared<Light>();
    auto scene = Scene(camera, light);

    auto vertices = cube<true, false, 0>(50.0f);
    auto worldspace_mesh = std::make_shared<Mesh<true, false, 0>>(
    load_scene<false, true, false, 0>("teapot2.obj")[0]
    );
    // auto worldspace_mesh = std::make_shared<Mesh<true, false, 0>>(vertices);
    auto worldspace_node =
        std::make_shared<Node<false, true, false, 0>>(worldspace_mesh, worldspace_shader, glm::mat4(1));
    scene.add_node(worldspace_node);

    auto triangle_vertices = std::vector<VertexAttributes<true, false, 0>>({
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
        {{1.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
        {{0.75f, 0.75f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {}},
    });
    auto ndc_mesh = std::make_shared<Mesh<true, false, 0>>(triangle_vertices);
    auto ndc_node = std::make_shared<Node<true, true, false, 0>>(ndc_mesh, ndcspace_shader, glm::mat4(1));
    scene.add_node(ndc_node);

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


        camera->set_aspect_ratio(static_cast<float>(window_width) / static_cast<float>(window_height));

        GLfloat curr_time =
            std::chrono::duration<float>(std::chrono::system_clock::now().time_since_epoch() - start_time).count();
        worldspace_node->set_transform(glm::rotate(glm::mat4(1), glm::radians(30.0f * curr_time), glm::vec3(0.0f, 1.0f, 0.0f)));

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.draw();

        // Present the backbuffer to the screen
        SDL_GL_SwapWindow(window.get());

        frames++;
    }
    SDL_Quit();
    return 0;
}
// NOLINTEND
