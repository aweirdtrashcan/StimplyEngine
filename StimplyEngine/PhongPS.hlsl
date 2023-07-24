cbuffer CBufs
{
    float4 lightPos;
};

static const float intensity = 1.0f;
static const float3 lightColor = { 1.0f, 1.0f, 0.6f };

float4 main(float3 col : Color, float3 normal : Normal, float3 worldPos : Pos) : SV_Target
{
    const float3 distanceToLight = (float3)lightPos - worldPos;
    const float3 ambient = col * 0.15f;
    const float lenghtToLight = length(distanceToLight);
    const float3 lightToSurfaceAngle = distanceToLight / lenghtToLight;
    const float finalCol = lightColor * max(0.0f, dot(lightToSurfaceAngle, normal));
    return float4(saturate((finalCol + ambient) * col), 1.0f);
}