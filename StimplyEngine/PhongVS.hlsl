cbuffer ConstBufs
{
    matrix model;
    matrix MVP;
};

struct VSOut
{
    float3 col : Color;
    float3 normal : Normal;
    float3 psPos : Pos;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 col : Color, float3 normal : Normal)
{
    VSOut vso;
    vso.pos = mul(float4(pos, 1.0f), model);
    vso.normal = mul(normal, (float3x3)model);
    vso.col = col;
    vso.psPos = (float3)mul(float4(pos, 1.0f), model);
    return vso;
}