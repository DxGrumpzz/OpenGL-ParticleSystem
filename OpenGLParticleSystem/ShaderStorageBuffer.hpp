#pragma once

#include <cstdint>
#include <cstddef>
#include <glad/glad.h>
#include <functional>

#include "VertexBuffer.hpp"


/// <summary>
/// A warpper class for an SSBO buffer
/// </summary>
class ShaderStorageBuffer
{

private:

    /// <summary>
    /// An identifier used by the API
    /// </summary>
    std::uint32_t _bufferId = 0;


public:


    ShaderStorageBuffer(const void* const bufferData, const std::size_t bufferSizeInBytes, const std::uint32_t bindIndex = 0, const std::uint32_t usageType = GL_STATIC_DRAW)
    {
        glGenBuffers(1, &_bufferId);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);

        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeInBytes, bufferData, usageType);

        // Typically with SSBO's we bind them to binding point defined in the shader
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

    // Don't allow copying of OpenGL buffers
    ShaderStorageBuffer(const VertexBuffer&) = delete;


    ~ShaderStorageBuffer()
    {
        Destroy();
    };


public:

    /// <summary>
    /// Bind this buffer as a GL_SHADER_STORAGE_BUFFER
    /// </summary>
    void Bind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    };

    /// <summary>
    /// Retrieve the data inside the SSBO to bufferData
    /// </summary>
    /// <typeparam name="T"> The type of data contained in the buffer </typeparam>
    /// <param name="bufferData"> A pointer to buffer data which will be filled </param>
    /// <param name="numberOfElementsInBuffer"> The number of _elements_ to fill </param>
    /// <param name="elemetOffset"> The _element_ offset at which to start reading from </param>
    template<typename T>
    void GetBuffer(T* bufferData, const std::size_t numberOfElementsInBuffer, std::size_t elemetOffset = 0) const
    {
        Bind();

        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * elemetOffset, sizeof(T) * numberOfElementsInBuffer, bufferData);
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

    // Don't allow copying of OpenGL buffers
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
