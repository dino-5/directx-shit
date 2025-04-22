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
        for(u32 i = 0; i < submesh.IndexCount; i+=3)
        {
            u32 index = submesh.StartIndexLocation + i;
            auto ind = &indices[index];
            triangles.push_back(Triangle(ind[0], ind[1], ind[2], vertices, submesh.BaseVertexLocation));
        }
    }

    triIndices.resize(triangles.size());
    util::printInfo("{} triangles", triangles.size());
    u32 i = 0;
    for(auto& index : triIndices)
        index = i++;

    m_nodes[0].leftChild = 0;
    m_nodes[0].triangleCount = (u32)triangles.size();
    updateNodeBounds(0);
    subdivide(0);
    util::printInfo("{} elements of BVH", lastElement);
}

float BVHBuilder::evaluteSAH(u32 nodeIndex, u32 axisIndex, float splitPos)
{
    BVHNode& node = m_nodes[nodeIndex];
    auto& vertices = m_model->m_geometry.vertices;

    u32 leftCount = 0, rightCount = 0;
    AABB leftAABB, rightAABB;

    for(u32 i = node.leftChild; i < node.leftChild + node.triangleCount; ++i)
    {
        auto& tri = triangles[triIndices[i]];
        if(tri.centroid[axisIndex] < splitPos)
        {
            leftAABB.grow(vertices[tri.firstI].position);
            leftAABB.grow(vertices[tri.secondI].position);
            leftAABB.grow(vertices[tri.thirdI].position);
            leftCount++;
        }
        else
        {
            rightAABB.grow(vertices[tri.firstI].position);
            rightAABB.grow(vertices[tri.secondI].position);
            rightAABB.grow(vertices[tri.thirdI].position);
            rightCount++;
        }
        
    }
    float cost = leftCount * leftAABB.area() + rightCount * rightAABB.area();
    return cost > 0 ? cost : 1e20f;
}

float BVHBuilder::findBestSplitPosition(u32 nodeIndex, u32& splitAxis, float& splitPosition)
{
    BVHNode& node = m_nodes[nodeIndex];

    float bestSah = 1e20;
    for(int i = 0; i < 3; ++i)
    {
        float boundMin = 1e20;
        float boundMax = -1e20;
        for (u32 j = node.leftChild; j < node.leftChild + node.triangleCount; ++j)
        {
            float centroid = triangles[triIndices[j]].centroid[i];
            boundMin = min(boundMin, centroid);
            boundMax = max(boundMax, centroid);
        }
        if(boundMax == boundMin) continue;
        u32 stepCount = 4;
        float stepSize = (boundMax - boundMin) / (float)stepCount;
        for(u32 j = 0; j < stepCount; ++j)
        {
            float candidateSplitLine = boundMin + j * stepSize;
            float sah = evaluteSAH(nodeIndex, i, candidateSplitLine);
            if (sah < bestSah)
            {
                bestSah = sah;
                splitAxis = i;
                splitPosition = candidateSplitLine;
            }
        }
    }
    return bestSah > 0 ? bestSah : 1e20;
}

void BVHBuilder::subdivide(u32 index)
{
    BVHNode& node = m_nodes[index];
    if (node.triangleCount <= 50)
        return;

    u32 bestSplitAxisIndex = 0;
    float bestSplitLine = 0;
    float bestSah = findBestSplitPosition(index, bestSplitAxisIndex, bestSplitLine);
    
    /*Vector3 diagonal = node.aabbMax - node.aabbMin;*/
    /*u32 splitAxisIndex = 0;*/
    /*if (abs(diagonal[splitAxisIndex]) < abs(diagonal[1]))*/
    /*    splitAxisIndex = 1;*/
    /*if (abs(diagonal[splitAxisIndex]) < abs(diagonal[2])) */
    /*    splitAxisIndex = 2;*/

    float splitLine;
    float splitAxisIndex = bestSplitAxisIndex;
    splitLine = bestSplitLine;

    Vector3 diagonal = node.aabbMax - node.aabbMin;
    float parrentCost = node.triangleCount * (diagonal[0] * diagonal[1] + diagonal[0] * diagonal[2] + diagonal[1] * diagonal[2]);
    if(parrentCost <= bestSah)
        return;;

    u32 i = node.leftChild, j = i + node.triangleCount - 1;
    while (i <= j)
    {
        auto& triangle = triangles[triIndices[i]];
        if(triangle.centroid[splitAxisIndex] < splitLine)
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
    for (u32 i = node.leftChild; i < node.leftChild + node.triangleCount; ++i)
    {
        auto& triangle = triangles[triIndices[i]];
        node.aabbMin = math::minVectorCoords(node.aabbMin, vertices[triangle.firstI].position);
        node.aabbMin = math::minVectorCoords(node.aabbMin, vertices[triangle.secondI].position);
        node.aabbMin = math::minVectorCoords(node.aabbMin, vertices[triangle.thirdI].position);
        node.aabbMax = math::maxVectorCoords(node.aabbMax, vertices[triangle.firstI].position);
        node.aabbMax = math::maxVectorCoords(node.aabbMax, vertices[triangle.secondI].position);
        node.aabbMax = math::maxVectorCoords(node.aabbMax, vertices[triangle.thirdI].position);
    }
}

void BVHBuilder::generateDrawData(GfxContext& context)
{
    auto& vertices = m_geometry.vertices;
    auto& indices = m_geometry.indices;

    vertices.resize(lastElement * 8);
    indices.resize(lastElement * 24);
    uint leafCount=0;
    uint triangleCount=0;

    for(u32 i = 0; i < lastElement; ++i)
    {
        BVHNode& node = m_nodes[i];
        if(node.triangleCount>0)
        {
            leafCount++;
            triangleCount += node.triangleCount;
        }
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
    util::printInfo("{} leafs {}", leafCount, triangleCount);
    m_submesh = m_geometry.getSubmesh();
    m_mesh.init(context, m_geometry);
}

};
