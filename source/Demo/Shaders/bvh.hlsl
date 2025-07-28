#include "Shaders/primitives.hlsl"

struct BVHDescription
{
    uint bvhNodeBufferIndex;
    uint bvhIndicesBufferIndex;
    uint bvhNodeCount;
};

struct AABB
{
    float3 mi, ma;

    bool intersect(Ray r, float t)
    {
        float3 t_min = (mi - r.pos) / r.dir;
        float3 t_max = (ma - r.pos) / r.dir;

        float t_maxmin = max(min(t_min.x, t_max.x), min(t_min.y, t_max.y));
        t_maxmin = max(t_maxmin, min(t_min.z, t_max.z));

        float t_minmax = min(max(t_min.x, t_max.x), max(t_min.y, t_max.y));
        t_minmax = min(t_minmax, max(t_min.z, t_max.z));

        return t_maxmin<t_minmax && t_minmax>0 && t_maxmin<t;
    }
};

struct BVHNode
{
    AABB aabb;
    uint first, count;
    bool isLeaf() { return count > 0; }
};

struct BVH
{
    StructuredBuffer<BVHNode> nodes;
    StructuredBuffer<uint> indices;
    uint bvhNodeCount;

    HitRecord hit(Ray ray, SphereArray array, float2 interval)
    {
        uint nodeIndex = 0, taskCount = 0;
        uint tasks[512];
        float t = 100000;

        HitRecord record = getHit(); 
        bool hitAny = false;
        float closestHit = interval.y;

        while(1)
        {
            while(1)
            {
                BVHNode node = nodes[nodeIndex];

                if(!node.aabb.intersect(ray, t)) break;

                if(node.isLeaf() && node.aabb.intersect(ray, closestHit))
                {
                    for(uint i=node.first, end=i+node.count; i<end; i++)
                    {
                        HitRecord tempRec = getHit();//hit_sphere(array.spheres[indices[i]],
                         //             ray, interval.x, closestHit);

                        if(tempRec.hit) // we shrink interval to the closestHit every time
                        {
                            record = tempRec;
                            record.hit = true;
                            closestHit = tempRec.t;
                        }
                    }
                    break;
                }
                else
                {
                    nodeIndex = node.first;
                    if(taskCount>510) break;
                    tasks[taskCount++] = nodeIndex+1;
                }

                
            }
            if(!taskCount) break;
            nodeIndex = tasks[--taskCount];
        }
        return record;

    }
};

