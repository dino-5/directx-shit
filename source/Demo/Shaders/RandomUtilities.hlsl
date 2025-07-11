uint Hash(uint x)
{
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

float Random1(inout uint x)
{
    float result =  (float)(Hash(x)) / 4294967296.0; // normalize to [0,1)
    x = Hash(x);
    return result;
}

float Random1Range(inout uint x, float min, float max)
{
    float result = Random1(x);
    x++;
    return result*(max-min) + min;
}

float2 Random2(inout uint x)
{
    float2 result;
    result.x = Random1(x);
    x += Hash(x);
    result.y = Random1(x);
    x += Hash(x);
    return result;
}

float3 Random3(inout uint x)
{
    float3 result;
    result.x = Random1(x);
    x++;// = Hash(x);
    result.y = Random1(x);
    x++;// = Hash(x);
    result.z = Random1(x);
    x++;// = Hash(x);
    return result;
}

float3 Random3Unit(inout uint x)
{
    return normalize((Random3(x) - 0.5) * 2);
}

float3 random_in_unit_disk(inout uint x ) 
{
    while(true){
        float3 p = float3(Random1Range(x, -1, 1), Random1Range(x, -1, 1), 0);
        float len = length(p);
        if (len*len < 1)
            return p;
    }
}

float3 RandomOnHemisphere(float3 normal, inout uint x)
{
    float3 v = Random3Unit(x);
    if(dot(normal, v) > 0)
        return v;
    return -v;
}

