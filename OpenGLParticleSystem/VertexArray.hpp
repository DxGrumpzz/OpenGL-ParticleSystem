#pragma once

#include <cstdint>
#include <glad/glad.h>

#include "GLUtilities.hpp"
#include "VertexBuffer.hpp"
#include "BufferLayout.hpp"


/// <summary>
/// A class that 
/// </summary>
class VertexArray
{
private:

    /// <summary>
    /// An identifier used by the API
    /// </summary>
    std::uint32_t _id = 0;

public:

    VertexArray()
    {
        glGenVertexArrays(1, &_id);
        glBindVertexArray(_id);
    };

    ~VertexArray()
    {
        glDeleteVertexArrays(1, &_id);
        glBindVertexArray(0);
    };

public:

    void Bind() const
    {
        glBindVertexArray(_id);
    };


    /// <summary>
    /// Adds a VBO to this VAO
    /// </summary>
    /// <param name="vertexBuffer"></param>
    /// <param name="bufferLayout"></param>
    void AddBuffer(const VertexBuffer& vertexBuffer, const BufferLayout& bufferLayout)
    {
        // Make sure that this VertexArray is bound
        Bind();

        // Make sure that the correct VBO is bound
        vertexBuffer.Bind();


        const auto& bufferLayoutElements = bufferLayout.GetElements();

        std::size_t offset = 0;

        for (const auto& bufferLayoutElement : bufferLayoutElements)
        {
            if ((bufferLayoutElement.ApiTypeID == GL_INT) ||
                (bufferLayoutElement.ApiTypeID == GL_UNSIGNED_INT))
            {
                glVertexAttribIPointer(bufferLayoutElement.StartingIndex,
                                       bufferLayoutElement.ElementCount,
                                       bufferLayoutElement.ApiTypeID,
                                       bufferLayout.GetStride(),
                                       reinterpret_cast<const void*>(offset));
            }
            else
            {
                glVertexAttribPointer(bufferLayoutElement.StartingIndex,
                                      bufferLayoutElement.ElementCount,
                                      bufferLayoutElement.ApiTypeID,
                                      bufferLayoutElement.Normalize,
                                      bufferLayout.GetStride(),
                                      reinterpret_cast<const void*>(offset));
            };


            glEnableVertexAttribArray(bufferLayoutElement.StartingIndex);

            glVertexAttribDivisor(bufferLayoutElement.StartingIndex, bufferLayoutElement.Divisor);

            // Calculate the offset of the next vertex 
            offset += GL::GetAPITypeSizeInBytes(bufferLayoutElement.ApiTypeID) * bufferLayoutElement.ElementCount;
        };

    };

public:

    std::uint32_t GetID()
    {
        return _id;
    };

};