cbuffer LPos
{
    matrix model;
};

struct VSOut
{
    float4 pspos : SV_Position;
};

VSOut main(float3 pos : Position)
{
    VSOut vout;
    vout.pspos = mul(float4(pos, 1.0f), model);
    return vout;
}