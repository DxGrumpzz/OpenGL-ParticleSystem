#pragma once
#define GLM_CONSTEXPR_SIMD

#include <glm/vec2.hpp>
#include <glm/glm.hpp>


// Defined in Main.cpp
extern std::uint32_t MouseX;
extern std::uint32_t MouseY;

extern int WindowWidth;
extern int WindowHeight;


constexpr glm::vec2 CartesianToNDC(const glm::vec2& cartesianPosition)
{
    return
    {
        ((2.0f * cartesianPosition.x) / WindowWidth),
        ((2.0f * cartesianPosition.y) / WindowHeight),
    };
};

constexpr glm::vec2 ScreenToNDC(const glm::vec2& screenPosition)
{
    return
    {
        ((2.0f * screenPosition.x) / WindowWidth) - 1.0f,
        -(((2.0f * screenPosition.y) / WindowHeight) - 1.0f),
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



float RandomNumberGenerator(const glm::vec2& uv, float seed)
{
    const float fixedSeed = std::fabs(seed) + 1.0f;
    
    const float x = glm::dot(uv, glm::vec2(12.9898, 78.233) * fixedSeed);

    return glm::fract(glm::sin(x) * 43758.5453f);
};

float RandomNumberGenerator(const glm::vec2& uv, float seed, float min, float max)
{
    const float rng = RandomNumberGenerator(uv, seed);

    // Map [0, 1] to [min, max] 
    const float rngResult = min + rng * (max - min);

    return rngResult;
};


/// <summary>
/// A simple parabolic trajectory function, returns the next 'y' position of a particle depending on it's 'x' position
/// </summary>
/// <param name="particleX">  </param>
/// <param name="a"> No idea what this actually does, but it affects the trajectory somewhat </param>
/// <param name="b"> No idea what this actually does, but it affects the trajectory somewhat </param>
/// <returns></returns>
constexpr float ParticleTrajectoryFunction(const float particleX, float a = 1.0f, float b = 1.0f)
{
    return particleX * (((-a) * particleX) + b);
};
