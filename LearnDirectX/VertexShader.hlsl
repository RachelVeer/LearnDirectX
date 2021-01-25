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
    float4 normal : NORMAL;
    float4 worldPos : Position;
};

VSOut VSmain(float3 pos : POSITION, float3 normal : NORMAL)
{
    VSOut output;
    
    float4 worldPos = mul(float4(pos, 1.0f), model);
    output.normal = float4(mul(transpose((float3x3) model), normal), 0.0f);
    output.worldPos = worldPos;
    
    output.pos = output.worldPos;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    return output;
}