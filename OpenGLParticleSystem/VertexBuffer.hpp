#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <glad/glad.h>

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


public:

    VertexBuffer(const void* const bufferData, std::size_t bufferSizeInBytes)
    {
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, bufferSizeInBytes, bufferData, GL_STATIC_DRAW);
    };


    // There shouldn't be any reason for this constrcutor to exist in this context, right?
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
    std::unique_ptr<T, std::function<void(T*)>> GetBuffer(const GL::AccessType accessType = GL::AccessType::ReadWrite) const
    {
        // The deleter function used by the unique_ptr to release the acquired memory.
        // Can't get the unique_ptr deleter to behave with anything other than a lambda and an 'std::function'
        const static auto deleter = [this](T* buffer)
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
        auto particleTransformBuffer = std::unique_ptr<T, std::function<void(T*)>>(static_cast<T*>(address), deleter);

        return particleTransformBuffer;
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


private:


};
