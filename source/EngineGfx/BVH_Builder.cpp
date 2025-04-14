#include "BVH_Builder.h"
#include "Model.h"

namespace engine::graphics
{

BVHBuilder::BVHBuilder(const Model& model)
{
    build(model);
}

void BVHBuilder::build(const Model& model)
{
    m_model = &model;

    auto& submeshes = model.m_submeshes;
    auto& vertices = model.m_geometry.vertices;
    auto& indices = model.m_geometry.indices;

    triangles.reserve(indices.size()/3);
    m_nodes.resize(triangles.capacity() * 2 - 1);

    for(auto& submesh : submeshes)
    {
        for(int i = 0; i < submesh.IndexCount; i+=3)
        {
            u32 index = submesh.StartIndexLocation + i;
            auto ind = &indices[index];
            triangles.push_back(Triangle(ind[0], ind[1], ind[2], vertices, submesh.BaseVertexLocation));
        }
    }
    util::printInfo("{} avg distribution", Triangle::avarageDistribution);

    triIndices.resize(triangles.size());
    util::printInfo("{} triangles and {} triIndices", triangles.size(), triIndices.size());
    u32 i = 0;
    for(auto& index : triIndices)
        index = i++;

    m_nodes[0].leftChild = 0;
    m_nodes[0].triangleCount = triangles.size();
    updateNodeBounds(0);
    subdivide(0);
    util::printInfo("{} elements of BVH", lastElement);
}

void BVHBuilder::subdivide(u32 index)
{
    BVHNode& node = m_nodes[index];
    if (node.triangleCount <= 3)
        return;

    Vector3 diagonal = node.aabbMax - node.aabbMin;
    u32 splitAxisIndex = 0;
    if (abs(diagonal[splitAxisIndex]) < abs(diagonal[1]))
        splitAxisIndex = 1;
    if (abs(diagonal[splitAxisIndex]) < abs(diagonal[2])) 
        splitAxisIndex = 2;

    float splitLine = node.aabbMin[splitAxisIndex] + diagonal[splitAxisIndex] * 0.5f;
    util::printInfo("{} {}", splitLine, splitAxisIndex);
    int i = node.leftChild, j = i + node.triangleCount - 1;
    while (i <= j)
    {
        if(triangles[triIndices[i]].centroid[splitAxisIndex] < splitLine)
            i++;
        else
            std::swap(triIndices[i], triIndices[j--]);
    }

    u32 leftCount = i - node.leftChild;
    if (leftCount == 0 || leftCount == node.triangleCount)
        return;

    u32 leftChildIndex = ++lastElement;
    u32 rightChildIndex = ++lastElement;
    BVHNode& leftNode = m_nodes[leftChildIndex];
    BVHNode& rightNode = m_nodes[rightChildIndex];
    leftNode.triangleCount = leftCount;
    leftNode.leftChild = node.leftChild;
    rightNode.triangleCount = node.triangleCount - leftCount;
    rightNode.leftChild = i;
    node.triangleCount = 0;
    node.leftChild = leftChildIndex;

    updateNodeBounds(leftChildIndex);
    updateNodeBounds(rightChildIndex);

    subdivide(leftChildIndex);
    subdivide(rightChildIndex);
}

void BVHBuilder::updateNodeBounds(u32 index)
{
    BVHNode& node = m_nodes[index];
    node.aabbMin = math::Vector3(1e30);
    node.aabbMax = math::Vector3(-1e30);
    auto& vertices = m_model->m_geometry.vertices;
    for (int i = node.leftChild; i < node.leftChild + node.triangleCount; ++i)
    {
        auto& triangle = triangles[triIndices[i]];
        node.aabbMin = math::minVectorCoords(node.aabbMin, vertices[triangle.firstI].position);
        node.aabbMin = math::minVectorCoords(node.aabbMin, vertices[triangle.secondI].position);
        node.aabbMin = math::minVectorCoords(node.aabbMin, vertices[triangle.thirdI].position);
        node.aabbMax = math::maxVectorCoords(node.aabbMax, vertices[triangle.firstI].position);
        node.aabbMax = math::maxVectorCoords(node.aabbMax, vertices[triangle.secondI].position);
        node.aabbMax = math::maxVectorCoords(node.aabbMax, vertices[triangle.thirdI].position);
    }
    Vector3& min = node.aabbMin;
    Vector3& max = node.aabbMax;
    util::printInfo("index {}, aabb min {} {} {} aabb max {} {} {}", index, min[0], min[1], min[2], max[0], max[1], max[2]);

}

void BVHBuilder::generateDrawData(GfxContext& context)
{
    auto& vertices = m_geometry.vertices;
    auto& indices = m_geometry.indices;

    vertices.resize(lastElement * 8);
    indices.resize(lastElement * 24);

    for(int i = 0; i < lastElement; ++i)
    {
        BVHNode& node = m_nodes[i];
        Vector3 d = node.aabbMax - node.aabbMin;
        vertices[i * 8 + 0] = node.aabbMin;
        vertices[i * 8 + 1] = node.aabbMin + Vector3({ d[0], 0,    0    });
        vertices[i * 8 + 2] = node.aabbMin + Vector3({ d[0], 0,    d[2] });
        vertices[i * 8 + 3] = node.aabbMin + Vector3({ 0,    0,    d[2] });
        vertices[i * 8 + 4] = node.aabbMin + Vector3({ 0,    d[1], 0    });
        vertices[i * 8 + 5] = node.aabbMin + Vector3({ d[0], d[1], 0    });
        vertices[i * 8 + 6] = node.aabbMax;
        vertices[i * 8 + 7] = node.aabbMin + Vector3({ 0,    d[1], d[2] });

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
    m_submesh = m_geometry.getSubmesh();
    m_mesh.init(context, m_geometry);
}

};
