cbuffer ConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 lightPos;
    float4 objectColor;
    float4 lightColor;
}

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
};

VSOut VSmain(float3 pos : POSITION, float3 normal : NORMAL)
{
    VSOut output;
    
    float4 PixelPos = mul(float4(pos, 1.0f), model);
    float4x4 vp = mul(view, projection);
    float4 mvp = mul(PixelPos, vp);
    output.pos = mvp;
    
    output.normal = float4(normal, 1.0f);
    
    // Ambient 
    float4 ambientStrength = float4(0.1f, 0.1f, 0.1f, 1.0f);
    float4 ambient = ambientStrength * lightColor;
    
    // Diffuse
    float3 norm = normalize(normal);
    float3 lightDir = normalize(lightPos - PixelPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = diff * lightColor;
    
    float4 result = (ambient + diffuse) * objectColor;
    output.color = result;

    return output;
}