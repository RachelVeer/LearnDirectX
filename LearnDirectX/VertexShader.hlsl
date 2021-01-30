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
    float2 tex : TexCoord;
};

VSOut VSmain(float3 Position : POSITION, float3 Normal : NORMAL, float2 TexCoords : TEXCOORD)
{
    VSOut output;
    
    float4 worldPos = mul(float4(Position, 1.0f), model);
    output.normal = float4(mul(transpose((float3x3) model), Normal), 0.0f);
    output.worldPos = worldPos;
    
    output.pos = output.worldPos;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    output.tex = TexCoords;
    return output;
}