Texture2D txDiffuse[2] : register(t0);
SamplerState samLinear : register(s0);

static float4 PixelColor;

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
    
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}