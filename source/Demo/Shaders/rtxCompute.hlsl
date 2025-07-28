#include "Shaders/bvh.hlsl"

#define PI 3.1415

struct ViewSettings
{
    float4x4 cameraMatrix;
    float4x4 projection;
    float3 cameraPos;
    float pad0;
    float3 cameraViewDir;
    float pad1;
    float3 cameraRightDir;
    float pad2;
    float3 cameraUpDir;
    float pad3;
    float fov;
};

struct RTXData
{
    uint sphereCount;
    uint imWidth;
    uint imHeight;
    float3 color;
};

struct IndirectIndices
{
    uint outputTextureIndex;
    uint sphereBufferIndex;
    uint bvhDescIndex;
};

ConstantBuffer<ViewSettings> g_view : register(b0);
ConstantBuffer<RTXData> g_rtxData : register(b1);
ConstantBuffer<IndirectIndices> bindless : register(b2);

#define RDH(index) ResourceDescriptorHeap[index]

float degrees_to_radians(float degrees)
{
    return degrees / 180 * PI;
}


float4 calculateSky(float blue)
{
    return float4((1 - blue) * float2(1,1) + blue * float2(0.5, 0.7), 
                  1.f, 1.f); 
}

/* */
HitRecord hit(BVH bvh, Ray ray, SphereArray array, uint sphereCount, float2 interval)
{
    uint nodeIndex = 0, taskCount = 0;
    uint tasks[512];

    HitRecord record = getHit(); 
    bool hitAny = false;
    float closestHit = interval.y;

    while(1)
    {
        while(1)
        {
            if(nodeIndex >= bvh.bvhNodeCount) break;
            BVHNode node = bvh.nodes[nodeIndex];

            if(!node.aabb.intersect(ray, closestHit)) break;

            if(node.isLeaf())
            {
                if(!node.aabb.intersect(ray, closestHit)) break;

                uint start=node.first, end=start+node.count;
                for(uint i=start; i<end; i++)
                {
                    HitRecord tempRec = getHit();

                    if(i < sphereCount)
                    {
                        if(bvh.indices[i] < sphereCount)
                            tempRec = hit_sphere(array.spheres[bvh.indices[i]],
                                             ray, interval.x, closestHit);
                    }

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
                if(nodeIndex < bvh.bvhNodeCount-1) tasks[taskCount++] = nodeIndex+1;
            }

            
        }
        if(!taskCount) break;
        nodeIndex = tasks[--taskCount];
    }
    return record;

}
/*
*/


struct Camera
{

    void createCamera(BVH aBVH,
                      float width,
                      float height,
                      float3 cameraPos,
                      float3 cameraV,
                      float3 cameraR,
                      float3 cameraU,
                      float fov,
                      uint2 id,
                      uint samples,
                      uint depth,
                      float lensR,
                      float focusD)
    {
        bvh = aBVH;

        lensAngle = lensR;
        focusDist = focusD;
        pos = cameraPos;

        V = cameraV;
        R = cameraR;
        U = cameraU;

        sampleCount = samples;
        maxDepth = depth;

        uint seed = id.x + id.y * (uint)width;
        hash = Hash(seed);

        float aspectRatio = width / height;
        viewPlaneC = pos + cameraV * focusDist;
        float cosHalfFov = cos(fov/2);
        float sy = cosHalfFov * focusDist;
        float sx = sy * aspectRatio; 

        float3 lb = viewPlaneC - sx * cameraR + sy * cameraU; // left bottom

        dx =  2 * sx / width;
        dy = 2 * sy / height;

        pixel = lb + dx * id.x * cameraR - dy * id.y * cameraU;

        float defocusRadius = focusDist * tan(degrees_to_radians(lensAngle / 2));
        defocus_disk_u = U * defocusRadius;
        defocus_disk_r = R * defocusRadius;

    }

    Ray getRay()
    {
        Ray r;
        float3 rayOrigin = pos;
        r.pos = rayOrigin;
        r.dir = normalize(pixel - pos);
        return r;
    }

    Ray getRay(uint i)
    {
        Ray r;

        r.pos = defocus_disk_sample();

        float randomX = dx * (Random1(hash) - 0.5f); 
        float randomY = dy * (Random1(hash) - 0.5f); 

        r.dir = normalize(pixel + R * randomX + U * randomY - r.pos);

        return r;
    }
#define USE_BVH 1
    float4 pixelColor(SphereArray array, Ray r)
    {
        float4 color = float4(1,1,1,1);

        HitRecord hitRec;
        for(int i = 0; i < maxDepth; i++)
        {
        #if USE_BVH
            hitRec = hit(bvh, r, array, sphCount, interv);
        #else
            hitRec = hitArray(array, r, interv, sphCount);
        #endif
            if(hitRec.hit)
            {
                Ray scattered;
                float4 attenuation;

                scattered.pos = hitRec.p;
                bool result = true;

                if(hitRec.m.isLambertian())
                {
                    attenuation = hitRec.m.color;
                    scattered.dir = hitRec.n + RandomOnHemisphere(hitRec.n, hash); // could be zero vector
                }
                else if (hitRec.m.isMetal())
                {
                    attenuation = hitRec.m.color;
                    scattered.dir = reflect(r.dir, hitRec.n);
                    scattered.dir +=  hitRec.m.fuzz * RandomOnHemisphere(hitRec.n, hash);
                    result = dot(scattered.dir, hitRec.n) > 0;
                }
                else if(hitRec.m.isDielectric())
                {
                    attenuation = float4(1,1,1,1);
                    float rIndex = hitRec.m.refractionIndex;
                    float ri = hitRec.frontFace ? 1 / rIndex : rIndex;
                    scattered.dir = refractRay(r.dir, hitRec.n, ri);
                }
                else
                {
                    break;
                }

                scattered.dir = normalize(scattered.dir);

                if(result)
                {
                    r = scattered;
                    color *= attenuation;
                }
                else
                {
                    color = float4(0,0,0,0);
                    break;

                }
            }
            else 
            {

                r.dir = normalize(r.dir);
                float blue = 0.5 * (r.dir.y + 1);
                color *= calculateSky(blue);
                break;
            }
        }

        return color;
    }

    float3 defocus_disk_sample() {
        // Returns a random point in the camera defocus disk.
        float2 p = random_in_unit_disk(hash);
        return pos + (p.x * defocus_disk_r) + (p.y * defocus_disk_u);
    }

    float4 render(SphereArray array, float2 interval, uint sphereCount)
    {
        Ray r = getRay();
        float4 color = float4(0,0,0,0);

        interv = interval;
        sphCount = sphereCount;

        float scale = 1 / float(sampleCount);
        for(int i = 0; i < sampleCount; i++)
        {
            color += pixelColor(array, getRay(i));
        }

        return color / sampleCount;
    }

    float3 V;
    float3 R;
    float3 U;

    float3 viewPlaneC;

    float3 pixel;
    float dx;
    float3 pos;
    float dy;
    uint hash;
    uint sampleCount;
    uint maxDepth;

    uint sphCount;
    float2 interv;

    float3 defocus_disk_u;       // Defocus disk horizontal radius
    float lensAngle;
    float3 defocus_disk_r;       // Defocus disk vertical radius
    float focusDist;

    BVH bvh;
};

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if(id.x >= g_rtxData.imWidth || id.y >= g_rtxData.imHeight)
        return;

    RWTexture2D<float4> tex = RDH(bindless.outputTextureIndex);
    ConstantBuffer<CB_Sphere> sphereArray = RDH(bindless.sphereBufferIndex);
    ConstantBuffer<BVHDescription> bvhDesc = RDH(bindless.bvhDescIndex);

    BVH bvh;
    bvh.nodes = RDH(bvhDesc.bvhNodeBufferIndex);
    bvh.indices = RDH(bvhDesc.bvhIndicesBufferIndex);
    bvh.bvhNodeCount = bvhDesc.bvhNodeCount;

    Camera camera;
    camera.createCamera(bvh, 
                        g_rtxData.imWidth,
                        g_rtxData.imHeight,
                        g_view.cameraPos,
                        g_view.cameraViewDir,
                        g_view.cameraRightDir,
                        g_view.cameraUpDir,
                        g_view.fov,
                        id.xy,
                        50,
                        4,
                        0.6,
                        10);

    float4 color = camera.render(sphereArray.array,
                               float2(0.001, 100000),
                               g_rtxData.sphereCount); 
    color.xyz = pow(color.xyz, 1/2.2);
    tex[id.xy] = color;
}

