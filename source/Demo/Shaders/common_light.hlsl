
struct Light
{
    float3 position;
	float3 direction;
	uint flags;
};

struct LightSettings
{
    float3 cameraPosition;
    float3 cameraDirection;
};
