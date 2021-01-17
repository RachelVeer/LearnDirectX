cbuffer ConstantBuffer : register(b0)
{
    matrix transform;
    float4 objectColor;
    float4 lightColor;
}

struct VSOut
{
    float4 pos : SV_POSITION;
};

VSOut VSmain(float3 pos : POSITION)
{
    VSOut output;

    output.pos = mul(float4(pos, 1.0f), transform);

    return output;
}