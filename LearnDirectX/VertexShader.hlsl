cbuffer ConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 viewPos;
}

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
};

VSOut VSmain(float3 pos : POSITION, float3 normal : NORMAL)
{
    VSOut output;
    
    float4 PixelPos = mul(float4(pos, 1.0f), model);
    float4x4 vp = mul(view, projection);
    float4 mvp = mul(PixelPos, vp);
    output.pos = mvp;
    
    output.normal = float4(normal, 1.0f);
    output.color = PixelPos;
    return output;
}