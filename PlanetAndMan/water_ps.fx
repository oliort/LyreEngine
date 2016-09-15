struct PS_INPUT
{
	float4 Pos : SV_Position;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};

Texture2D texOcean : register(t0);
SamplerState tex2DSampler : register(s0);

float4 PS(PS_INPUT input) : SV_Target
{
	float4 outputColor = input.Color * texOcean.Sample(tex2DSampler, input.Tex);
	return outputColor;
}
