float4 main(float3 lightColor : Color) : SV_TARGET
{
	return float4(lightColor, 1.0f);
}