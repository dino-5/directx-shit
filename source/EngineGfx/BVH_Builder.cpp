#include "BVH_Builder.h"
#include "Model.h"
#include "EngineCommon/util/Timer.h"

namespace engine::graphics
{

constexpr u32 NUMBER_OF_BINS = 8;
struct Bin
{
    AABB aabb;
    u32 triangleCount = 0;
};

BVHBuilder::BVHBuilder(const Model& model)
{
    build(model);
}

void BVHBuilder::build(const Model& model)
{
    m_model = &model;
    PROFILER("BVH::build");

    auto& submeshes = model.m_submeshes;
    auto& vertices = model.m_geometry.vertices;
    auto& indices = model.m_geometry.indices;

    triangles.reserve(indices.size()/3);
    m_nodes.resize(triangles.capacity() * 2 - 1);

    for(auto& submesh : submeshes)
    {
        for(u32 i = submesh.StartIndexLocation; i < submesh.StartIndexLocation + submesh.IndexCount; i+=3)
        {
            auto ind = &indices[i];
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
    float minDiagonal = 1e20, maxDiagonal = -1e20;
    for(auto& node: m_nodes)
    {
        if(node.isLeaf())
        {
            float d = node.aabb.diagonal().length();
            if (d > maxDiagonal)
                maxDiagonal = d;
            if (d < minDiagonal)
                minDiagonal = d;
        }
    }
    util::printInfo("{} elements of BVH, {} min and max {} ", lastElement, minDiagonal, maxDiagonal);
}

float BVHBuilder::findBestSplitPosition(u32 nodeIndex, u32& splitAxis, float& splitPosition)
{
    BVHNode& node = m_nodes[nodeIndex];

    float bestSah = 1e20 + 5.1235f;
    auto& vertices = m_model->m_geometry.vertices;

    {
        AABB aabb;
        for (u32 j = node.leftChild; j < node.leftChild + node.triangleCount; ++j)
            aabb.grow(triangles[triIndices[j]].centroid);

        Bin bins[NUMBER_OF_BINS][3];
        Vector3 stepSize =  (float)NUMBER_OF_BINS / aabb.diagonal();
        for (u32 j = node.leftChild; j < node.leftChild + node.triangleCount; ++j)
        {
            Triangle& triangle = triangles[triIndices[j]];
            Vector3 binDistr = (triangle.centroid - aabb.aabbMin) * stepSize;
            for(int i = 0; i < 3; ++i)
            {
                u32 binIdx = min(NUMBER_OF_BINS - 1, (u32)(binDistr[i]));
                bins[binIdx][i].aabb.grow(vertices[triangle.vertex0].position);
                bins[binIdx][i].aabb.grow(vertices[triangle.vertex1].position);
                bins[binIdx][i].aabb.grow(vertices[triangle.vertex2].position);
                bins[binIdx][i].triangleCount++;
            }
        }

        float leftArea[NUMBER_OF_BINS - 1][3], rightArea[NUMBER_OF_BINS - 1][3];

        u32 currentLeftCount[3] = {0,0,0}, currentRightCount[3] = {0,0,0};
        AABB leftAABB[3], rightAABB[3];

        for (u32 j = 0; j < NUMBER_OF_BINS - 1; ++j)
        {
            for(int i = 0; i < 3; ++i)
            {
                if(bins[j][i].triangleCount)
                    leftAABB[i].grow(bins[j][i].aabb);
                currentLeftCount[i] += bins[j][i].triangleCount;
                leftArea[j][i] = leftAABB[i].area() * currentLeftCount[i];

                if(bins[NUMBER_OF_BINS - j - 1][i].triangleCount)
                    rightAABB[i].grow(bins[NUMBER_OF_BINS - j - 1][i].aabb);
                currentRightCount[i] += bins[NUMBER_OF_BINS - j - 1][i].triangleCount;
                rightArea[NUMBER_OF_BINS - j - 2][i] = rightAABB[i].area() * currentRightCount[i];
            }
        }

        stepSize =  aabb.diagonal() / (float)NUMBER_OF_BINS ;
        for (u32 j = 0; j < NUMBER_OF_BINS - 1; ++j)
        {
            for(int i = 0; i < 3; ++i)
            {
                float sah = leftArea[j][i] + rightArea[j][i];
                if (sah < bestSah)
                {
                    bestSah = sah;
                    splitAxis = i;
                    splitPosition = aabb.aabbMin[i] + (j+1) * stepSize[i];
                }
            }
        }
    }
    return bestSah;
}

void BVHBuilder::subdivide(u32 index)
{
    BVHNode& node = m_nodes[index];

    u32 bestSplitAxisIndex = 0;
    float bestSplitLine = 0;
    float bestSah = findBestSplitPosition(index, bestSplitAxisIndex, bestSplitLine);
    
    float parentCost = node.triangleCount * node.aabb.area();
    if(parentCost <= bestSah)
        return;

    u32 i = node.leftChild, j = i + (node.triangleCount - 1);
    while (i < j)
    {
        auto& triangle = triangles[triIndices[i]];
        if(triangle.centroid[bestSplitAxisIndex] < bestSplitLine)
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
    auto& vertices = m_model->m_geometry.vertices;
    for (u32 i = node.leftChild; i < node.leftChild + node.triangleCount; ++i)
    {
        auto& triangle = triangles[triIndices[i]];
        node.aabb.grow(vertices[triangle.vertex0].position);
        node.aabb.grow(vertices[triangle.vertex1].position);
        node.aabb.grow(vertices[triangle.vertex2].position);
    }
}

};
