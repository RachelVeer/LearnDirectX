// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}