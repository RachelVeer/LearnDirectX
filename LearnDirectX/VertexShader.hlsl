cbuffer ConstantBuffer : register(b0)
{
    matrix transform;
    float4 objectColor;
    float4 lightColor;
}

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOut VSmain(float3 pos : POSITION)
{
    VSOut output;

    float4 ambientStrength = float4(0.1f, 0.1f, 0.1f, 1.0f);
    float4 ambient = ambientStrength * lightColor;
    
    output.pos = mul(float4(pos, 1.0f), transform);
    float4 result = ambient * objectColor;
    output.color = result;

    return output;
}