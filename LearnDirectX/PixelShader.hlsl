Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET
{
    return txDiffuse.Sample(samLinear, tex);
}