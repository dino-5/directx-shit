struct Material
{
    float4 diffuse;
    float4 specular;
    float shininess; 
};

struct LightColor{
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

struct  PointLight
{
    float4 Pos;  
    float4 Color;
    float saturation_a;
    float saturation_b;
    float saturation_c;
};


#ifdef POINT_LIGHT_SOURCE
float4 CalculateLighting(Material material, PointLight light, float4 position, float4 normal)
{

}
#endif