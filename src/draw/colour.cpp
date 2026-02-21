#include "colour.h"
#include <cstdint>

#define BYTE_MASK 0xFF

auto Colour::get_alpha() const -> uint8_t
{
    return static_cast<uint8_t>(value >> ALPHA_OFFSET);
}

auto Colour::get_red() const -> uint8_t
{
    return static_cast<uint8_t>(value >> RED_OFFSET);
}

auto Colour::get_green() const -> uint8_t
{
    return static_cast<uint8_t>(value >> GREEN_OFFSET);
}

auto Colour::get_blue() const -> uint8_t
{
    return static_cast<uint8_t>(value >> BLUE_OFFSET);
}

auto Colour::set_alpha(uint8_t val)
{
    value &= ~(BYTE_MASK << ALPHA_OFFSET);
    value |= static_cast<uint32_t>(val) << ALPHA_OFFSET;
}

auto Colour::set_red(uint8_t val)
{
    value &= ~(BYTE_MASK << RED_OFFSET);
    value |= static_cast<uint32_t>(val) << RED_OFFSET;
}

auto Colour::set_green(uint8_t val)
{
    value &= ~(BYTE_MASK << GREEN_OFFSET);
    value |= static_cast<uint32_t>(val) << GREEN_OFFSET;
}

auto Colour::set_blue(uint8_t val)
{
    value &= ~(BYTE_MASK << BLUE_OFFSET);
    value |= static_cast<uint32_t>(val) << BLUE_OFFSET;
}

auto Colour::argb8888() const -> uint32_t
{
    return value;
}