#pragma once

#include "EngineCommon/math/Vector.h"
#include "EngineGfx/Model.h"
#include "Mesh.h"
#include "Model.h"
#include "dx12/Device.h"

using namespace engine::math;

namespace engine::graphics
{

struct Triangle
{
    u32 vertex0, vertex1, vertex2; // indices to real vertices inside Model
    Vector3 centroid;
    Triangle(u32 first, u32 second, u32 third, const std::vector<Vertex>& vertices, u32 offset=0):
        vertex0(first + offset), vertex1(second + offset), vertex2(third + offset)
    {
        auto result = (vertices[vertex0].position + vertices[vertex1].position + vertices[vertex2].position);
        centroid = result * 0.333;
    }
};

const float AABB_MIN = -1e20;
const float AABB_MAX =  1e20;
struct AABB
{
    Vector3 aabbMin = AABB_MAX;
    Vector3 aabbMax = AABB_MIN;
    void grow(Vector3 p) { aabbMin = minVectorCoords(aabbMin, p); aabbMax = maxVectorCoords(aabbMax, p); }
    void grow(AABB p) { grow(p.aabbMin); grow(p.aabbMax); } 
    Vector3 diagonal() const { return aabbMax - aabbMin;}
    float area() const
    {
        Vector3 d = diagonal()*0.01;
        return (d[0]*d[1] + d[0]*d[2] +d[1]*d[2]);
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

    void generateDrawData(GfxContext& context);
    const Mesh& getMesh()const { return m_mesh; }
    const Submesh getSubmesh() const { return m_submesh; }

private:
    void subdivide(u32 index);
    void updateNodeBounds(u32 index);
    float findBestSplitPosition(u32 nodeIndex, u32& splitAxis, float& splitPosition);

    const Model* m_model;
    Mesh m_mesh;
    Geometry<Vector3> m_geometry;
    Submesh m_submesh;
    std::vector<Triangle> triangles;
    std::vector<u32> triIndices;
    u32 lastElement=0;

    std::vector<BVHNode> m_nodes;
};
};
