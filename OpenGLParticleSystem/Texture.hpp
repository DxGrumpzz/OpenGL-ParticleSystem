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

    /// <summary>
    /// Generate a texture
    /// </summary>
    /// <param name="texturePath"> Path to texture resource </param>
    /// <param name="keepBound"> Should the texture be kept bound after initializing </param>
    Texture(const std::string_view& texturePath, bool keepBound = false)
    {
        // For some reason, when generating more than 1 texture with 'keepBound=true', causes massive FPS drops
        _textureID = GL::GenerateTexture(texturePath, keepBound);
    };

    ~Texture()
    {
        if(_textureID != 0)
            glDeleteTextures(1, &_textureID);
    };

public:

    void Bind(const std::uint32_t& textureUnit = 0) const
    {
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glActiveTexture(GL_TEXTURE0 + textureUnit);
    };
};