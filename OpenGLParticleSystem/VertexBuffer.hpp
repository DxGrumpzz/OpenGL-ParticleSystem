#pragma once

#include <cstdint>
#include <vector>
#include <glad/glad.h>

class VertexBuffer
{
private:
    std::uint32_t _id = 0;


public:

    VertexBuffer(const std::vector<std::byte>& bufferData)
    {
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, bufferData.size(), bufferData.data(), GL_STATIC_DRAW);
    };

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



    void Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, _id);
    };



public:

    std::uint32_t GetID()
    {
        return _id;
    };

};
