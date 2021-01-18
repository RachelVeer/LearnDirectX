// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float4 color : COLOR, float4 normal : NORMAL) : SV_TARGET
{
    return color;
}