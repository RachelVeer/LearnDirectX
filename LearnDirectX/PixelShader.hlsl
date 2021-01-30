Texture2D txDiffuse[1] : register(t0);
SamplerState samLinear : register(s0);

cbuffer Transform : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 viewPos;
}


// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float4 normal : NORMAL, float4 worldPos : Position, float2 tex : TexCoord) : SV_TARGET
{
    
    return txDiffuse[0].Sample(samLinear, tex);
}