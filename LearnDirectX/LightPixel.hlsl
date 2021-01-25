struct Light
{
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

cbuffer Light : register(b2)
{
    Light light;
}

float4 PSmain(float4 pos : SV_POSITION) : SV_TARGET
{
	return light.diffuse;
}