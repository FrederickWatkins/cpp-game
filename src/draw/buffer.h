#pragma once
#include <cstddef>
#include <vector>

#include "cga_colour.h"
#include "colour.h"

/// A 2D pixel buffer
class Buffer
{
  public:
    Buffer(size_t width, size_t height)
        : width(width), height(height), buffer(std::vector<Colour>(width * height, BLACK)) {};

    inline void write_pixel(size_t x_pos, size_t y_pos, Colour colour)
    {
        buffer[y_pos * width + x_pos] = colour;
    };

    inline auto read_pixel(size_t x_pos, size_t y_pos) -> Colour
    {
        return buffer[y_pos * width + x_pos];
    };

    size_t width;
    size_t height;
    std::vector<Colour> buffer;
};