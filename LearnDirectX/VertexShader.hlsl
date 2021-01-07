struct VSOut
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
    float4 color : COLOR;
};

VSOut VSmain(float2 pos : POSITION, float2 tex : TEXCOORD, float3 color : COLOR)
{
    VSOut output;

    output.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
    output.tex = tex;
    output.color = float4(color, 1.0f);

    return output;
}