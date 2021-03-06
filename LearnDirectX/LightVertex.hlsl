cbuffer Transform : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 viewPos;
}

struct VSOut
{
    float4 pos : SV_POSITION;
};

VSOut VSmain(float3 pos : POSITION, float3 normal : NORMAL, float2 tex : TEXCOORD)
{
    VSOut output;
    float4x4 vp = mul(view, projection);
    float4x4 mvp = mul(model, vp);
    float4 result = mul(float4(pos, 1.0f), mvp);
    output.pos = result;

    return output;
}