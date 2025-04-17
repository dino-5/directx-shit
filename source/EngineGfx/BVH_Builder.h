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
    static inline double avarageDistribution = 0;
    static inline u32 count = 0;
    u32 firstI, secondI, thirdI; // indices to real vertices inside Model
    Vector3 centroid;
    Triangle(u32 first, u32 second, u32 third, const std::vector<Vertex>& vertices, u32 offset=0):
        firstI(first + offset), secondI(second + offset), thirdI(third + offset)
    {
        auto result = (vertices[firstI].position + vertices[secondI].position + vertices[thirdI].position);
        centroid = result * 0.333;

        auto avg = (vertices[firstI].position - centroid).length() + (vertices[secondI].position - centroid).length() +(vertices[thirdI].position - centroid).length();
        avg *= 0.333;
        avarageDistribution = avarageDistribution * count / (count + 1) + avg / (count + 1);
        count++;
    }
};

struct AABB
{
    Vector3 leftBottom;
    Vector3 rightTop;
    void grow(Vector3 p) { leftBottom = minVectorCoords(leftBottom, p); rightTop = maxVectorCoords(rightTop, p); }
    float area() const
    {
        Vector3 diagonal = rightTop - leftBottom;
        return diagonal[0]*diagonal[1] + diagonal[0]*diagonal[2] +diagonal[1]*diagonal[2];
    }

    
};

struct BVHNode
{
    Vector3 aabbMin, aabbMax;
    u32 leftChild = 0, triangleCount = 0;
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
    float evaluteSAH(u32 nodeIndex, u32 axisIndex, float splitPos);

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
