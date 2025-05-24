#pragma once
#include <unordered_map>
#include "Texture.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/math/Vector.h"

namespace engine::graphics
{
struct Submesh
{
    u32 IndexCount = 0;
    u32 StartIndexLocation = 0;
    u32 BaseVertexLocation = 0;
    i32 materialIndex = 0;
    Submesh(u32 indexCount, u32 startIndex, u32 baseVertexLoc, i32 matIndex) : IndexCount(indexCount), StartIndexLocation(startIndex),
        BaseVertexLocation(baseVertexLoc), materialIndex(matIndex) {}
    Submesh() = default;

    void draw(ID3D12GraphicsCommandList* cmList) const;
};

struct Vertex
{
    math::Vector3 position;
    math::Vector3 normal;
    math::Vector4 tangent;
    math::Vector2 uv;
};

template<typename VertexT>
struct Geometry
{
    std::vector<VertexT> vertices;
    std::vector<u32> indices;
    u32 lastIndexLocation = 0;
    u32 lastVertexOffset = 0;
    Submesh getSubmesh(i32 index = -1)
    {
        Submesh result;
        result.IndexCount = (u32)indices.size() - lastIndexLocation;
        result.StartIndexLocation = lastIndexLocation;
        result.BaseVertexLocation = lastVertexOffset;
        result.materialIndex = index;
        lastIndexLocation = (u32)indices.size();
        lastVertexOffset = (u32)vertices.size();
        return result;
    }
};

struct Mesh
{
public:
    Mesh() = default;

    template<typename VertexType>
    Mesh(GfxContext& context, const Geometry<VertexType>& geometry)
    {
        init(context, geometry.vertices.data(), (u32)geometry.vertices.size(),
            geometry.indices.data(), (u32)geometry.indices.size());
    }

    template<typename VertexType, typename IndexType>
    Mesh(
        GfxContext& context,
        const VertexType* vertexData, UINT vertexCount,
        const IndexType* indexData, UINT indexCount)
    {
        init(context, vertexData, vertexCount, indexData, indexCount);
    }

    template<typename VertexType>
    void init(GfxContext& context, const Geometry<VertexType>& geometry)
    {
        init(context, geometry.vertices.data(), (u32)geometry.vertices.size(),
            geometry.indices.data(), (u32)geometry.indices.size());
    }

    template<typename VertexType, typename IndexType>
    void init(
        GfxContext& context,
        const VertexType* vertexData, u32 vertexCount,
        const IndexType* indexData, u32 indexCount)
    {
        m_vertexByteStride = sizeof(VertexType);
        m_vertexBufferByteSize = m_vertexByteStride * vertexCount;

        BufferDescription desc(vertexData, vertexCount, "VertexBuffer", ResourceState::VERTEX_CONSTANT_BUFFER, BufferType::VERTEX);
        m_vertexBuffer.init(context, desc);

        if (indexData != nullptr)
        {
            BufferDescription indexDesc(indexData, indexCount, "IndexBuffer", ResourceState::INDEX_BUFFER, BufferType::INDEX);
            m_indexBuffer.init(context, indexDesc);

            m_indexBufferByteSize = sizeof(IndexType);
            m_indexCount = indexCount;
        }
    }

    void reset()
    {
        m_vertexBuffer.reset();
        m_indexBuffer.reset();
    }

public:

    std::string Name;
    Buffer m_vertexBuffer;
    Buffer m_indexBuffer;
    // Data about the buffers.
    u32 m_vertexByteStride = 0;
    u32 m_vertexBufferByteSize = 0;
    u32 m_indexBufferByteSize = 0;
    u32 m_indexCount = 0;
};
};
