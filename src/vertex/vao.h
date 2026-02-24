#include <glad/glad.h>
#include <vector>

template <typename vertex_attributes, size_t... attribute_sizes> class VertexArrayObject
{

  public:
    VertexArrayObject() : count(0)
    {
        glGenVertexArrays(1, &VAO);
    }
    void use()
    {
        glBindVertexArray(VAO);
    }
    void add_vbo(std::vector<vertex_attributes> &vertex_data)
    {
        use();
        unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertex_data.size() * sizeof(vertex_attributes),
            vertex_data.data(),
            GL_STATIC_DRAW
        );
        static constexpr size_t sizes[] = {attribute_sizes...};
        size_t offset = 0;
        for (size_t i = 0; i < sizeof...(attribute_sizes); i++)
        {
            glVertexAttribPointer(i, sizes[i], GL_FLOAT, GL_FALSE, sizeof(vertex_attributes), (void *)offset);
            offset += sizes[i] * sizeof(float);

            glEnableVertexAttribArray(i);
        }
        count += vertex_data.size();
        // Unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    void draw()
    {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }

  private:
    unsigned int VAO;
    unsigned int count;
};