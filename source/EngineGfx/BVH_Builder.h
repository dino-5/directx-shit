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
    Vector3 aabbMin = AABB_MAX;
    Vector3 aabbMax = AABB_MIN;
    void grow(Vector3 p) { aabbMin = minVectorCoords(aabbMin, p); aabbMax = maxVectorCoords(aabbMax, p); }
    void grow(AABB p) { aabbMin = minVectorCoords(aabbMin, p.aabbMin); aabbMax = maxVectorCoords(aabbMax, p.aabbMax);  } 
    inline Vector3 diagonal() const { return aabbMax - aabbMin;}
    inline Vector3 middle() const { return (aabbMin + aabbMax) * 0.5; }
    inline float area() const
    {
        Vector3 d = diagonal();
        return d[0] * d[1] + d[0] * d[2] + d[1] * d[2];
    }
};

struct Triangle
{
    AABB aabb;
    Triangle(u32 first, u32 second, u32 third, const std::vector<Vertex>& vertices, u32 offset=0)
    {
        aabb.grow(vertices[first+offset].position);
        aabb.grow(vertices[second+offset].position);
        aabb.grow(vertices[third+offset].position);
    }
};

struct BVHNode
{
    AABB aabb;
    u32 leftChild = 0, triangleCount = 0;
    bool isLeaf() const { return triangleCount > 0; }
};

class BVHBuilder
{
public:
    BVHBuilder() = default;
    BVHBuilder(const graphics::Model& model);
    void build(const graphics::Model& model);

    const Mesh& getMesh()const { return m_mesh; }
    const Submesh getSubmesh() const { return m_submesh; }
    template<typename BVHNodeType>
    static Model generateDrawData(GfxContext& context,
                                  const BVHNodeType* rootNode,
                                  u32 nodeCount,
                                  std::function<Vector3(const BVHNodeType&)> diagonalF,
                                  std::function<Vector3(const BVHNodeType&)> aabbMinF);

    const BVHNode* getRootNode() const { return &m_nodes[0]; }
    u32 getNodeCount() const { return lastElement; }

private:
    void subdivide(u32 index);
    void updateNodeBounds(u32 index);
    float findBestSplitPosition(u32 nodeIndex, u32& splitAxis, float& splitPosition);

    const Model* m_model;
    Mesh m_mesh;
    Submesh m_submesh;
    std::vector<Triangle> triangles;
    std::vector<u32> triIndices;
    u32 lastElement=0;
    Vector3 m_minDim;

    std::vector<BVHNode> m_nodes;
};

template<typename BVHNodeType>
Model BVHBuilder::generateDrawData(GfxContext& context,
                                  const BVHNodeType* rootNode,
                                  u32 nodeCount,
                                  std::function<Vector3(const BVHNodeType&)> diagonalF,
                                  std::function<Vector3(const BVHNodeType&)> aabbMinF)
{
    Geometry<Vector3> geometry;
    auto& vertices = geometry.vertices;
    auto& indices = geometry.indices;

    vertices.resize(nodeCount * 8);
    indices.resize(nodeCount * 24);

    for(u32 i = 0; i < nodeCount; ++i)
    {
        const BVHNodeType& node = rootNode[i];
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
    return model;
}
};
