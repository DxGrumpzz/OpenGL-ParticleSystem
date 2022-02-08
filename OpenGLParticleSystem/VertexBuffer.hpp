#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <functional>

#include "GLUtilities.hpp"


/// <summary>
/// A class that holds a buffer of data that will be later associated with a vertex
/// </summary>
class VertexBuffer
{

private:

    /// <summary>
    /// An identifier used by the API
    /// </summary>
    std::uint32_t _id = 0;

    /// <summary>
    /// The size of this buffer in bytes
    /// </summary>
    std::size_t _bufferSizeInBytes = 0;

public:

    VertexBuffer(const void* const bufferData, const std::size_t bufferSizeInBytes, const std::uint32_t usageType = GL_STATIC_DRAW) :
        _bufferSizeInBytes(bufferSizeInBytes)
    {
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, bufferSizeInBytes, bufferData, usageType);
    };


    VertexBuffer(VertexBuffer&& vertexBuffer) noexcept : 
        // Swap between this and the other buffer such that when we then call the destructor on the "old" buffer
        // we call on a "null" object, which is fine apparently
        _id(vertexBuffer._id)
    {
        vertexBuffer._id = 0;
    };

    VertexBuffer(const VertexBuffer&) = delete;


    ~VertexBuffer()
    {
        glDeleteBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    };

public:


    /// <summary>
    /// Returns a pointer to VBO buffer which can be written to and released automatically
    /// </summary>
    /// <typeparam name="T"> The type of data we're dealing with </typeparam>
    /// <returns></returns>
    template<typename T>
    std::unique_ptr<T, std::function<void(T*)>> MapBuffer(const GL::AccessType accessType = GL::AccessType::ReadWrite) const
    {
        // The deleter function used by the unique_ptr to release the acquired memory.
        // Can't get the unique_ptr deleter to behave with anything other than a lambda and an 'std::function'
        static auto deleter = [this](T* buffer)
        {
            // Make sure that the correct buffer is bound before we unmap
            Bind();
            glUnmapBuffer(GL_ARRAY_BUFFER);
        };

        // Ensure that this VBO is bound before we acquire the buffer
        Bind();

        const std::uint32_t apiAccessType = GL::AccessTypeToAPIEnum(accessType);

        // Acquire a pointer to buffer
        void* address = glMapBuffer(GL_ARRAY_BUFFER, apiAccessType);

        // Convert the pointer into a unique_ptr
        auto buffer = std::unique_ptr<T, std::function<void(T*)>>(std::bit_cast<T*>(address), deleter);

        return buffer;
    };


    template<typename T>
    void Fill(const T& value)
    {
        constexpr auto valueSizeInBytes = sizeof(value);

        auto buffer = MapBuffer<T>(GL::AccessType::WriteOnly);

        for(std::size_t i = 0; i < _bufferSizeInBytes / valueSizeInBytes; i++)
        {
            T& bufferValue = buffer.get()[i];

            bufferValue = value;
        };

    };


    void Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, _id);
    };


public:

    std::uint32_t GetID()
    {
        return _id;
    };


public:

    VertexBuffer& operator = (const VertexBuffer&) = delete;

};