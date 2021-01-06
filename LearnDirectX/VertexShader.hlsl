struct VSOut
{
    // Color is moved to the top of struct to be read in by pixel shader.
    // Otherwise, pixel shader input would need to take in position as well (similiar to VSmain input). 
    float4 color : COLOR;
    float4 position : SV_POSITION;
};

VSOut VSmain(float2 position : POSITION, float3 color : COLOR)
{
    VSOut output;

    output.position = float4(position.x, position.y, 0.0f, 1.0f);
    output.color = float4(color, 1.0f);

    return output;
}