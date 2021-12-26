#pragma once

#include <cstdint>
#include <glad/glad.h>

#include "GLUtilities.hpp"
#include "VertexBuffer.hpp"
#include "BufferLayout.hpp"


class VertexArray
{
private:
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


    void AddBuffer(const VertexBuffer& vertexBuffer, const BufferLayout& bufferLayout)
    {
        Bind();

        vertexBuffer.Bind();

        const auto& bufferLayoutElements = bufferLayout.GetElements();

        std::size_t offset = 0;

        for (std::size_t i = 0; i < bufferLayoutElements.size(); i++)
        {
            const auto& bufferLayoutElement = bufferLayoutElements[i];

            glVertexAttribPointer(bufferLayoutElement.StartingIndex,
                                  bufferLayoutElement.ElementCount,
                                  bufferLayoutElement.ApiTypeID,
                                  bufferLayoutElement.Normalize,
                                  bufferLayout.GetStride(),
                                  reinterpret_cast<const void*>(offset));

            glEnableVertexAttribArray(bufferLayoutElement.StartingIndex);

            glVertexAttribDivisor(bufferLayoutElement.StartingIndex, bufferLayoutElement.Divisor);

            offset += GL::GetAPITypeSizeInBytes(bufferLayoutElement.ApiTypeID) * bufferLayoutElement.ElementCount;
        };

    };

public:

    std::uint32_t GetID()
    {
        return _id;
    };

};
