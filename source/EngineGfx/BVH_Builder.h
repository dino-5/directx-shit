#pragma once

#include "EngineCommon/math/Vector.h"
#include "EngineGfx/Model.h"
#include "Mesh.h"
#include "Model.h"
#include "dx12/Device.h"

using namespace engine::math;

namespace engine::graphics
{
const float AABB_MIN = -1e20;
const float AABB_MAX =  1e20;
struct AABB
{
    Vector3 min = AABB_MAX;
    Vector3 max = AABB_MIN;
    void grow(Vector3 p) { min = minVectorCoords(min, p);
                           max = maxVectorCoords(max, p); }
    void grow(AABB p) { min = minVectorCoords(min, p.min);
                        max = maxVectorCoords(max, p.max);  } 
    inline Vector3 diagonal() const { return max - min;}
    inline Vector3 middle() const { return (min + max) * 0.5; }
    inline float area() const
    {
        Vector3 d = diagonal();
        return d[0] * d[1] + d[0] * d[2] + d[1] * d[2];
    }
};

inline AABB AABBFromTriangle(const u32* indices, 
         const std::vector<Vertex>& vertices, u32 offset=0)
{
    AABB aabb;
    aabb.grow(vertices[indices[0]+offset].position);
    aabb.grow(vertices[indices[1]+offset].position);
    aabb.grow(vertices[indices[2]+offset].position);
    return aabb;
}

struct BVHNode
{
    AABB aabb;
    u32 leftChild = 0, triangleCount = 0;
    bool isLeaf() const { return triangleCount > 0; }
};

template<typename T>
using functionT = std::function<Vector3(const T&)>;
template<typename T>
using isLeafT = std::function<bool(const T&)>;

class BVHBuilder
{
public:
    BVHBuilder() = default;
    BVHBuilder(const graphics::Model& model);
    void build(const graphics::Model& model);

    const Mesh& getMesh()const { return m_mesh; }
    const Submesh getSubmesh() const { return m_submesh; }
    const BVHNode* getRootNode() const { return &m_nodes[0]; }
    u32 getNodeCount() const { return lastElement; }

    template<typename BVHNodeType>
    static Model generateDrawData(GfxContext& context,
                                  const BVHNodeType* rootNode,
                                  u32 nodeCount,
                                  functionT<const BVHNodeType&> diagonalF,
                                  functionT<const BVHNodeType&> aabbMinF,
                                  isLeafT<const BVHNodeType&> leafF);

private:
    void subdivide();
    float findBestSplitPosition(u32 nodeIndex, u32& splitAxis, float& splitPosition);

    const Model* m_model;
    Mesh m_mesh;
    Submesh m_submesh;
    std::vector<AABB> aabbs;
    std::vector<u32> triIndices;
    u32 lastElement=0;
    Vector3 m_minDim;

    std::vector<BVHNode> m_nodes;
};

template<typename BVHNodeType>
Model BVHBuilder::generateDrawData(GfxContext& context,
                                  const BVHNodeType* rootNode,
                                  u32 nodeCount,
                                  functionT<const BVHNodeType&> diagonalF,
                                  functionT<const BVHNodeType&> aabbMinF,
                                  isLeafT<const BVHNodeType&> leafF)
{
    Geometry<Vector3> geometry;
    auto& vertices = geometry.vertices;
    auto& indices = geometry.indices;

    vertices.resize(nodeCount * 8);
    indices.resize(nodeCount * 24);

    for(u32 i = 0; i < nodeCount; ++i)
    {
        const BVHNodeType& node = rootNode[i];
        if(!leafF(node))
            continue;

        Vector3 d = diagonalF(node);
        Vector3 aabbMin = aabbMinF(node);
        vertices[i * 8 + 0] = aabbMin;
        vertices[i * 8 + 1] = aabbMin + Vector3({ d[0], 0,    0    });
        vertices[i * 8 + 2] = aabbMin + Vector3({ d[0], 0,    d[2] });
        vertices[i * 8 + 3] = aabbMin + Vector3({ 0,    0,    d[2] });
        vertices[i * 8 + 4] = aabbMin + Vector3({ 0,    d[1], 0    });
        vertices[i * 8 + 5] = aabbMin + Vector3({ d[0], d[1], 0    });
        vertices[i * 8 + 6] = aabbMin + d;
        vertices[i * 8 + 7] = aabbMin + Vector3({ 0,    d[1], d[2] });

        // bottom
        for (int j = 0; j <= 3; j++)
        {
            indices[i * 24 + 2 * j + 0] = i * 8 + j;
            indices[i * 24 + 2 * j + 1] = i * 8 + (j+1) % 4;
        }
        // top
        for (int j = 0; j <= 3; j++)
        {
            indices[i * 24 + 2 * j + 8] = i * 8 + 4 + j;
            indices[i * 24 + 2 * j + 9] = i * 8 + 4 + (j+1) % 4;
        }
        // edges
        for (int j = 0; j <= 3; j++)
        {
            indices[i * 24 + 2 * j + 16] = i * 8 + j;
            indices[i * 24 + 2 * j + 17] = i * 8 + j + 4;
        }
    }

    Submesh submesh = geometry.getSubmesh();
    Mesh mesh(context, geometry);
    Model model;
    model.m_mesh = mesh;
    model.m_submeshes.push_back(submesh);
    model.m_isInitialized = true;
    return model;
}
};
