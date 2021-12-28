#pragma once

#include <cstdint>
#include <string_view>
#include <glad/glad.h>

#include "GLUtilities.hpp"

/// <summary>
/// A simple encapsulation of a GL texture 
/// </summary>
class Texture
{
private:

    std::uint32_t _textureID = 0;

public:

    Texture(const std::string_view& texturePath)
    {
        _textureID = GL::GenerateTexture(texturePath, true);
    };

public:

    void Bind(const std::uint32_t& textureUnit = 0) const
    {
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glActiveTexture(GL_TEXTURE0 + textureUnit);
    };
};