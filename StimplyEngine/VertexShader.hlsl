struct InputData
{
    float3 pos : Position;
    float3 color : Color;
//  float2 texCoord : TCoord;
};

struct OutData
{
//  float2 texCoord : TCoord;
    float4 color : Color;
    float4 pos : SV_Position;
};

cbuffer Cbuf
{
    matrix mat;
};

OutData main(InputData id)
{
    OutData od;
    od.pos = mul(float4(id.pos, 1.0f), mat);
    //od.pos = float4(id.pos, 1.0f);
    od.color = float4(id.color, 1.0f);
    //od.texCoord = id.texCoord;
    return od;
}