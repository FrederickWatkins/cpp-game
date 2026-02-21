#pragma once
#include <cstdint>

const uint8_t ALPHA_OFFSET = 24;
const uint8_t RED_OFFSET = 16;
const uint8_t GREEN_OFFSET = 8;
const uint8_t BLUE_OFFSET = 0;

class Colour
{

  public:
    Colour(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
        : value(static_cast<uint32_t>(alpha) << ALPHA_OFFSET | static_cast<uint32_t>(red) << RED_OFFSET |
                static_cast<uint32_t>(green) << GREEN_OFFSET | static_cast<uint32_t>(blue) << BLUE_OFFSET) {};
    explicit Colour(uint32_t argb8888);

    [[nodiscard]] auto get_alpha() const -> uint8_t;
    [[nodiscard]] auto get_red() const -> uint8_t;
    [[nodiscard]] auto get_blue() const -> uint8_t;
    [[nodiscard]] auto get_green() const -> uint8_t;
    [[nodiscard]] auto argb8888() const -> uint32_t;

    auto set_alpha(uint8_t val);
    auto set_red(uint8_t val);
    auto set_blue(uint8_t val);
    auto set_green(uint8_t val);

  private:
    uint32_t value;
};
