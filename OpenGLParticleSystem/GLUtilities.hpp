#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <cassert>
#include <iostream>
#include <stb_image.h>


namespace GLUtilities
{

    enum class AccessType
    {
        ReadOnly = 0,
        WriteOnly = 1,
        ReadWrite = 2,
    };


    /// <summary>
    /// Returns the size in bytes of a numeric type defined by the APi
    /// </summary>
    /// <param name="glTypeID"></param>
    /// <returns></returns>
    static constexpr std::size_t GetAPITypeSizeInBytes(const std::uint32_t glTypeID)
    {
        switch (glTypeID)
        {
            case GL_FLOAT:
            {
                return sizeof(float);
            };

            case GL_INT:
            case GL_UNSIGNED_INT:
            {
                return sizeof(std::uint32_t);
            };

            default:
                assert(false && "Unsupported type");
                return -1;
        };
    };


    /// <summary>
    /// Generate a texture
    /// </summary>
    /// <param name="texturePath"> A path to the texture </param>
    /// <param name="keepBound"> Should the texture be bound after exiting this function</param>
    /// <returns></returns>
    static std::uint32_t GenerateTexture(const std::string_view& texturePath, bool keepBound = false)
    {
        // The the texture extends beyond it's boundaries, just repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Use linear interpolation when magnifying and minifying 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


        std::uint32_t textureID = 0;
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);


        int width = 0;
        int height = 0;
        int channels = 0;

        // Load the texture
        stbi_set_flip_vertically_on_load(true);
        std::uint8_t* pixels = stbi_load(texturePath.data(), &width, &height, &channels, 0);

        if (pixels == nullptr)
        {
            const char* error = stbi_failure_reason();

            std::cerr << "Texture load error: \"" << error << "\"\n";
            __debugbreak();
            
            return 0; 
        };


        // Determine correct pixel format
        std::uint32_t format = 0;

        if (channels == 4)
            format = GL_RGBA;
        else if (channels == 3)
            format = GL_RGB;

        // _Actually_ create the texture 
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);


        stbi_image_free(pixels);
        pixels = nullptr;

        if (keepBound == false)
            glBindTexture(GL_TEXTURE_2D, 0);


        return textureID;
    };


    /// <summary>
    /// Convert a AccessType to a GL access enumeration.
    /// </summary>
    /// <param name="accessType"> Access type </param>
    /// <returns></returns>
    static constexpr std::uint32_t AccessTypeToAPIEnum(const AccessType accessType)
    {
        switch (accessType)
        {
            case AccessType::ReadOnly:
                return GL_READ_ONLY;

            case AccessType::WriteOnly:
                return GL_WRITE_ONLY;

            case AccessType::ReadWrite:
                return GL_READ_WRITE;

            default:
            {
                assert(false && "Invalid AccessType value");
                return -1;
            };
        };
    };

};


namespace GLUtils = GLUtilities;
namespace GL = GLUtils;