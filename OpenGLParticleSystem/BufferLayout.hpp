#pragma once

#include <cstdint>
#include <vector>
#include <glad/glad.h>


struct BufferLayoutElement
{
    std::uint32_t StartingIndex = 0;
    std::uint32_t ElementCount = 0;
    std::uint32_t ApiTypeID = 0;
    bool Normalize = false;
    std::uint32_t Divisor = 0;
};


class BufferLayout
{
private:

    std::vector<BufferLayoutElement> _elements;

    /// <summary>
    /// The stride per vertex
    /// </summary>
    std::uint32_t _stride = 0;


public:

    template<typename TElement>
    void AddElement(const std::uint32_t startingIndex, const std::uint32_t elementCount, const std::uint32_t divisor = 0, const bool normalize = false)
    {
        static_assert(false, "Unsupported type");
    };


    template<>
    void AddElement<float>(const std::uint32_t startingIndex, const std::uint32_t elementCount, const std::uint32_t divisor, const bool normalize)
    {
        _elements.emplace_back(startingIndex, elementCount, GL_FLOAT, normalize, divisor);

        _stride += sizeof(float) * elementCount;
    };


public:

    const std::vector<BufferLayoutElement>& GetElements() const
    {
        return _elements;
    };

    std::uint32_t GetStride() const
    {
        return _stride;
    };

};