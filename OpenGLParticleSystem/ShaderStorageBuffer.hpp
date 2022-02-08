#pragma once

#include <cstdint>
#include <cstddef>
#include <glad/glad.h>
#include <functional>

#include "VertexBuffer.hpp"


class ShaderStorageBuffer
{
private:

    std::uint32_t _bufferId = 0;


public:


    ShaderStorageBuffer(const void* const bufferData, const std::size_t bufferSizeInBytes, const std::uint32_t bindIndex = 0, const std::uint32_t usageType = GL_STATIC_DRAW)
    {
        glGenBuffers(1, &_bufferId);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);

        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeInBytes, bufferData, usageType);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindIndex, _bufferId);
    };


    ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept :
        // Swap between this and the other buffer such that when we then call the destructor 
        // we call on a "null" object, which is fine apparently 
        _bufferId(other._bufferId)
    {
        // The "other" buffer is set to zero, because it will then be destroyed
        other._bufferId = 0;
    };


    ShaderStorageBuffer(const VertexBuffer&) = delete;


    ~ShaderStorageBuffer()
    {
        Destroy();
    };


public:

    void Bind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    };

    template<typename T>
    void GetBuffer(T* bufferData, const std::size_t numberOfElementsInBuffer) const
    {
        Bind();

        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * numberOfElementsInBuffer, bufferData);
    };


    void Destroy() const
    {
        glDeleteBuffers(1, &_bufferId);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    };


public:

    std::uint32_t GetBufferID() const
    {
        return _bufferId;
    };


public:

    ShaderStorageBuffer& operator = (const ShaderStorageBuffer&) = delete;


    ShaderStorageBuffer& operator = (ShaderStorageBuffer&& other) noexcept
    {
        if(this != &other)
        {
            std::swap(_bufferId, other._bufferId);

            other.Destroy();
            other._bufferId = 0;
        };

        return *this;
    };

};
