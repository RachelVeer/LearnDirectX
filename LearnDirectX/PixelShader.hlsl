Texture2D txDiffuse[2] : register(t0);
SamplerState samLinear : register(s0);

// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float2 tex : TEXCOORD, float3 color : COLOR) : SV_TARGET
{
    float4 tex1;
    float4 tex2;
    float4 FinalColor;
    
    tex1 = txDiffuse[0].Sample(samLinear, tex);
    tex2 = txDiffuse[1].Sample(samLinear, tex);
    
    // Linearly interpolate between both textures (80% container, 20% awesomeface).
    FinalColor = tex1 + tex2 / 2;
    
    return FinalColor;
}