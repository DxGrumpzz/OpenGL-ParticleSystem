#pragma once

#include <glm/vec2.hpp>


// Defined in Main.cpp
extern std::uint32_t MouseX;
extern std::uint32_t MouseY;

extern int WindowWidth;
extern int WindowHeight;



constexpr glm::vec2 CartesianToNDC(const glm::vec2& position)
{
    return
    {
        ((2.0f * position.x) / WindowWidth),
        ((2.0f * position.y) / WindowHeight),
    };
};

constexpr glm::vec2 ScreenToNDC(const glm::vec2& position)
{
    return
    {
        ((2.0f * position.x) / WindowWidth) - 1.0f,
        -(((2.0f * position.y) / WindowHeight) - 1.0f),
    };
};

constexpr glm::vec2 MouseToNDC()
{
    return ScreenToNDC({ MouseX, MouseY });
};

constexpr glm::vec2 ScreenToCartesian(const glm::vec2& screenPosition)
{
    return
    {
        screenPosition.x - (WindowWidth / 2.0f),
        -(screenPosition.y - (WindowHeight / 2.0f)),
    };
};

constexpr glm::vec2 MouseToCartesian()
{
    return ScreenToCartesian({ MouseX, MouseY });
};