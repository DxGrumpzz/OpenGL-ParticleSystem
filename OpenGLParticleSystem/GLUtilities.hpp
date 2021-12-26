#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <cassert>
#include <stb_image.h>


namespace GLUtilities
{

    constexpr std::size_t GetAPITypeSizeInBytes(const std::uint32_t glTypeID)
    {
        switch (glTypeID)
        {
            case GL_FLOAT:
            {
                return sizeof(float);
            };

            default:
                assert(false && "Unsupported type");
                return -1;
        };
    };


    std::uint32_t GenerateTexture(const std::string_view& texturePath, bool keepBound = false)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

        stbi_set_flip_vertically_on_load(true);
        std::uint8_t* pixels = stbi_load(texturePath.data(), &width, &height, &channels, 0);

        if (pixels == nullptr)
        {
            const char* error = stbi_failure_reason();

            __debugbreak();
        };


        std::uint32_t format = 0;

        if (channels == 4)
            format = GL_RGBA;
        else if (channels == 3)
            format = GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);


        stbi_image_free(pixels);
        pixels = nullptr;

        if (keepBound == false)
            glBindTexture(GL_TEXTURE_2D, 0);


        return textureID;
    };

};


namespace GLUtils = GLUtilities;
namespace GL = GLUtils;