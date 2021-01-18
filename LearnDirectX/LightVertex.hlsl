cbuffer ConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 lightPos;
    float4 objectColor;
    float4 lightColor;
}

struct VSOut
{
    float4 pos : SV_POSITION;
};

VSOut VSmain(float3 pos : POSITION, float3 normal : NORMAL)
{
    VSOut output;
    float4x4 vp = mul(view, projection);
    float4x4 mvp = mul(model, vp);
    float4 result = mul(float4(pos, 1.0f), mvp);
    output.pos = result;

    return output;
}