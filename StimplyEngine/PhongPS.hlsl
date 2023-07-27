cbuffer CBufs
{
    float3 lightPos;
    float padding;
};

static const float intensity = 1.0f;
static const float3 lightColor = { 1.0f, 1.0f, 0.6f };
static const float3 col = { 0.7f, 0.7f, 0.7f };

float4 main(float3 normal : Normal, float3 worldPos : Pos) : SV_Target
{
    const float3 distanceToLight = (float3)lightPos - worldPos;
    const float3 ambient = col * 0.15f;
    const float lenghtToLight = length(distanceToLight);
    const float3 lightToSurfaceAngle = distanceToLight / lenghtToLight;
    const float3 finalCol = col * max(0.0f, dot(lightToSurfaceAngle, normal));
    return float4(saturate(finalCol + ambient), 1.0f);
}