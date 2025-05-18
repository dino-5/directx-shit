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
    m_minDim = m_nodes[0].aabb.diagonal() * 1e-20f;

    subdivide(0);
    util::printInfo("{} elements of BVH", lastElement);
}

void BVHBuilder::subdivide(u32 index)
{
    PROFILER("BVHBuilder::subdivide");
	uint32_t task[256], taskCount = 0, nodeIdx = 0;

    while(1)
    {
        while(1)
        {
            BVHNode& node = m_nodes[nodeIdx];

            u32 bestSplitAxisIndex = 0;
            u32 bestSplitPos = 0;
            float bestSah = 1e20;
            AABB bestLeftAABB;
            AABB bestRightAABB;
            Vector3 stepSize =  (float)NUMBER_OF_BINS / node.aabb.diagonal();
            Vector3 nodeMin = node.aabb.aabbMin;
            {
                PROFILER("binning");
                {
                    AABB binsAABB[3][NUMBER_OF_BINS];
                    u32 binsCount[3][NUMBER_OF_BINS];
                    memset(binsCount, 0, 3 * NUMBER_OF_BINS * sizeof(u32));
                    {
                        PROFILER("bin sorting");
                        for (u32 j = node.leftChild; j < node.leftChild + node.triangleCount; ++j)
                        {
                            u32 triIndex = triIndices[j];
                            Int3 binDistr = (triangles[triIndex].aabb.middle() - nodeMin) * stepSize;
                            binDistr = math::clamp(binDistr, Int3(0), Int3(NUMBER_OF_BINS - 1));
                            binsAABB[0][binDistr[0]].grow(triangles[triIndex].aabb);
                            binsCount[0][binDistr[0]]++;
                            binsAABB[1][binDistr[1]].grow(triangles[triIndex].aabb);
                            binsCount[1][binDistr[1]]++;
                            binsAABB[2][binDistr[2]].grow(triangles[triIndex].aabb);
                            binsCount[2][binDistr[2]]++;
                        }
                    }

                    for(int i = 0; i < 3; ++i)
                        if(node.aabb.diagonal()[i] > m_minDim[i])
                    {

                        float leftArea[NUMBER_OF_BINS - 1], rightArea[NUMBER_OF_BINS - 1];
                        AABB leftAABB[NUMBER_OF_BINS - 1], rightAABB[NUMBER_OF_BINS - 1];

                        u32 currentLeftCount = 0, currentRightCount = 0;
                        AABB currentLeftAABB, currentRightAABB;

                        {
                            PROFILER("calculation of area");
                            for (u32 j = 0; j < NUMBER_OF_BINS - 1; ++j)
                            {
                                    currentLeftAABB.grow(binsAABB[i][j]);
                                    leftAABB[j] = currentLeftAABB;
                                    currentLeftCount += binsCount[i][j];
                                    leftArea[j] = currentLeftCount == 0 ? 1e20 : currentLeftAABB.area() * currentLeftCount;

                                    currentRightAABB.grow(binsAABB[i][NUMBER_OF_BINS - j - 1]);
                                    rightAABB[NUMBER_OF_BINS - j - 2] = currentRightAABB;
                                    currentRightCount += binsCount[i][NUMBER_OF_BINS - j - 1];
                                    rightArea[NUMBER_OF_BINS - j - 2] = currentRightCount == 0 ? 1e20 : currentRightAABB.area() * currentRightCount;
                            }
                        }

                        {
                            PROFILER("best area finding");
                            for (u32 j = 0; j < NUMBER_OF_BINS - 1; ++j)
                            {
                                    float sah = leftArea[j] + rightArea[j];
                                    if (sah < bestSah)
                                    {
                                        bestSah = sah;
                                        bestSplitAxisIndex = i;
                                        bestLeftAABB = leftAABB[j];
                                        bestRightAABB = rightAABB[j];
                                        bestSplitPos = j;
                                    }
                            }
                        }
                    }

                }
            }
            
            float splitCost = 1 + bestSah / node.aabb.area();
            float noSplitCost = (float)node.triangleCount;
            if(splitCost >= noSplitCost)
                break;

            u32 i = node.leftChild, j = i + (node.triangleCount - 1);
            {
                PROFILER("sorting of triangles");
                while (i < j)
                {
                    auto& triAABB = triangles[triIndices[i]].aabb;
                    float centroid = (triAABB.aabbMin[bestSplitAxisIndex] + triAABB.aabbMax[bestSplitAxisIndex]) * 0.5;
                    i32 binDistr = (i32)((centroid - nodeMin[bestSplitAxisIndex]) * stepSize[bestSplitAxisIndex]);
                    binDistr = math::clamp(binDistr, 0, (i32)NUMBER_OF_BINS - 1);
                    if((u32)binDistr <= bestSplitPos)
                        i++;
                    else
                        std::swap(triIndices[i], triIndices[j--]);
                }
            }

            u32 leftCount = i - node.leftChild;
            if (leftCount == 0 || leftCount == node.triangleCount)
                break;

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

            {
                BVHNode& node = m_nodes[leftChildIndex];
                node.aabb = bestLeftAABB;
            }

            {
                BVHNode& node = m_nodes[rightChildIndex];
                node.aabb = bestRightAABB;
            }

            task[taskCount++] = rightChildIndex;
            nodeIdx = leftChildIndex;
        }
        if(taskCount == 0)
            break;
        nodeIdx = task[--taskCount];
    }

}

void BVHBuilder::updateNodeBounds(u32 index)
{
    PROFILER("BVHBuilder::updateNodeBounds");
    BVHNode& node = m_nodes[index];
    for (u32 i = node.leftChild; i < node.leftChild + node.triangleCount; ++i)
    {
        auto& triangle = triangles[triIndices[i]];
        node.aabb.grow(triangle.aabb);
    }
}

};
