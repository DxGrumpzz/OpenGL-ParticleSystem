#pragma once

#include <cstdint>
#include <vector>
#include <glad/glad.h>

/// <summary>
/// A struct that describes a vertex' layout
/// </summary>
struct BufferLayoutElement
{
    std::uint32_t StartingIndex = 0;

    /// <summary>
    /// How many elements are present in this vertex 
    /// </summary>
    std::uint32_t ElementCount = 0;

    /// <summary>
    /// The ID of a given type, defined by the API
    /// </summary>
    std::uint32_t ApiTypeID = 0;

    bool Normalize = false;
    
    /// <summary>
    /// How to divide the layout element per instance
    /// </summary>
    std::uint32_t Divisor = 0;
};


/// <summary>
/// 
/// </summary>
class BufferLayout
{
private:

    /// <summary>
    /// List of vertex elements
    /// </summary>
    std::vector<BufferLayoutElement> _elements;

    /// <summary>
    /// The stride per vertex
    /// </summary>
    std::uint32_t _stride = 0;


public:

    /// <summary>
    /// Addds a new BufferLayoutElement
    /// </summary>
    /// <typeparam name="TElement"></typeparam>
    /// <param name="startingIndex"></param>
    /// <param name="elementCount"></param>
    /// <param name="divisor"></param>
    /// <param name="normalize"></param>
    template<typename TElement>
    void AddElement(const std::uint32_t startingIndex, const std::uint32_t elementCount, const std::uint32_t divisor = 0, const bool normalize = false)
    {
        static_assert(false, "Unsupported type");
    };


    template<>
    void AddElement<float>(const std::uint32_t startingIndex, const std::uint32_t elementCount, const std::uint32_t divisor, const bool normalize)
    {
        _elements.emplace_back(startingIndex, elementCount, GL_FLOAT, normalize, divisor);

        // We calculate the stride per added vertex
        _stride += sizeof(float) * elementCount;
    };

    template<>
    void AddElement<std::uint32_t>(const std::uint32_t startingIndex, const std::uint32_t elementCount, const std::uint32_t divisor, const bool normalize)
    {
        _elements.emplace_back(startingIndex, elementCount, GL_UNSIGNED_INT, normalize, divisor);

        // We calculate the stride per added vertex
        _stride += sizeof(std::uint32_t) * elementCount;
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